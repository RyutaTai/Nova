#pragma once

#include <memory>

#include "../Character.h"
#include "../../../Nova/Resources/Effect.h"
#include "../../../Nova/AI/StateMachine.h"
#include "../../../Nova/Input/Input.h"
#include "../../../Nova/Audio/AudioSource.h"

//	プレイヤークラス
class Player :public Character
{
public:
	//	アニメーション情報
	enum class AnimationType
	{
		Idle = 0,		//	待機
		IdleCombat,		//	待機(構え状態)
		Walk,			//	歩き
		RunStart,		//	走り始め
		Run,			//	走り状態
		RunEnd,			//	走り終わり
		JumpVertical,	//	垂直ジャンプ	
		JumpFront,		//	前ジャンプ
		JumpRight,		//	右ジャンプ
		JumpBack,		//	後ろジャンプ
		JumpLeft,		//	左ジャンプ
		DoubleJamp,		//	2段ジャンプの2段目
		ComboOne1,		//	コンボ1_1
		ComboOne2,		//	コンボ1_2
		ComboOne3,		//	コンボ1_3
		ComboOne4,		//	コンボ1_4
		DodgeFront,		//	前回避
		DodgeRight,		//	右回避
		DodgeBack,		//	後ろ回避
		DodgeLeft,		//	左回避
		DodgeAirFront,	//	空中前回避
		DodgeAirRight,	//	空中右回避
		DodgeAirBack,	//	空中後ろ回避
		DodgeAirLeft,	//	空中左回避
		HitBack,		//	くらい(後ろからくらった)
		HitDeath,		//	くらい(後ろに吹っ飛ぶ)
		HitFront,		//	くらい(前からくらった)
		HitLeft,		//	くらい(左からくらった)
		HitRight,		//	くらい(右からくらった)
		Execution01,	//	コンボ2として使用
		GetUp,			//	起き上がり	

		Max,			//	アニメーション最大数
	};

	//	ステートの種類
	enum class StateType
	{
		Idle = 0, 		//	待機
		Move,			//	移動
		ComboOne1,		//	コンボ1_1
		ComboOne2,		//	コンボ1_2
		ComboOne3,		//	コンボ1_3
		ComboOne4,		//	コンボ1_4
		ComboTwo1,		//	コンボ2_1
		ComboTwo2,		//	コンボ2_2
		Dodge,			//	回避
		GetUp,			//	起き上がり
		Damage,			//	ダメージを受けた
		Flinch,			//	怯み
		Death,			//	死亡
		Max,			//	ステート最大数
	};

private:
	//	オーディオの種類
	enum class AudioStereo
	{
		Footsteps,	//	足音
		HitAttack,	//	攻撃ヒット音
		Max
	};

public:
	Player();
	~Player()override = default;

	static Player& Instance();

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;

	//	デバッグ
	void DrawDebug()override;	//	ImGui描画
	void DrawDebugPrimitive();	//	デバッグプリミティブ描画

	//	----- 移動入力処理 -----
	bool InputMove(const float& elapsedTime);
	
	//	----- エフェクト再生 -----
	void PlayEffect();

	//	----- 判定処理 -----
	bool RayVsVertical(const float& elapsedTime)override;		//	ステージとの当たり判定(垂直方向)
	bool RayVsHorizontal(const float& elapsedTime)override;		//	ステージとの当たり判定(水平方向)	

	//	----- エフェクト -----
	void SetEffectScale(const float& scale) { effectScale_ = scale; }
	void SetPlayEffectFlag(const bool& playEffect) { playEffectFlag_ = playEffect; }
	void SetEffectPos(const DirectX::XMFLOAT3& pos) { effectPos_ = pos; }
	const bool IsPlayEffect()const { return playEffectFlag_; }

	//	----- HP -----
	const float	GetMaxHp()		const { return MaxHp_; }
	//	----- ダメージ処理 -----
	void AddDamage(const float& damage) { hp_ -= damage; }

	//	----- ポーズ -----
	void		SetIsPose(const bool& isPose) { isPose_ = isPose; }
	const bool	GetPose()const { return isPose_; }

	//	----- オートコンボ -----
	void		SetAutoCombo(const bool& isAutoCombo)	{ isAutoCombo_ = isAutoCombo; }
	const bool	IsAutoCombo()const						{ return isAutoCombo_; }
	//	----- コンボ数 -----
	void		SetComboCount(const int& comboCount)	{ comboCount_ = comboCount; }
	void		AddComboCount()							{ comboCount_++; }
	void		ResetComboCount()						{ comboCount_ = 0; }
	const int	GetComboCount()const					{ return comboCount_; }

