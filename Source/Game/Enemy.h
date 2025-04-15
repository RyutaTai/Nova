#pragma once

#include "Character.h"
#include "../Nova/Others/JudgeTime.h"

class Enemy :public Character
{
public:
	enum class EnemyType	//	Enemyの種類
	{
		Drone = 0,
		Dragonkin,
		Max,
	};
	EnemyType myType_;

public:
	Enemy(const std::string& filename, const std::string& rootNodeName = "root");
	~Enemy()override {}

	virtual void	Initialize()	override = 0;
	virtual void	Update(const float& elapsedTime)override = 0;
	virtual void	Render()		override = 0;
	virtual void	DrawDebug()		override = 0;
	virtual void	DrawDebugPrimitive() = 0;
	
	//virtual bool OnMessage(const Telegram& msg);							//	メッセージ受信関数

	//	----- 旋回処理 -----
	virtual void Turn(const float& elapsedTime);

	//	----- 破棄処理 -----
	virtual void Destroy();

	//	----- プレイヤー探索 -----
	virtual bool SearchPlayer();
	const float GetSearchRange()const { return searchRange_; }

	//	----- 敵の種類 -----
	void		SetMyType(const EnemyType& myType) { myType_ = myType; }	//	自分の種類設定
	EnemyType	GetMyType()const { return myType_; }						//	敵の種類取得

	//	----- Collision -----
	virtual void RegisterCollisionData()override = 0;
	virtual void UpdateCollisions(const float& elapsedTime) = 0;

	//	----- ダメージ処理 -----
	void			SetIsDamaged(const bool& isDamaged)	{ isDamaged_ = isDamaged; }	//	ダメージフラグ設定
	bool			IsDamaged()const				{ return isDamaged_; }			//	ダメージフラグ取得
	virtual void	AddDamage(const float& damage);

	//	----- 死亡処理 -----
	virtual void OnDead() {}

	//	----- ターゲット -----
	void				UpdateTargetPosition();	//	ターゲット位置更新
	virtual void		SetRandomTargetPosition();																//	ターゲット位置をランダム設定
	void				SetTargetPosition(const DirectX::XMFLOAT3& position)	{ targetPosition_ = position; }	//	ターゲットポジション設定
	DirectX::XMFLOAT3	GetTargetPosition()const								{ return targetPosition_; }		//	ターゲットポジション取得
	const	float		CalcDistanceToTarget();						//	ターゲットまでの距離
	virtual void		ApproachingTarget(const float& elapsdTime);	//	ターゲットに接近する

	//	----- ステートタイマー -----
	void				UpdateRunTimer(const float& elapsedTime)	{ runTimer_ += elapsedTime; }	//	ステートタイマー更新
	void				SetRunTimer(const float& timer)				{ runTimer_ = timer; }			//	ステートタイマー設定
	void				ResetRunTimer()								{ runTimer_ = 0.0f; }			//	ステートタイマー初期化
	float				GetRunTimer()const							{ return runTimer_; }			//	ステートタイマー取得

	bool				IsUseOffsetY()const { return useOffsetY_; }

protected:
	//	----- ターゲット -----
	DirectX::XMFLOAT3	targetPosition_		=	{ 0.0f,0.0f,0.0f };				//	ターゲット位置

	//	----- 索敵範囲 -----
	DirectX::XMFLOAT3	territoryOrigin_	=	{ 0.0f,0.0f,0.0f };				//	索敵範囲の原点
	float				territoryRange_		=	10.0f;							//	索敵範囲
	float				searchRange_		=	13.0f;							//	索敵距離

	float				runTimer_			=	0.0f;							//
	bool				isDamaged_			=	false;							//	攻撃を受けたかどうか

	bool				useOffsetY_			=	true;	//	当たり判定でY方向のオフセット値を使うか
};

