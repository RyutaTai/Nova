#include "Decal.h"

#include <algorithm>

#include "Shader.h"
#include "Graphics.h"
#include "../Resources/Texture.h"
#include "../Others/Misc.h"

Decal::Decal(ID3D11Device* device, const wchar_t* filename)
{
	HRESULT hr = S_OK;

	DirectX::XMFLOAT3 vertices[24] = {};
	UINT indices[36] = {};

	int face = 0;

	//	top-side
	//	0---------1
	//	|         |
	//	|   -Y    |
	//	|         |
	//	2---------3
	face = 0;
	vertices[face * 4 + 0] = { -0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 1] = { +0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2] = { -0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 3] = { +0.5f, +0.5f, -0.5f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	//	bottom-side
	//	0---------1
	//	|         |
	//	|   -Y    |
	//	|         |
	//	2---------3
	face += 1;
	vertices[face * 4 + 0] = { -0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 1] = { +0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 2] = { -0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3] = { +0.5f, -0.5f, -0.5f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;

	//	front-side
	//	0---------1
	//	|         |
	//	|   +Z    |
	//	|         |
	//	2---------3
	face += 1;
	vertices[face * 4 + 0] = { -0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 1] = { +0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 2] = { -0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3] = { +0.5f, -0.5f, -0.5f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	//	back-side
	//	0---------1
	//	|         |
	//	|   +Z    |
	//	|         |
	//	2---------3
	face += 1;
	vertices[face * 4 + 0] = { -0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 1] = { +0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2] = { -0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 3] = { +0.5f, -0.5f, +0.5f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;

	//	right-side
	//	0---------1
	//	|         |      
	//	|   -X    |
	//	|         |
	//	2---------3
	face += 1;
	vertices[face * 4 + 0] = { +0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 1] = { +0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2] = { +0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3] = { +0.5f, -0.5f, +0.5f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	//	left-side
	//	0---------1
	//	|         |      
	//	|   -X    |
	//	|         |
	//	2---------3
	face += 1;
	vertices[face * 4 + 0] = { -0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 1] = { -0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2] = { -0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3] = { -0.5f, -0.5f, +0.5f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;

	D3D11_BUFFER_DESC bufferDesc{};
	D3D11_SUBRESOURCE_DATA subresourceData{};

	//	vertex buffer
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(vertices));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	subresourceData.pSysMem = vertices;
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBuffer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	index buffer
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(indices));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	subresourceData.pSysMem = indices;
	hr = device->CreateBuffer(&bufferDesc, &subresourceData, indexBuffer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	constant buffer
	bufferDesc.ByteWidth = sizeof(Constants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&bufferDesc, NULL, constantBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	Graphics::Instance().GetShader()->CreateVsFromCso(device, "./Resources/Shader/DecalVS.cso", vertexShader_.GetAddressOf(), inputLayout_.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/DecalPS.cso", pixelShader_.GetAddressOf());

	LoadTextureFromFile(device, filename, decalTexture_.GetAddressOf(), NULL);

	D3D11_DEPTH_STENCIL_DESC decalDepthStencilDesc = {};
	decalDepthStencilDesc.DepthEnable = TRUE;
	decalDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	decalDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	decalDepthStencilDesc.StencilEnable = TRUE;
	decalDepthStencilDesc.StencilReadMask = 0xFF;
	decalDepthStencilDesc.StencilWriteMask = 0xFF;
	decalDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	decalDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	decalDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
	decalDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
	decalDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	decalDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
	decalDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
	decalDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
	hr = device->CreateDepthStencilState(&decalDepthStencilDesc, decalDepthStencilState_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	hr = device->CreateRasterizerState(&rasterizerDesc, decalRasterizerStates_[0/*cull back*/].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	hr = device->CreateRasterizerState(&rasterizerDesc, decalRasterizerStates_[1/*cull front*/].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	
}

void Decal::Blit(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* const* depthStencilShaderResourceView)
{
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> cachedDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>	cachedRasterizerState;
	deviceContext->OMGetDepthStencilState(cachedDepthStencilState.GetAddressOf(), 0);
	deviceContext->RSGetState(cachedRasterizerState.GetAddressOf());

	UINT stride = sizeof(DirectX::XMFLOAT3);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(inputLayout_.Get());

	for (decltype(spots_)::const_reference Spot : spots_)
	{
		// Calculates the orientation along the normal direction of mesh
		DirectX::XMVECTOR Z = DirectX::XMVector3Normalize(DirectX::XMVectorSet(-Spot.normal_.x, -Spot.normal_.y, -Spot.normal_.z, 0));
		DirectX::XMVECTOR Y = DirectX::XMVector3Normalize(DirectX::XMVectorSet(0, 1, 0, 0));
		DirectX::XMVECTOR X = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(Y, Z));
		Y = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(Z, X));
		DirectX::XMMATRIX R = DirectX::XMMatrixIdentity();
		R.r[0] = X;
		R.r[1] = Y;
		R.r[2] = Z;

		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(Spot.scale_, Spot.scale_, Spot.scale_);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(Spot.position_.x, Spot.position_.y, Spot.position_.z);
		DirectX::XMMATRIX W = S * R * T;

		Constants data;
		DirectX::XMStoreFloat4x4(&data.world_, W);
		DirectX::XMMATRIX V = DirectX::XMMatrixInverse(NULL, W);
		DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicLH(1, 1, 0, 1);
		DirectX::XMStoreFloat4x4(&data.decalInverseProjection_, V * P);
		deviceContext->UpdateSubresource(constantBuffer_.Get(), 0, 0, &data, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
		deviceContext->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());

		deviceContext->OMSetDepthStencilState(decalDepthStencilState_.Get(), 1);
		deviceContext->RSSetState(decalRasterizerStates_[1/*cull front*/].Get());
		deviceContext->VSSetShader(vertexShader_.Get(), NULL, 0);
		deviceContext->PSSetShader(NULL, NULL, 0);
		deviceContext->DrawIndexed(36, 0, 0);

		deviceContext->OMSetDepthStencilState(decalDepthStencilState_.Get(), 0);
		deviceContext->RSSetState(decalRasterizerStates_[0/*cull back*/].Get());
		deviceContext->VSSetShader(vertexShader_.Get(), NULL, 0);
		deviceContext->PSSetShader(pixelShader_.Get(), NULL, 0);
		deviceContext->PSSetShaderResources(0, 1, decalTexture_.GetAddressOf());
		deviceContext->PSSetShaderResources(1, 1, depthStencilShaderResourceView);
		deviceContext->DrawIndexed(36, 0, 0);
	}

	deviceContext->OMSetDepthStencilState(cachedDepthStencilState.Get(), 0);
	deviceContext->RSSetState(cachedRasterizerState.Get());

}

void Decal::Add(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& normal, const float& scale)
{
	spots_.push_back({ position, normal, scale });
	if (spots_.size() < SpotsDataMax_)return;

	/*for (int i = 0; i < SpotsDataMax_ - 1; ++i)
	{
		spots_.at(i) = spots_.at(i + 1);
	}
	spots_.erase(spots_.begin() + SpotsDataMax_ - 1);*/
}

void Decal::DrawDebug()
{
	if (ImGui::TreeNode("Decal"))
	{
		spotsIndexMax_ = static_cast<int>(spots_.size()) - 1;
		if (spots_.empty() == false)
		{
			ImGui::InputInt("SpotsIndex", &spotsIndex_);
			ImGui::InputInt("SpotsIndexMax", &spotsIndexMax_);
			spotsIndex_ = std::clamp(spotsIndex_, 0, static_cast<int>(spots_.size()));
			ImGui::DragFloat3("SpotPosition",	&spots_[spotsIndex_].position_.x);
			ImGui::DragFloat3("SpotNormal",		&spots_[spotsIndex_].normal_.x);
			ImGui::DragFloat("SpotScale",		&spots_[spotsIndex_].scale_);
		}


		ImGui::TreePop();
	}

}
