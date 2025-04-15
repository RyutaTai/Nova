#pragma once

#include <DirectXMath.h>
#include <Effekseer/Effekseer.h>
#include <thread>
#include <mutex>

//	エフェクト
class Effect
{
public:
	Effect(const char* filename);
	~Effect() {};

	//	再生
	Effekseer::Handle Play(const DirectX::XMFLOAT3& position, const float& scale = 1.0f);

	//	停止
	void Stop(Effekseer::Handle handle);

	//	座標設定
	void SetPosition(Effekseer::Handle handle, const DirectX::XMFLOAT3& position);

	//	スケール設定
	void SetScale(Effekseer::Handle handle, const DirectX::XMFLOAT3& scale);

	//	再生のスレッド化
	void PlayAsync(const DirectX::XMFLOAT3& position, const float& scale = 1.0f);

private:
	Effekseer::EffectRef	effekseerEffect_;
	std::mutex				playMutex_;			// スレッドで再生を行うためのロック

};
