#pragma once

#include <memory>
#include <string>
#include <map>

#include "GltfModel.h"
#include "GltfModelStaticBatching.h"
#include "Sprite.h"
#include "Effect.h"

//	リソースマネージャー
class ResourceManager
{
private:
	ResourceManager() {}
	~ResourceManager() = default;
public:
	static ResourceManager& Instance()
	{
		static ResourceManager instance;
		return instance;
	}

	//	Gltfモデルリソース読み込み
	std::shared_ptr <GltfModel>					LoadGltfModelResource(const std::string& filename, const std::string& rootNodeName = "root");

	//	Gltfモデルリソース読み込み
	std::shared_ptr <GltfModelStaticBatching>	LoadGltfModelStaticResource(const std::string& filename, const bool& setColor = false, const DirectX::XMFLOAT4& color = { 0,0,0,1 });

	//	スプライトリソース読み込み
	std::shared_ptr <Sprite>					LoadSpriteResource(const std::string& filename);

	//	エフェクトリソース読み込み
	std::shared_ptr <Effect>					LoadEffectResource(const char* filename);

private:
	//	Gltfモデルリソースマップ
	using GltfModelMap = std::map<std::string, std::shared_ptr<GltfModel>>;
	GltfModelMap gltfModels_;

	//	Gltfモデルリソースマップ
	using GltfModelStaticMap = std::map<std::string, std::shared_ptr<GltfModelStaticBatching>>;
	GltfModelStaticMap gltfStaticModels_;

	//	スプライトリソースマップ
	using SpriteMap = std::map<std::string, std::shared_ptr<Sprite>>;
	SpriteMap sprites_;

	//	エフェクトリソースマップ
	using EffectMap = std::map<std::string, std::shared_ptr<Effect>>;
	EffectMap effects_;

};