#pragma once

#include <memory>
#include <string>
#include <map>

#include "Model.h"
#include "GltfModel.h"
#include "GltfModelStaticBatching.h"
#include "Sprite.h"
#include "Effect.h"

class Model;

//	リソースマネージャー
class ResourceManager
{
private:
	ResourceManager() {};
	~ResourceManager() {};
public:
	static ResourceManager& Instance()
	{
		static ResourceManager instance;
		return instance;
	}

	//	Fbxモデルリソース読み込み
	std::shared_ptr <Model>						LoadFbxModelResource(const char* fileName, bool triangulate = false, float samplingRate = 0);

	//	Gltfモデルリソース読み込み
	std::shared_ptr <GltfModel>					LoadGltfModelResource(const std::string& fileName, const std::string& rootNodeName = "root");

	//	Gltfモデルリソース読み込み
	std::shared_ptr <GltfModelStaticBatching>	LoadGltfModelStaticResource(const std::string& fileName, const bool setColor = false, const DirectX::XMFLOAT4 color = { 0,0,0,1 });

	//	スプライトリソース読み込み
	std::shared_ptr <Sprite>					LoadSpriteResource(const std::string& fileName);

	//	エフェクトリソース読み込み
	std::shared_ptr <Effect>					LoadEffectResource(const char* fileName);

private:
	//	Fbxモデルリソースマップ
	using ModelMap = std::map<std::string, std::weak_ptr <Model>>;
	ModelMap models_;

	//	Gltfモデルリソースマップ
	using GltfModelMap = std::map<std::string, std::weak_ptr <GltfModel>>;
	GltfModelMap gltfModels_;

	//	Gltfモデルリソースマップ
	using GltfModelStaticMap = std::map<std::string, std::weak_ptr <GltfModelStaticBatching>>;
	GltfModelStaticMap gltfStaticModels_;

	//	スプライトリソースマップ
	using SpriteMap = std::map<std::string, std::weak_ptr<Sprite>>;
	SpriteMap sprites_;

	//	エフェクトリソースマップ
	using EffectMap = std::map<std::string, std::weak_ptr <Effect>>;
	EffectMap effects_;

};