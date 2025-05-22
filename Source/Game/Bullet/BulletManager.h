#pragma once

#include <vector>
#include <set>

class Bullet;

//	弾丸管理クラス
class BulletManager
{
public:
	BulletManager();
	~BulletManager();

	static BulletManager& Instance()
	{
		static BulletManager instance;
		return instance;
	}

	//	初期化処理
	void Initialize();

	//	更新処理
	void Update(const float& elapsedTime);

	//	カバーモデルスケール更新処理
	void CoverModelUpdate(const float& elapsedTime);

	//	描画処理
	void Render();

	//	シャドウマップ
	void CastShadows();

	//	弾丸登録
	void Register(Bullet* bullet);

	//	弾丸全削除
	void Clear();

	//	カバーモデルスケールセット
	void SetCoverScale(const float& coverScale) { coverScale_ = coverScale; }

	//	カバーモデルスケール取得
	const float& GetCoverScale() const { return coverScale_; }

	//	無敵状態セット
	void SetInvincible(const bool& isInvincible);

	//	弾丸取得
	std::vector<Bullet*> GetBullets() { return bullets_; }

	//	弾丸取得
	Bullet* GetBullet(const int& index) { return bullets_.at(index); }

	//	弾丸数取得
	int GetBulletCount()const { return static_cast<int>(bullets_.size()); }
	
	//	弾丸削除
	void Remove(Bullet* projectile);

	//	デバッグプリミティブ描画
	void DrawDebugPrimitive();

	//	デバッグ描画
	void DrawDebug();

private:
	std::vector	<Bullet*>	bullets_;
	std::set	<Bullet*>	generates_;
	std::set	<Bullet*>	removes_;

	float					coverScale_ = 0.1f;		//	カバーモデルのスケール
	float					scaleAdd_ = 0.4f;		//	どのくらいscaleを加算するか
	const float				CoverScaleMax_ = 0.48f;	//	スケール上限
	const float				CoverScaleMin_ = 0.28f;	//	スケール下限	小さすぎると弾に埋まって見えない

	float					coverScaleTimer_ = 0.0f;

private:	
	//	デバッグ用の変数
	bool isInvincible_ = false;			//	無敵状態
	bool isBulletDestroy_ = false;		//	弾丸破棄
	bool coverModelDraw_ = true;		//	カバーモデル描画フラグ
	bool isCoverModelUpdate_ = true;	//	カバーモデル更新処理

};