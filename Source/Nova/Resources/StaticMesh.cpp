#include"StaticMesh.h"

#include <fstream>
#include <filesystem>

#include "Texture.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"

//	コンストラクタ
StaticMesh::StaticMesh(ID3D11Device* device, const wchar_t* objFileName,bool inverted)
{
	std::vector<Vertex>   vertices;
	std::vector<uint32_t> indices;
	uint32_t currentIndex{ 0 };

	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMFLOAT3> normals;
	std::vector<DirectX::XMFLOAT2> texcoords;
	std::vector<std::wstring>      mtlFilenames;

	//	OBJファイルパーサー
	std::wifstream fin(objFileName);
	_ASSERT_EXPR(fin, L"OBJ file not found.");
	wchar_t command[256];
	while (fin)
	{
		fin >> command;
		if (0 == wcscmp(command, L"v"))
		{
			float x, y, z;
			fin >> x >> y >> z;
			positions.push_back({ x,y,z });
			fin.ignore(1024, L'\n');
		}
		else if (0 == wcscmp(command, L"vn"))
		{
			float i, j, k;
			fin >> i >> j >> k;
			normals.push_back({ i,j,k });
			fin.ignore(1024, L'\n');
		}
		
		else if (0 == wcscmp(command, L"vt"))
		{
			float u, v;
			fin >> u >> v;
			if (inverted)texcoords.push_back({ u,1.0f - v });
			else if (!inverted)texcoords.push_back({ u,v });
			fin.ignore(1024, L'\n');
		}
		else if (0 == wcscmp(command, L"f"))
		{
			for (size_t i = 0; i < 3; i++)
			{
				Vertex vertex;
				size_t v, vt, vn;

				fin >> v;
				vertex.position_ = positions.at(v - 1);
				if (L'/' == fin.peek())
				{
					fin.ignore(1);
					if (L'/' != fin.peek())
					{
						fin >> vt;
						vertex.texcoord_ = texcoords.at(vt - 1);
					}
					if (L'/' == fin.peek())
					{
						fin.ignore(1);
						fin >> vn;
						vertex.normal_ = normals.at(vn - 1);
					}
				}
				vertices.push_back(vertex);
				indices.push_back(currentIndex++);
			}
			fin.ignore(1024, L'\n');
		}
		
		else if (0 == wcscmp(command, L"mtllib"))
		{
			wchar_t mtllib[256];
			fin >> mtllib;
			mtlFilenames.push_back(mtllib);
		}

		else if (0== wcscmp(command, L"usemtl"))
		{
			wchar_t usemtl[MAX_PATH]{ 0 };
			fin >> usemtl;
			subsets_.push_back({ usemtl,static_cast<uint32_t>(indices.size()),0 });
		}
		else 
		{
			fin.ignore(1024, L'\n');
		}
	}
	fin.close();

	std::vector<Subset>::reverse_iterator iterator = subsets_.rbegin();
	iterator->indexCount_ = static_cast<uint32_t>(indices.size()) - iterator->indexStart_;
	for (iterator = subsets_.rbegin() + 1; iterator != subsets_.rend(); ++iterator)
	{
		iterator->indexCount_ = (iterator - 1)->indexStart_ - iterator->indexStart_;
	}

	std::filesystem::path mtlFilename(objFileName);
	mtlFilename.replace_filename(std::filesystem::path(mtlFilenames[0]).filename());

	fin.open(mtlFilename);
	_ASSERT_EXPR(fin, L"MTL file not found");

	//	MTLファイルパーサー
	while (fin)
	{
		fin >> command;
		if (0 == wcscmp(command, L"map_Kd"))
		{
			fin.ignore();
			wchar_t mapKd[256];
			fin >> mapKd;

			std::filesystem::path path(objFileName);
			path.replace_filename(std::filesystem::path(mapKd).filename());
			materials_.rbegin()->textureFilenames_[0] = path;
			fin.ignore(1024, L'\n');
		}
		else if (0 == wcscmp(command, L"map_bump") || 0 == wcscmp(command, L"bump"))
		{
			fin.ignore();
			wchar_t mapBump[256];
			fin >> mapBump;

			std::filesystem::path path(objFileName);
			path.replace_filename(std::filesystem::path(mapBump).filename());
			materials_.rbegin()->textureFilenames_[1] = path;
			fin.ignore(1024, L'\n');
		}
		else if (0 == wcscmp(command, L"newmtl"))
		{
			fin.ignore();
			wchar_t newmtl[256];
			Material material;
			fin >> newmtl;
			material.name_ = newmtl;
			materials_.push_back(material);
		}
		else if (0 == wcscmp(command, L"Kd"))
		{
			float r, g, b;
			fin >> r >> g >> b;
			materials_.rbegin()->Kd_ = { r,g,b,1 };
			fin.ignore(1024, L'\n');
		}
	}
	fin.close();

	D3D11_TEXTURE2D_DESC texture2dDesc{};
	for (Material& material : materials_)
	{
		LoadTextureFromFile(device, material.textureFilenames_[0].c_str(),
			material.shaderResourceViews_[0].GetAddressOf(), &texture2dDesc);
		MakeDummyTexture(device, material.shaderResourceViews_[0].GetAddressOf(), 0xFFFFFFFF, 16);
		MakeDummyTexture(device, material.shaderResourceViews_[1].GetAddressOf(), 0xFFFF7F7F, 16);
	}
	CreateComBuffers(device, vertices.data(), vertices.size(), indices.data(), indices.size());

	HRESULT hr{ S_OK }; 
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0} ,
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0} ,
	};
	Graphics::Instance().GetShader()->CreateVsFromCso(device, "./Resources/Shader/StaticMeshVs.cso", vertexShader_.GetAddressOf(),
		inputLayout_.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/StaticMeshPs.cso", pixelShader_.GetAddressOf());

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(Constants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	
}

void StaticMesh::CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
	uint32_t* indices, size_t indexCount)
{
	HRESULT hr{ S_OK };

	D3D11_BUFFER_DESC bufferDesc{};
	D3D11_SUBRESOURCE_DATA subresourceData{};
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertexCount);
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

	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indexCount);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	subresourceData.pSysMem = indices;
	hr = device->CreateBuffer(&bufferDesc, &subresourceData, indexBuffer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

//	描画処理
void StaticMesh::Render(ID3D11DeviceContext* deviceContext,
	const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor)
{
	uint32_t stride{ sizeof(Vertex) };
	uint32_t offset{ 0 };
	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(inputLayout_.Get());
	deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
	deviceContext->PSSetShader(pixelShader_.Get(), nullptr, 0);

	for (const Material& material : materials_)
	{
		Constants data{ world,materialColor };
		XMStoreFloat4(&data.materialColor_, DirectX::XMVectorMultiply(XMLoadFloat4(&materialColor), XMLoadFloat4(&material.Kd_)));
		deviceContext->UpdateSubresource(constantBuffer_.Get(), 0, 0, &data, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());

		deviceContext->PSSetShaderResources(0, 1, material.shaderResourceViews_[0].GetAddressOf());
		deviceContext->PSSetShaderResources(1, 1, material.shaderResourceViews_[1].GetAddressOf());
		if (materials_.size() == 0)
		{
			for (const Subset& subset : subsets_)
			{
				materials_.push_back({ subset.usemtl_ });
			}
		}

		for (const Subset& subset : subsets_) 
		{
			if (material.name_ == subset.usemtl_)
			{
				deviceContext->DrawIndexed(subset.indexCount_, subset.indexStart_, 0);
			}
		}
	}

}