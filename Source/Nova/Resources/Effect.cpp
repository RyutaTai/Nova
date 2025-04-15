#include "Effect.h"

#include "EffectManager.h"
#include "../Graphics/Graphics.h"

//	コンストラクタ
Effect::Effect(const char* filename)
{
	//	エフェクトを読み込みする前にロックする
	//	※マルチスレッドでEffectを作成するとDeviceContextを同時アクセスして
	//	フリーズする可能性があるので排他制御する
	std::lock_guard<std::mutex> lock(Graphics::Instance().GetMutex());

	//	Effekseerのリソースを読み込む
	//	EffekseerはUTF-16のファイルパス以外は対応していないため文字コード変換が必要
	char16_t utf16filename[256];
	Effekseer::ConvertUtf8ToUtf16(utf16filename, 256, filename);

	//	Effekseer::Managerを取得
	Effekseer::ManagerRef effekseerManager_ = EffectManager::Instance().GetEffecseerManager();

	//	Effekseerエフェクトを読み込み
	effekseerEffect_ = Effekseer::Effect::Create(effekseerManager_, (EFK_CHAR*)utf16filename);

	_ASSERT_EXPR(effekseerEffect_.Get(), L"Failed to load effect.");

}

//	停止
void Effect::Stop(Effekseer::Handle handle)
{
	Effekseer::ManagerRef effekseerManager_ = EffectManager::Instance().GetEffecseerManager();

	effekseerManager_->StopEffect(handle);
}

//	座標設定
void Effect::SetPosition(Effekseer::Handle handle, const DirectX::XMFLOAT3& position)
{
	Effekseer::ManagerRef effekseerManager_ = EffectManager::Instance().GetEffecseerManager();

	effekseerManager_->SetLocation(handle, position.x, position.y, position.z);
}

//	スケール設定
void Effect::SetScale(Effekseer::Handle handle, const DirectX::XMFLOAT3& scale)
{
	Effekseer::ManagerRef effekseerManager_ = EffectManager::Instance().GetEffecseerManager();

	effekseerManager_->SetScale(handle, scale.x, scale.y, scale.z);
}

//	再生
Effekseer::Handle Effect::Play(const DirectX::XMFLOAT3& position, const float& scale)
{
	_ASSERT_EXPR(effekseerEffect_.Get(), L"Failed to load effect.");

	Effekseer::ManagerRef effekseerManager_ = EffectManager::Instance().GetEffecseerManager();

	Effekseer::Handle handle = effekseerManager_->Play(effekseerEffect_, position.x, position.y, position.z);
	effekseerManager_->SetScale(handle, scale, scale, scale);
	return handle;
}

//	プレイをスレッド化した実装
void Effect::PlayAsync(const DirectX::XMFLOAT3& position, const float& scale)
{
	_ASSERT_EXPR(effekseerEffect_.Get(), L"Failed to load effect.");

	//	ロックしてプレイを実行
	std::lock_guard<std::mutex> lock(playMutex_);

	Effekseer::ManagerRef effekseerManager_ = EffectManager::Instance().GetEffecseerManager();
	Effekseer::Handle handle = effekseerManager_->Play(effekseerEffect_, position.x, position.y, position.z);
	effekseerManager_->SetScale(handle, scale, scale, scale);
}


