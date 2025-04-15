#pragma once

#include "../Game/BulletManager.h"
#include "../Nova/Resources/GltfModelStaticBatching.h"
#include "../Nova/Resources/Effect.h"
#include "../Nova/Audio/AudioSource3D.h"
#include "../Nova/Audio/AudioSource.h"
#include "../Nova/Collision/CollisionData.h"

//	弾丸クラス
class Bullet
{
public:
	enum class OpponentType	//	攻撃する相手の種類
	{
		Player,	//	プレイヤー
		Enemy,	//	敵
		None,	//	無し
		All,	//	全て
		Max
	};

public:
	Bullet(const std::string& filename);
	virtual ~Bullet() {}

	virtual void			Initialize();
	virtual void			Update(const float& elapsedTime);
	virtual void			Launch(const DirectX::XMFLOAT3& direction = {}, const DirectX::XMFLOAT3& position = {});
	virtual void			Render()			= 0;
	virtual void			RnederCoverModel()	= 0;
	
	Transform* GetTransform()const { return gltfStaticModelResource_->GetTransform(); }
	
	//	----- Collision -----
	const float	 GetRadius()const { return radius_; }				//	半径取得

	//	----- デバッグ描画 -----
	virtual void			DrawDebug();																//	デバッグ描画
	void					DrawDebugPrimitive();														//	デバッグプリミティブ描画

	//	----- 弾丸の所有者 -----
	void					SetOwnerPosition(const DirectX::XMFLOAT3& pos)	{ ownerPosition_ = pos; }	//	弾丸所有者の位置設定
	const DirectX::XMFLOAT3 GetOwnerPosition()								{ return ownerPosition_; }	//	弾丸所有者の位置取得
	
	//	----- カバーモデル -----
	virtual void			CoverModelUpdate(const float& elapsedTime);									//	カバーモデル更新処理
	Transform*				GetCoverTransform()const { return coverModel_->GetTransform(); }
	
	//	----- オーディオ -----
	void					UpdateEmitter();
	void					UpdateAudioSource();	//	オーディオソース更新処理
	
	//	----- 破棄処理 -----
	virtual void			Destroy();											//	破棄処理
	
	//	----- ダメージフラグ -----
	void					SetDamaged(const bool& damaged) { isDamaged_ = damaged; }					//	ダメージフラグ設定

	//	----- 無敵フラグ -----
	void					SetInvincible(const bool& isInvincible) { isInvincible_ = isInvincible; }	//	無敵フラグ設定

	//	----- 生存時間 -----
	void UpdateLifeTimer(const float& elapsedTime);

	//	----- 攻撃力 -----
	void SetAttackPower(const float& attackPower) { attackPower_ = attackPower; }
	const float GetAttackPower()const { return attackPower_; }

	//	----- 攻撃する相手 -----
	void SetOpponentType(const OpponentType& opponentType) { opponentType_ = opponentType; }
	const OpponentType GetOpponentType()const { return opponentType_; }

	//	----- ターゲット位置 -----
	void SetTargetPos(const DirectX::XMFLOAT3& target) { targetPos_ = target; }
	const DirectX::XMFLOAT3 GetTargetPos()const { return targetPos_; }

private:
	enum class Audio3D	//	3Dで鳴らすSEの種類
	{
		Explosion = 0,			//	爆発音
		Move,					//	移動音
		Max
	};

protected:
	//	----- モデル -----
	std::shared_ptr<GltfModelStaticBatching>	gltfStaticModelResource_;		//	Gltfモデル
	std::unique_ptr<GltfModelStaticBatching>	coverModel_;					//	弾の周りを覆うモデル

	//	----- カバーモデル -----
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	coverPixelShader_;				//	弾丸のキューブのピクセルシェーダー
	
	//	----- 移動処理 -----
	DirectX::XMFLOAT3	velocity_ = {};		//	速度
	float				moveSpeed_ = 1.0f;	//	弾の速さ
	DirectX::XMFLOAT3	direction_ = {};	//	弾が飛ぶ方向

	//	----- Collision -----
	std::vector<AttackDetectionData>	attackDetectionData_;		//	攻撃判定用
	std::vector<DamageDetectionData>	damageDetectionData_;		//	くらい判定
	std::vector<CollisionDetectionData>	collisionDetectionData_;	//	押し出し判定用
	float								radius_ = 1.0f;				//	弾の半径

	//	----- ダメージ -----
	bool										isDamaged_ = false;				//	攻撃を受けたかどうか
	float										attackPower_ = 5.0f;			//	攻撃力

	//	----- 弾丸の所有者 -----
	DirectX::XMFLOAT3							ownerPosition_ = {};			//	弾丸所有者の位置

	//	----- 攻撃する相手 -----
	OpponentType		opponentType_;		//	攻撃相手の種類
	DirectX::XMFLOAT3	targetPos_ = {};	//	ターゲット位置

	//	----- 無敵処理 -----
	bool isInvincible_ = false;

	//	----- 生存時間 -----
	float lifeTimer_ = 2.5f;

	//	----- オーディオ -----
	SoundEmitter emitter_ = {};											//	エミッターを自分の位置で持つ
	AudioSource3D* se_[static_cast<int>(Audio3D::Max)] = { nullptr };	//	弾丸のSE(3Dで鳴らす)
	AudioSource* debugSE_ = nullptr;

	//	----- 更新フラグ -----
	bool updateFlag_ = true;

private:
	//	----- エフェクト -----
		enum class EffectType
		{
			Fire = 0,		//	弾の周りのエフェクト
			Explosion,		//	爆発
			Max,			//	最大数
		};

private:
	//	----- エフェクト -----
	std::shared_ptr <Effect>	effectResource_[static_cast<int>(EffectType::Max)];			//	エフェクトリソース
	float						effectScale_[static_cast<int>(EffectType::Max)] = { 1.0f };	//	エフェクトスケール


};

