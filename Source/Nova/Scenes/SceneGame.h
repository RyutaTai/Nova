#pragma once

#include "Scene.h"
#include "../Resources/Sprite.h"
#include "../Input/Input.h"
#include "../Audio/AudioManager.h"
#include "../../Game/Stage.h"
#include "../../Game/Player.h"
#include "../../Game/Dragonkin.h"
#include "../../Game/Drone.h"
#include "../PostProcess/Bloom.h"
#include "../../Game/UI/UI.h"
#include "../../Nova/Graphics/CascadedShadowMaps.h"
#include "../../Nova/Graphics/Vignette.h"
#include "../../Nova/Graphics/ColorFilter.h"

class SceneGame : public Scene
{
public:
	//	ステート
	enum class SceneGameState
	{
		Wave1,
		Wave2,
		Wave3,
		Clear,
		GameOver,
		Continue,
		Max,
	};

public:
	SceneGame(){}
	~SceneGame()override{}

	void Initialize()						override;
	void Finalize()							override;

	void Update(const float& elapsedTime)	override;
	void ShadowRender() 					override;
	void Render()							override;
	void DrawDebug()						override;

	void ChangeState(SceneGameState state) { stateMachine_->ChangeState(static_cast<int>(state)); }	//	ステート遷移

	void LoadWaveSprite(const wchar_t* filename);
	StateMachine<State<SceneGame>>* GetStateMachine() { return stateMachine_.get(); }	//	ステートマシン取得
	void IsPose(bool isPose);
	void Reset();

	void SetChangeTitleTimer(const float& changeTitleTimer) { changeTitleTimer_ = changeTitleTimer; }
	void ChangeToTitle(bool changeTitle)	{ changeTitle_ = changeTitle; }
	void SetWaveStartTimer(float timer)		{ waveStartTimer_ = timer; }
	void SetGameOver(bool gameOver)			{ isGameOver_ = gameOver; }
	void SetGameClear(bool gameClear)		{ isGameClear_ = gameClear; }
	void SetIsResult(bool isResult)			{ isResult_ = isResult; }

	float	GetWaveStartTimer()		{ return waveStartTimer_; }
	float	GetChangeTitleTimer()	{ return changeTitleTimer_; }
	bool	GetIsResult()			{ return isResult_; }

private:
	//	シャドウマップ
	void MakeShadow();	//	シャドウ生成
	void DrawShadow();	//	シャドウ描画

private:
	/* ----- オブジェクト ----- */
	std::unique_ptr	<Stage>		stage_;
	std::unique_ptr	<Player>	player_;
	std::unique_ptr	<Dragonkin>	dragonkin_;
	std::unique_ptr<StateMachine<State<SceneGame>>>		stateMachine_ = nullptr;		//	ステートマシン

	/* ----- 描画関係 ----- */
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceViews_[8];
	std::unique_ptr<Bloom>						bloomer_ = nullptr;		//	BLOOM
	std::unique_ptr<FrameBuffer>				framebuffers_[8];
	std::unique_ptr<FullScreenQuad>				bitBlockTransfer_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShaders_[8];
	float										nearZ_ = 50.0f;
	float										farZ_ = 400000.0f;
	Microsoft::WRL::ComPtr <ID3D11Buffer>		sceneConstantBuffer_;
	DirectX::XMFLOAT4							lightDirection_ = { +3.545f, -3.860f, -0.326f, 0.0f };
	DirectX::XMFLOAT4							adjustColor_ = {};

	//	シャドウマップ
	std::unique_ptr<CascadedShadowMaps> cascadedShadowMaps_;
	float criticalDepthValue_ = 115.0f; // If this value is 0, the camera's far panel distance is used.

	//	ヴィネット(周辺減光)
	std::unique_ptr<Vignette> vignette_;

	//	カラーフィルター
	std::unique_ptr<ColorFilter> colorFilter_;

	/* ----- スプライト ----- */
	enum SPRITE_GAME
	{
		BACK,			//	背景画像
		WAVE,			//	ウェーブ
		RESULT,			//	リザルト
		HpFrame,		//	HPゲージ枠
		HpGauge,		//	HPゲージ
		HpGaugeBack,	//	HPゲージの減少量を描画する用
		Instructions,	//	操作方法
		Clear,			//	クリア
		GameOver,		//	ゲームオーバー
		Max,			//	スプライトの上限数
	};
	std::unique_ptr <Sprite>			  sprite_[static_cast<int>(SPRITE_GAME::Max)];

	/* ----- ゲーム内で使う変数 ----- */
	float	waveStartTimer_		= 0.0f;		//	ウェーブ開始のUIが表示されている間
	bool	isGameOver_			= false;
	bool	changeTitle_		= false;
	bool	isGameClear_		= false;
	float	changeTitleTimer_	= 3.0f;
	bool	isResult_			= false;	//	リザルト画面かどうか(クリア、ゲームオーバー)

	//	デバッグ用
private:
	bool drawUI_ = true;
};

