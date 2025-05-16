#define NOMINMAX
#include "CascadedShadowMaps.h"

#include <array>

#include "../../Nova/Others/Misc.h"
#include "../../imgui/imgui.h"

//	Calculate the 8 vertices of the view frustum based on the provided view and projection matrices.
std::array<DirectX::XMFLOAT4, 8> ExtractFrustumCorners(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	//	Define the NDC space corners
	std::array<DirectX::XMFLOAT4, 8> frustumCorners =
	{
		DirectX::XMFLOAT4{-1.0f, -1.0f, -1.0f, 1.0f},
		DirectX::XMFLOAT4{-1.0f, -1.0f,  1.0f, 1.0f},
		DirectX::XMFLOAT4{-1.0f,  1.0f, -1.0f, 1.0f},
		DirectX::XMFLOAT4{-1.0f,  1.0f,  1.0f, 1.0f},
		DirectX::XMFLOAT4{ 1.0f, -1.0f, -1.0f, 1.0f},
		DirectX::XMFLOAT4{ 1.0f, -1.0f,  1.0f, 1.0f},
		DirectX::XMFLOAT4{ 1.0f,  1.0f, -1.0f, 1.0f},
		DirectX::XMFLOAT4{ 1.0f,  1.0f,  1.0f, 1.0f}
	};
	const DirectX::XMMATRIX invViewProjection = DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&view) * DirectX::XMLoadFloat4x4(&projection));
	for (std::array<DirectX::XMFLOAT4, 8>::reference frustumCorner : frustumCorners)
	{
		DirectX::XMStoreFloat4(&frustumCorner, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat4(&frustumCorner), invViewProjection));
	}
	//	Return a array of 8 vertices representing the corners of the view frustum in world space.
	return frustumCorners;
}

