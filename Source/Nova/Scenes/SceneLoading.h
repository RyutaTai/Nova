#pragma once

#include <thread>

#include "Scene.h"
#include "../Resources/Sprite.h"

class SceneLoading :public Scene
{
public:
	SceneLoading(Scene* nextScene) :nextScene_(nextScene) {}
	~SceneLoading()override {}

	void Initialize()						override;
	void Finalize()							override;

	void Update(const float& elapsedTime)	override;
	void ShadowRender() 					override;
	void Render()							override;
	void DrawDebug()						override;

	//	----- フェード -----
	void SetAllSpriteAlpha();

private:
	enum class SpriteLoading
	{
		Back,				//	背景画像
		Loading,			//	ローディング　テキスト
		TitleText,
		Max,				//	スプライトの上限数
	};
	std::unique_ptr	<Sprite> sprites_[static_cast<int>(SpriteLoading::Max)];

	//	ローディングスレッド
	static void LoadingThread(SceneLoading* scene);

private:
	Scene*						nextScene_	= nullptr;
	std::thread*				thread_		= nullptr;

	//	----- スプライトアニメーション -----
	float loadSpriteAngle_ = 0.0f;
	int animationNumber_		= 0;		//	現在のアニメーション数(何枚目のアニメーションか)
	int animationMAX_			= 5;		//	画像のアニメーション枚数
	float animationFrame_		= 120.0f;	//	次のコマに行くまでのフレーム数
	float animationTimer_		= 0;		//	現在のフレーム数(アニメーションタイマー)

	//	----- フェード -----
	float allSpriteAlpha_ = 1.0f;

};

