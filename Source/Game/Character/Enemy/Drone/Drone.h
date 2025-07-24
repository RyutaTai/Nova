#pragma once

#include <memory>

#include "../../Enemy/Enemy.h"
#include "../../../../Nova/Resources/Effect.h"
#include "../../../Bullet/BulletManager.h"
#include "../../../../Nova/AI/StateMachine.h"
#include "../../../../Nova/Audio/AudioManager.h"
#include "../../../../Nova/Others/TimeRangeJudge.h"

//	ドローンクラス(浮いている敵)
class Drone :public Enemy
{
public:
	enum class StateType	//	ステートの種類
	{
		Idle = 0,			//	待機
		Search, 			//	サーチ
		Move,				//	移動
		Pursuit,			//	追跡
		Attack,				//	攻撃
		Avoidance,			//	回避
		Damage,				//	ダメージ
		Death,				//	死亡
		Max,				//	ステート最大数
	};

public:
	Drone();
	~Drone()override;

	void Initialize()override;
	void Update(const float& elapsedTime)override;
	void Render()override;

	//	----- 弾丸 -----
	void		LaunchBullet(const float& elapsedTime);				//	弾丸生成処理
	const float GetLaunchRange() const { return launchRange_; }	//	射程距離取得
	void		UpdateLaunchTimer(const float& elapsedTime) { launchTimer_ += elapsedTime; }
	void		ResetLaunchTimer() { launchTimer_ = 0.0f; }
	void		SetLaunchTimer(const float& launchTimer) { launchTimer_ = launchTimer; }
	const float GetLaunchTimer()const { return launchTimer_; }
	void		SetLaunchInterval(const float& launchInterval) { launchInterval_ = launchInterval; }
	const float GetLaunchInterval()const { return launchInterval_; }

	//	----- Collision -----
	void RegisterCollisionData(const std::string& jsonFileName)override;
	void UpdateCollisions(const float& elapsedTime)override;
	bool RayVsVertical(const float& elapsedTime)override;
	bool RayVsHorizontal(const float& elapsedTime)override;

	//	----- オーディオ -----
	void UpdateEmitter();		//	エミッター更新
	void UpdateAudioSource();	//	オーディオソース更新

	//	----- HP -----
	const int GetMaxHp()const { return MaxHp_; }		//	最大HP取得

	//	----- 死亡 -----
	void Destroy()override;		//	破棄処理
	void JudgeDeath();			//	死亡判定
	void OnDead()override;		//	死んだときに一回呼ばれる

	//	----- ステート -----
	void ChangeState(const StateType& state) { stateMachine_->ChangeState(static_cast<int>(state)); }	//	ステート遷移
	StateMachine<State<Drone>>* GetStateMachine() { return stateMachine_.get(); }	//	ステートマシン取得
	void ChangeDamageState();	//	ダメージステートへ遷移
	void DrawStateStr();		//	現在のステート文字列設定

	//	----- デバッグ描画 -----
	void DrawDebugPrimitive()override;			//	デバッグプリミティブ描画
	void DrawDebug()	override;				//	デバッグ描画

private:
	//	----- エフェクト -----
	std::shared_ptr <Effect>	effectResource_;	//	エフェクト
	float	effectScale_ = 0.8f;					//	エフェクトスケール
	
	// ----- 弾丸 -----
	float	launchTimer_	= 0.0f;		//	次の弾を発射するまでのタイマー
	float	launchInterval_ = 3.5f;		//	弾を発射する間隔
	float	launchRange_	= 10.0f;	//	射程距離

	//	----- オーディオ -----
	std::shared_ptr<SoundEmitter>	emitter_;		//	エミッター
	std::shared_ptr<AudioSource3D>	shotSE_;		//	弾丸の発射音

	//	----- ステート -----
	std::unique_ptr<StateMachine<State<Drone>>>	stateMachine_ = nullptr;		//	ステートマシン

private://	デバッグ用変数
	bool	isBulletLaunch_	= true;		//	弾丸発射処理をするかどうか
	static const int MaxHp_ = 40;		//	最大HP

	bool isHitStage_ = false;

};

