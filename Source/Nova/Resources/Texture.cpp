#include "texture.h"

#include <filesystem>
#include <fstream>
#include <DDSTextureLoader.h>

static std::map<std::pmr::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> resources;

HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView_,
	DWORD value/*0xAABBGGRR*/, UINT dimension)
{
	HRESULT hr{ S_OK };

	D3D11_TEXTURE2D_DESC texture2dDesc{};
	texture2dDesc.Width = dimension;
	texture2dDesc.Height = dimension;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	size_t texels = dimension * dimension;
	std::unique_ptr<DWORD[]>sysmem{ std::make_unique<DWORD[]>(texels) };
	for (size_t i = 0; i < texels; i++)sysmem[i] = value;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = sysmem.get();
	subresourceData.SysMemPitch = sizeof(DWORD) * dimension;

	Microsoft::WRL::ComPtr <ID3D11Texture2D> texture2d;
	hr = device->CreateTexture2D(&texture2dDesc, &subresourceData, texture2d.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = texture2dDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(texture2d.Get(), &shaderResourceViewDesc,
		shaderResourceView_);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	return hr;
}

HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* filename,
	ID3D11ShaderResourceView** shaderResourceView, D3D11_TEXTURE2D_DESC* texture2dDesc)
{
	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11Resource>resource;
	std::filesystem::path ddsFilename(filename);
	ddsFilename.replace_extension("dds");
	if (std::filesystem::exists(ddsFilename.c_str()))	//	ファイル拡張子がddsの場合
	{
		hr = DirectX::CreateDDSTextureFromFile(device, ddsFilename.c_str(), resource.GetAddressOf(), shaderResourceView);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	else
	{
		hr = DirectX::CreateWICTextureFromFile(device, filename, resource.GetAddressOf(), shaderResourceView);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	resources.insert(std::make_pair(filename, *shaderResourceView));
	
	if (texture2dDesc)
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2D;
		hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2D.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		texture2D->GetDesc(texture2dDesc);
	}

	return hr;
}

HRESULT LoadTextureFromMemory(ID3D11Device* device, const void* data,
	size_t size, ID3D11ShaderResourceView** shaderResourceView)
{
	HRESULT hr{ S_OK };
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	hr = DirectX::CreateDDSTextureFromMemory(device, reinterpret_cast<const uint8_t*>(data),
		size, resource.GetAddressOf(), shaderResourceView);
	if (hr != S_OK)
	{
		hr = DirectX::CreateWICTextureFromMemory(device, reinterpret_cast<const uint8_t*>(data),
			size, resource.GetAddressOf(), shaderResourceView);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	return hr;
}

void ReleaseAllTextures()
{
	resources.clear();
}