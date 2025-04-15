#include "ShadowMap.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Camera.h"
#include "../Others/Misc.h"

ShadowMap::ShadowMap()
{
	HRESULT hr{ S_OK };

	ID3D11Device* device = Graphics::Instance().GetDevice();

	//	サイズが大きいほど精度は上がるが、動作が重くなる
	uint32_t width = 2048;
	uint32_t height = 2048;

	//	Texture2Ddesc
	D3D11_TEXTURE2D_DESC texture2dDesc{};
	texture2dDesc.Width = width;
	texture2dDesc.Height = height;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	hr = device->CreateTexture2D(&texture2dDesc, 0, depthStencilBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	DepthStencilView
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &shaderResourceViewDesc, shaderResourceView_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	// ViewPort
	viewport_.Width		= static_cast<float>(width);
	viewport_.Height	= static_cast<float>(height);
	viewport_.MinDepth	= 0.0f;
	viewport_.MaxDepth	= 1.0f;
	viewport_.TopLeftX	= 0.0f;
	viewport_.TopLeftY	= 0.0f;

	// Buffer(shadowConstantBuffer)
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(Graphics::SceneConstants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	hr = Graphics::Instance().GetDevice()->CreateBuffer(&buffer_desc, nullptr, shadowConstantBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

void ShadowMap::Activate(float depth)
{
	Graphics::Instance().GetDeviceContext()->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH, depth, 0);

	viewportCount_ = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	viewportCount_ = 1;

	Graphics::Instance().GetDeviceContext()->RSGetViewports(&viewportCount_, cachedViewports_);
	Graphics::Instance().GetDeviceContext()->OMGetRenderTargets(1, cachedRenderTargetView_.ReleaseAndGetAddressOf(), cachedDepthStencilView_.ReleaseAndGetAddressOf());

	Graphics::Instance().GetDeviceContext()->RSSetViewports(1, &viewport_);
	ID3D11RenderTargetView* nullRenderTargetView{ NULL };
	Graphics::Instance().GetDeviceContext()->OMSetRenderTargets(1, &nullRenderTargetView, depthStencilView_.Get());

	//	SHADOW Buffer
	HRESULT hr{ S_OK };
	Graphics::Instance().ClearSceneConstant();
	Graphics::Instance().SetLightDirection(lightDirection_);
#if 1
	cameraPosition_.x = Camera::Instance().GetEye().x;
	cameraPosition_.y = Camera::Instance().GetEye().y;
	cameraPosition_.z = Camera::Instance().GetEye().z;
	cameraPosition_.w = 1.0f;
	Graphics::Instance().SetCameraPosition(cameraPosition_);

	/*lightViewFocus_.x = Camera::Instance().GetFocus().x;
	lightViewFocus_.y = Camera::Instance().GetFocus().y;
	lightViewFocus_.z = Camera::Instance().GetFocus().z;
	lightViewFocus_.w = 1.0f;*/
#else
	sceneConstant.cameraPosition_ = cameraPosition_;
#endif

	// SHADOW : make shadow map
	{
		// SHADOW
		using namespace DirectX;
		const float aspect_ratio = this->viewport_.Width / this->viewport_.Height;
		XMVECTOR F{ XMLoadFloat4(&lightViewFocus_) };
		XMVECTOR E{ F - XMVector3Normalize(XMLoadFloat4(&lightDirection_)) * lightViewDistance_ };	//TODO:ライトの位置
		XMVECTOR U{ XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };
		XMMATRIX V{ XMMatrixLookAtLH(E, F, U) };
		XMMATRIX P{ XMMatrixOrthographicLH(lightViewSize_ * aspect_ratio, lightViewSize_, lightViewNearZ_, lightViewFarZ_) };

		Graphics::Instance().SetViewProjection(V * P);
		DirectX::XMFLOAT4X4 lightViewProjection = Graphics::Instance().GetViewProjection();
		Graphics::Instance().SetLightViewProjection(lightViewProjection);
		Graphics::SceneConstants sceneConstant = Graphics::Instance().GetSceneConstant();
		Graphics::Instance().GetDeviceContext()->UpdateSubresource(shadowConstantBuffer_.Get(), 0, 0, &sceneConstant, 0, 0);
		Graphics::Instance().GetDeviceContext()->VSSetConstantBuffers(1, 1, shadowConstantBuffer_.GetAddressOf());

	}

}

//	ShadowMap
void ShadowMap::Deactivate()
{
	Graphics::Instance().GetDeviceContext()->RSSetViewports(viewportCount_, cachedViewports_);
	Graphics::Instance().GetDeviceContext()->OMSetRenderTargets(1, cachedRenderTargetView_.GetAddressOf(), cachedDepthStencilView_.Get());
}

// デバッグ描画
void ShadowMap::DrawDebug()
{
	if (ImGui::TreeNode(u8"Shadowシャドウ"))	//	SHADOW
	{
		ImGui::SliderFloat("lightViewDistance", &lightViewDistance_, 1.0f, +100.0f);
		ImGui::SliderFloat("lightViewSize",		&lightViewSize_, 1.0f, +100.0f);
		ImGui::SliderFloat("lightViewNear_z",	&lightViewNearZ_, 1.0f, lightViewFarZ_ - 1.0f);
		ImGui::SliderFloat("lightViewFar_z",	&lightViewFarZ_, lightViewNearZ_ + 1.0f, +100.0f);
		//ImGui::SliderFloat3("LightPosition",	&.x, -1.0f, +1.0f);
		ImGui::SliderFloat3("LightDirection",	&lightDirection_.x, -1.0f, +1.0f);
		ImGui::SliderFloat3("LightViewFocus",	&lightViewFocus_.x, -1.0f, +1.0f);
		ImGui::SliderFloat3("CameraPosition",	&cameraPosition_.x, -1.0f, +1.0f);
		ImGui::Image(reinterpret_cast<void*>(this->shaderResourceView_.Get()), ImVec2(2048 / 5.0f, 2048 / 5.0f));
		ImGui::TreePop();
	}
}
