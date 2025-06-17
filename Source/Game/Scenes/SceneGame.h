#pragma once

#include "Scene.h"
#include "../../Nova/Resources/Sprite.h"
#include "../../Nova/Input/Input.h"
#include "../../Nova/Audio/AudioManager.h"
#include "../../Game/Stage/Stage.h"
#include "../../Game/Character/Player/Player.h"
#include "../../Game/Character/Enemy/Dragonkin/Dragonkin.h"
#include "../../Game/Character/Enemy/Drone/Drone.h"
#include "../../Game/UI/UI.h"
#include "../../Nova/Graphics/Bloom.h"
#include "../../Nova/Graphics/CascadedShadowMaps.h"
#include "../../Nova/Graphics/Vignette.h"
#include "../../Nova/Graphics/ColorFilter.h"
#include "../../Nova/Graphics/ChromaticAberration.h"
#include "../../Nova/Graphics/ExposureFilter.h"
#include "../../Nova/Graphics/SharpenFilter.h"
#include "../../Nova/Graphics/ContrastFilter.h"

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
		Max,
	};

public:
	SceneGame(){}
	~SceneGame()override = default;

	void Initialize()						override;
	void Finalize()							override;

	void Update(const float& elapsedTime)	override;
	void Render()							override;
	void DrawDebug()						override;

	void ChangeState(SceneGameState state) { stateMachine_->ChangeState(static_cast<int>(state)); }	//	ステート遷移

	void LoadWaveSprite(const wchar_t* filename);
	StateMachine<State<SceneGame>>* GetStateMachine() { return stateMachine_.get(); }	//	ステートマシン取得
	void IsPose(const bool& isPose);
	void Reset();

	void SetChangeTitleTimer(const float& changeTitleTimer) { changeTitleTimer_ = changeTitleTimer; }
	void ChangeToTitle(const bool& changeTitle) { changeTitleFlag_ = changeTitle; }
	void SetWaveStartTimer(const float& timer)		{ waveStartTimer_ = timer; }
	void SetGameOver(const bool& gameOver)			{ isGameOver_ = gameOver; }
	void SetGameClear(const bool& gameClear)		{ isGameClear_ = gameClear; }
	void SetIsResult(const bool& isResult)			{ isResult_ = isResult; }

	float	GetWaveStartTimer()		{ return waveStartTimer_; }
	float	GetChangeTitleTimer()	{ return changeTitleTimer_; }
	bool	GetIsResult()			{ return isResult_; }

private:
	//	シャドウマップ
	void MakeShadow();	//	シャドウ生成
	void DrawShadow();	//	シャドウ描画

private:
	/* ----- オブジェクト ----- */
	std::unique_ptr	<Stage>		stage_;								//	ステージ
	std::unique_ptr	<Player>	player_;							//	プレイヤー
	std::unique_ptr	<Dragonkin>	dragonkin_;							//	竜人
	std::unique_ptr<StateMachine<State<SceneGame>>>	stateMachine_;	//	ステートマシン

	/* ----- 描画関係 ----- */
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceViews_[8];
	std::unique_ptr<FrameBuffer>				framebuffers_[8];
	std::unique_ptr<FullScreenQuad>				fullScreenQuad_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShaders_[8];
	float										nearZ_ = 50.0f;
	float										farZ_ = 400000.0f;
	Microsoft::WRL::ComPtr <ID3D11Buffer>		sceneConstantBuffer_;	//	シーン定数バッファ
	DirectX::XMFLOAT4							lightDirection_ = { +0.63f, -0.67f, 0.12f, 0.0f };	//	ディレクショナルライトの方向

	//	ブルーム
	std::unique_ptr<Bloom> bloomer_;

	//	シャドウマップ
	std::unique_ptr<CascadedShadowMaps> cascadedShadowMaps_;
	float criticalDepthValue_ = 100.0f; // If this value is 0, the camera's far panel distance is used.

	//	カラーフィルター
	std::unique_ptr<ColorFilter> colorFilter_;

	//	色収差
	std::unique_ptr<ChromaticAberration> chromaticAberration_;

	//	露出フィルター
	std::unique_ptr<ExposureFilter> exposureFilter_;

	//	コントラストフィルター
	std::unique_ptr<ContrastFilter> contrastFilter_;

	//	シャープネスフィルター
	std::unique_ptr<SharpenFilter> sharpenFilter_;

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
	std::unique_ptr <Sprite>			  sprites_[static_cast<int>(SPRITE_GAME::Max)];

	/* ----- ゲーム内で使う変数 ----- */
	float	waveStartTimer_		= 0.0f;		//	ウェーブ開始のUIが表示されている間
	bool	isGameOver_			= false;
	bool	changeTitleFlag_	= false;
	bool	isGameClear_		= false;
	float	changeTitleTimer_	= 3.0f;
	bool	isResult_			= false;	//	リザルト画面かどうか(クリア、ゲームオーバー)

	//	デバッグ用
private:
	bool drawUI_ = true;
};

