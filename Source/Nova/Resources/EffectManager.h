#pragma once

#include <DirectXMath.h>
#include <Effekseer/Effekseer.h>
#include <EffekseerRendererDX11/EffekseerRendererDX11.h>

//	エフェクトマネージャー
class EffectManager
{
private:
	EffectManager(){}
	~EffectManager(){}

public:
	//	唯一のインスタンス取得
	static EffectManager& Instance()
	{
		static EffectManager instance;
		return instance;
	}

	//	初期化
	void Initialize();

	//	終了化
	void Finalize();

	//	更新処理
	void Update(const float& elapsedTime);

	//	描画処理
	void Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	//	Effekseerマネージャーの取得
	Effekseer::ManagerRef GetEffecseerManager() { return effekseerManager_; }

private:
	Effekseer::ManagerRef			effekseerManager_;
	EffekseerRenderer::RendererRef	effekseerRenderer_;

};