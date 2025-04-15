#pragma once

#include <WICTextureLoader.h>
#include <wrl.h>
#include <string>
#include <map>

#include "../Others/Misc.h"

HRESULT MakeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView_,
	DWORD value/*0xAABBGGRR*/, UINT dimension);

HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* fileName,
	ID3D11ShaderResourceView** shaderResourceView_, D3D11_TEXTURE2D_DESC* texture2dDesc);

HRESULT LoadTextureFromMemory(ID3D11Device* device, const void* data, 
	size_t size, ID3D11ShaderResourceView** shaderResourceView);

void ReleaseAllTextures();