	//	----- Collision ----
	void RegisterCollisionData()override;
	void SetIsActiveCollisionDetection(const bool& isActiveCollisionDetection) { isActiveCollisionDetection_ = isActiveCollisionDetection; }
	const bool IsActiveCollisionDetection()const { return isActiveCollisionDetection_; }
	void UpdateCollisionDetectionData(const float& elapsedTime);

	//	----- 攻撃ヒットフラグ(自分の攻撃が相手に当たったか) -----
	void		SetAttackHit(const bool& isHit) { isAttackHit_ = isHit; }
	const bool	IsAttackHit()const { return isAttackHit_; }

	//	----- アニメーション -----
	void			PlayAnimation(const AnimationType& animType, const bool& loop = false, const float& blendTime = 1.0f, const float& animSpeed = 1.0f, const float& startFrame = 0.0f, const float& endFrame = 0.0f);
	int				GetCurrentAnimNum();						//	現在再生中のアニメーション番号取得
	AnimationType	GetCurrentAnimType();						//	現在再生中のアニメーションタイプ取得
	const float		GetCurrentAnimationSeconds();				//	現在のアニメーション再生時間取得
	const float		GetAnimationDuration(const AnimationType& animType);	//	アニメーションの長さ取得

	//	-----　移動方向取得 -----
	DirectX::XMFLOAT3					GetMoveVec()const;				//	スティック入力値から移動ベクトルを取得
	
	//	----- ステート -----
	StateMachine<State<Player>>*		GetStateMachine()	const { return stateMachine_.get(); }	//	ステートマシン取得
	void								ChangeState(const StateType& state);						//	ステート遷移
	void								ChangeDodgeState();											//	回避ステートへ遷移
	StateType							GetCurrentState()	const { return currentState_; }			//	現在のステート取得
	StateType							GetLastState()		const { return lastState_; }			//	ひとつ前のステート取得
	void								DrawStateStr();												//	現在のステート描画

	
	//	----- 攻撃してきた敵の位置 -----
	void SetEnemyPos(const DirectX::XMFLOAT3& enemyPos) { enemyPos_ = enemyPos; }
	const DirectX::XMFLOAT3 GetEnemyPos()const { return enemyPos_; }

private:
	static Player* instance_;

	//	----- State -----
	std::unique_ptr<StateMachine<State<Player>>>	stateMachine_ = nullptr;	//	ステートマシン
	StateType currentState_ = StateType::Idle;									//	現在のステート	
	StateType lastState_	= StateType::Idle;									//	ひとつ前のステート	

	//	----- エフェクト -----
	std::shared_ptr<Effect>		effectResource_;								//	エフェクト
	float						effectScale_ = 0.4f;							//	エフェクトスケール
	DirectX::XMFLOAT3			effectPos_ = {};								//	エフェクト再生位置
	bool						playEffectFlag_ = false;						//	エフェクト再生フラグ
	bool						drawEffectFlag_ = true;							//	エフェクト描画フラグ(falseなら描画しない)
	//AnimationType				currentAnimNum_;								//	現在のアニメーション番号
	
	//	----- プレイヤーのパラメータ -----
	static constexpr float MaxHp_ = 100.0f;											//	最大HP
	float				 turnSpeed_ = DirectX::XMConvertToRadians(720);			//	旋回速度

	//	----- Collision -----
	bool isActiveCollisionDetection_ = true;	//	押し出し判定が有効かどうか
	bool isAttackHit_ = false;					//	攻撃ヒットフラグ

	//	----- ポーズ -----
	bool isPose_ = false;		//	ポーズ中プレイヤーの操作を受け付けない
	
	//	----- コンボ -----
	bool isAutoCombo_ = false;	//	オートコンボ(デフォルトはfalseにする)

	//	----- ターゲット -----
	bool				isTraget_	= false;	//	ターゲットがいるか
	float				serchRange_ = 10.0f;	//	ターゲットを見つける範囲
	DirectX::XMFLOAT3	targetPos	= {};		//	ターゲット位置
	DirectX::XMFLOAT3	enemyPos_	= {};		//	攻撃してきた敵の位置

	//	----- オーディオ -----
	AudioSource* sources_[static_cast<int>(AudioStereo::Max)] = { nullptr };


private://	----- デバッグ用 -----
	//	----- Collision -----
		//	----- DebugPrimitive -----
	bool isCollisionSphere_ = true;
	bool isAttackSphere_ = true;
	bool isDamageSphere_ = false;

	//	----- コンボ -----
	int comboCount_ = 0;	//	コンボ攻撃が何連撃ヒットしたか

	//	ImGui用
	bool				isCollisionStage_	= true;
	bool				isHitStage_			= false;
	float				gravity_			= -5.0f;
	float				rayPosRadius_		= 0.01f;
	bool				isAddGravity_		= false;		//	重力加算フラグ

};