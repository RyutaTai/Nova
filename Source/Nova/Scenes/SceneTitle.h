#pragma once

#include <memory>
#include <thread>

#include "Scene.h"
#include "../Resources/Sprite.h"
#include "../../Game/UI/UI.h"
#include "../Audio/AudioSource.h"
#include "../AI/StateMachine.h"

class SceneTitle : public Scene
{
public:	
	//	ステート
	enum class StateType
	{
		FadeIn,
		Main,
		Setting,
		Max,
	};

public:
	SceneTitle(){}
	~SceneTitle()override{}

	void Initialize()	override;
	void Finalize()		override;

	void Update(const float& elapsedTime)override;
	void ShadowRender()	override {}
	void Render()		override;
	void DrawDebug()	override;
	void DrawStateStr();

	//	----- ステート -----
	void ChangeState(const StateType& state) { stateMachine_->ChangeState(static_cast<int>(state)); }	//	ステート遷移
	StateMachine<State<SceneTitle>>* GetStateMachine() { return stateMachine_.get(); }					//	ステートマシン取得

	//	----- フェード -----
	void SetTitleLogoAlpha(const float& alpha) { titleLogoAlpha_ = alpha; }
	void SetKeyTextAlpha(const float& alpha) { keyTextAlpha_ = alpha; }

	//	----- ロードシーンへ遷移 -----
	void ChangeLoadingScene();

private:
	//	----- ステート -----
	std::unique_ptr<StateMachine<State<SceneTitle>>>	stateMachine_ = nullptr;	//	ステートマシン

private:	//	スプライト
	enum class SpriteTitle
	{
		Back,				//	背景画像
		Groove,				//	グルーブ(タイトルテキスト)
		KeyText,			//	キーテキスト
		Max,				//	スプライトの上限数
	};
	std::unique_ptr	<Sprite> sprites_[static_cast<int>(SpriteTitle::Max)];

	//	----- フェード処理 -----
	float titleLogoAlpha_ = 0.0f;		//	タイトルロゴのアルファ値
	float keyTextAlpha_ = 0.0f;			//	キーテキストのアルファ値

};

