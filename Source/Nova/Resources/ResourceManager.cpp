#include"ResourceManager.h"

#include "../Graphics/Graphics.h"
#include "../Others/Converter.h"

//	FBXモデルリソース読み込み
std::shared_ptr<FbxModel> ResourceManager::LoadFbxModelResource(const char* filename, const bool& triangulate, const float& samplingRate)
{
	auto it = fbxModels_.find(filename);
	if (it != fbxModels_.end())
	{
		return it->second;
	}

	auto model = std::make_shared<FbxModel>(filename,triangulate,samplingRate);

	fbxModels_[filename] = model;

	return model;
}

std::shared_ptr<GltfModel> ResourceManager::LoadGltfModelResource(const std::string& filename, const std::string& rootNodeName)
{
	auto it = gltfModels_.find(filename);
	if (it != gltfModels_.end())
	{
		return it->second;
	}

	auto model = std::make_shared<GltfModel>(filename);

	gltfModels_[filename] = model;

	return model;
}

//	GLTFモデルリソース読み込み
std::shared_ptr<GltfModelStaticBatching> ResourceManager::LoadGltfModelStaticResource(const std::string& filename, const bool& setColor, const DirectX::XMFLOAT4& color)
{
	auto it = gltfStaticModels_.find(filename);
	if (it != gltfStaticModels_.end())
	{
		return it->second;
	}

	auto model = std::make_shared<GltfModelStaticBatching>(filename);

	gltfStaticModels_[filename] = model;

	return model;
}

//	スプライトリソース読み込み
std::shared_ptr<Sprite> ResourceManager::LoadSpriteResource(const std::string& filename)
{
	//	stringからwstringへ変換
	std::wstring wFilename = ConvertStringToWstring(filename);
	//	wstringからwchar_t*へ変換
	const wchar_t* wcharFilename = wFilename.c_str();

	auto it = sprites_.find(filename);
	if (it != sprites_.end())
	{
		return it->second;
	}

	auto sprite = std::make_shared<Sprite>(wcharFilename);

	sprites_[filename] = sprite;

	return sprite;
}

//	エフェクトリソース読み込み
std::shared_ptr<Effect> ResourceManager::LoadEffectResource(const char* filename)
{
	auto it = effects_.find(filename);
	if (it != effects_.end())
	{
		return it->second;
	}

	auto effect = std::make_shared<Effect>(filename);

	effects_[filename] = effect;

	return effect;
}