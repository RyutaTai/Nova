#include "EffectManager.h"

#include "../Graphics/Graphics.h"

//	初期化
void EffectManager::Initialize()
{
	Graphics& graphics = Graphics::Instance();

	//	Effekseerレンダラ生成
	effekseerRenderer_ = EffekseerRendererDX11::Renderer::Create(graphics.GetDevice(), graphics.GetDeviceContext(), 2048);

	//	Effekseerマネージャー生成
	effekseerManager_ = Effekseer::Manager::Create(2048);

	//	Effekseerレンダラの各種生成(特別なカスタマイズをしない場合は定型的に以下の設定でOK)
	effekseerManager_->SetSpriteRenderer(effekseerRenderer_->CreateSpriteRenderer());
	effekseerManager_->SetRibbonRenderer(effekseerRenderer_->CreateRibbonRenderer());
	effekseerManager_->SetRingRenderer(effekseerRenderer_->CreateRingRenderer());
	effekseerManager_->SetTrackRenderer(effekseerRenderer_->CreateTrackRenderer());
	effekseerManager_->SetModelRenderer(effekseerRenderer_->CreateModelRenderer());
	//	Effekseer内でのローダーの設定(特別なカスタマイズをしない場合は定型的に以下の設定でOK)
	effekseerManager_->SetTextureLoader(effekseerRenderer_->CreateTextureLoader());
	effekseerManager_->SetModelLoader(effekseerRenderer_->CreateModelLoader());
	effekseerManager_->SetMaterialLoader(effekseerRenderer_->CreateMaterialLoader());

	//	Effekseerを左手座標系で計算する
	effekseerManager_->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);

}

//	終了化
void EffectManager::Finalize()
{
	//	EffektManagerなどはスマートポインタによって破棄されるので何もしない
}

//	更新処理
void EffectManager::Update(const float& elapsedTime)
{
	//	エフェクト更新処理(引数にはフレームの経過時間を渡す)
	effekseerManager_->Update(elapsedTime * 60.0f);
}

//	描画処理
void EffectManager::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	//	ビュー&プロジェクション行列をEffekseerレンダラに設定
	effekseerRenderer_->SetCameraMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&view));
	effekseerRenderer_->SetProjectionMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&projection));

	//	Effekseer描画開始
	effekseerRenderer_->BeginRendering();

	//	Effekseer描画実行
	//	マネージャー単位で描画するので描画順を制御する場合はマネージャーを複数個作成し、
	//	Draw()関数を実行する順序で制御できそう
	effekseerManager_->Draw();

	//	Effekseer描画終了
	effekseerRenderer_->EndRendering();
}