CascadedShadowMaps::CascadedShadowMaps(ID3D11Device* device, UINT width, UINT height, UINT cascadeCount)
	: cascadeCount_(cascadeCount), cascadedMatrices_(cascadeCount), cascadedPlaneDistances_(cascadeCount + 1)
{
	HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC texture2dDesc = {};
	texture2dDesc.Width = width;
	texture2dDesc.Height = height;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = cascadeCount;
	texture2dDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;
	hr = device->CreateTexture2D(&texture2dDesc, 0, depthStencilBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
	depthStencilViewDesc.Texture2DArray.ArraySize = static_cast<UINT>(cascadeCount);
	depthStencilViewDesc.Texture2DArray.MipSlice = 0;
	depthStencilViewDesc.Flags = 0;
	hr = device->CreateDepthStencilView(depthStencilBuffer_.Get(), &depthStencilViewDesc, depthStencilView_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT; // DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT : DXGI_FORMAT_R16_UNORM
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shaderResourceViewDesc.Texture2DArray.ArraySize = static_cast<UINT>(cascadeCount);
	shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
	shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
	shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(depthStencilBuffer_.Get(), &shaderResourceViewDesc, shaderResourceView_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	viewport_.Width = static_cast<float>(width);
	viewport_.Height = static_cast<float>(height);
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = (sizeof(ShadowConstants) + 0x0f) & ~0x0f;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&bufferDesc, NULL, constantBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

void CascadedShadowMaps::Activate(ID3D11DeviceContext* deviceContext,
	const DirectX::XMFLOAT4X4& cameraView,
	const DirectX::XMFLOAT4X4& cameraProjection,
	const DirectX::XMFLOAT4& lightDirection,
	float criticalDepthValue,
	UINT cbSlot)
{
	deviceContext->RSGetViewports(&viewportCount_, cachedViewports_);
	deviceContext->OMGetRenderTargets(1, cachedRenderTargetView_.ReleaseAndGetAddressOf(), cachedDepthStencilView_.ReleaseAndGetAddressOf());

	// near/far value from perspective projection matrix
	float m33 = cameraProjection._33;
	float m43 = cameraProjection._43;
	float zn = -m43 / m33;
	float zf = (m33 * zn) / (m33 - 1);
	zf = criticalDepthValue > 0 ? std::min(zf, criticalDepthValue) : zf;

	// calculates split plane distances in view space
	for (size_t cascadeIndex = 0; cascadeIndex < cascadeCount_; ++cascadeIndex)
	{
		float idc = cascadeIndex / static_cast<float>(cascadeCount_);
		float logarithmicSplitScheme = zn * pow(zf / zn, idc);
		float uniformSplitScheme = zn + (zf - zn) * idc;
		cascadedPlaneDistances_.at(cascadeIndex) = logarithmicSplitScheme * splitSchemeWeight_ + uniformSplitScheme * (1 - splitSchemeWeight_);
	}
	// make sure border values are accurate
	cascadedPlaneDistances_.at(0) = zn;
	cascadedPlaneDistances_.at(cascadeCount_) = zf;

	for (size_t cascadeIndex = 0; cascadeIndex < cascadeCount_; ++cascadeIndex)
	{
		float nearPlane = fitToCascade_ ? cascadedPlaneDistances_.at(cascadeIndex) : zn;
		float farPlane = cascadedPlaneDistances_.at(cascadeIndex + 1);

		DirectX::XMFLOAT4X4 cascadedProjection = cameraProjection;
		cascadedProjection._33 = farPlane / (farPlane - nearPlane);
		cascadedProjection._43 = -nearPlane * farPlane / (farPlane - nearPlane);

		std::array<DirectX::XMFLOAT4, 8> corners = ExtractFrustumCorners(cameraView, cascadedProjection);

		DirectX::XMFLOAT4 center = { 0, 0, 0, 1 };
		for (DirectX::XMFLOAT4 corner : corners)
		{
			center.x += corner.x;
			center.y += corner.y;
			center.z += corner.z;
		}
		center.x /= corners.size();
		center.y /= corners.size();
		center.z /= corners.size();

		DirectX::XMMATRIX V;
		V = DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(center.x - lightDirection.x, center.y - lightDirection.y, center.z - lightDirection.z, 1.0f),
			DirectX::XMVectorSet(center.x, center.y, center.z, 1.0f),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (DirectX::XMFLOAT4 corner : corners)
		{
			DirectX::XMStoreFloat4(&corner, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat4(&corner), V));
			minX = std:: min(minX, corner.x);
			maxX = std::max(maxX, corner.x);
			minY = std::min(minY, corner.y);
			maxY = std::max(maxY, corner.y);
			minZ = std::min(minZ, corner.z);
			maxZ = std::max(maxZ, corner.z);
		}

#if 1
		zMult_ = std::max<float>(1.0f, zMult_);
		if (minZ < 0)
		{
			minZ *= zMult_;
		}
		else
		{
			minZ /= zMult_;
		}
		if (maxZ < 0)
		{
			maxZ /= zMult_;
		}
		else
		{
			maxZ *= zMult_;
		}
#endif

		DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
		DirectX::XMStoreFloat4x4(&cascadedMatrices_.at(cascadeIndex), V * P);
	}

	shadowConstants_.cascadedMatrices_[0] = cascadedMatrices_.at(0);
	shadowConstants_.cascadedMatrices_[1] = cascadedMatrices_.at(1);
	shadowConstants_.cascadedMatrices_[2] = cascadedMatrices_.at(2);
	shadowConstants_.cascadedMatrices_[3] = cascadedMatrices_.at(3);

	shadowConstants_.cascadedPlaneDistances_[0] = cascadedPlaneDistances_.at(1);
	shadowConstants_.cascadedPlaneDistances_[1] = cascadedPlaneDistances_.at(2);
	shadowConstants_.cascadedPlaneDistances_[2] = cascadedPlaneDistances_.at(3);
	shadowConstants_.cascadedPlaneDistances_[3] = cascadedPlaneDistances_.at(4);

	deviceContext->UpdateSubresource(constantBuffer_.Get(), 0, 0, &shadowConstants_, 0, 0);
	deviceContext->VSSetConstantBuffers(cbSlot, 1, constantBuffer_.GetAddressOf());
	deviceContext->PSSetConstantBuffers(cbSlot, 1, constantBuffer_.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> nullRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> nullDepthStencilView;
	deviceContext->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH, 1, 0);
	deviceContext->OMSetRenderTargets(1, nullRenderTargetView.GetAddressOf(), depthStencilView_.Get());
	deviceContext->RSSetViewports(1, &viewport_);


}

void CascadedShadowMaps::Deactivate(ID3D11DeviceContext* deviceContext)
{
	deviceContext->RSSetViewports(viewportCount_, cachedViewports_);
	deviceContext->OMSetRenderTargets(1, cachedRenderTargetView_.GetAddressOf(), cachedDepthStencilView_.Get());
}

void CascadedShadowMaps::DrawDebug()
{
	ImGui::SliderFloat("SplitSchemeWeight", &splitSchemeWeight_, 0.0f, +1.0f);
	ImGui::Checkbox("FitToCascade", &fitToCascade_);
	ImGui::DragFloat("zMult", &zMult_, 0.01f);

	ImGui::SliderFloat("ShadowColor", &shadowConstants_.shadowColor_, +0.0f, +1.0f);
	ImGui::DragFloat("ShadowDepthBias", &shadowConstants_.shadowDepthBias_, 0.00001f, 0.0f, 0.01f, "%.8f");
	ImGui::Checkbox("ColorizeCascadedLayer", &shadowConstants_.colorizeCascadedLayer_);

}