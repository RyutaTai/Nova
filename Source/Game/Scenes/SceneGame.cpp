#include "SceneGame.h"

#include "../../Nova/Core/Framework.h"
#include "../../Nova/Graphics/Graphics.h"
#include "../../Nova/Camera/Camera.h"
#include "../Scenes/SceneManager.h"
#include "../Scenes/SceneTitle.h"
#include "../Scenes/SceneLoading.h"
#include "../../Nova/Input/GamePad.h"
#include "../../Nova/Resources/EffectManager.h"
#include "../../Nova/Others/MathHelper.h"
#include "../../Game/Character/Enemy/EnemyManager.h"
#include "../../Game/Scenes/GameState.h"
#include "../../Game/UI/UIManager.h"
#include "../../Game/UI/UIHealth.h"
#include "../../Game/UI/UIInstructions.h"
#include "../../Game/UI/UITempo.h"
#include "../../Game/UI/UIRank.h"
#include "../../Game/Rhythm/JudgeRhythm.h"
#include "../../Nova/Collision/CollisionManager.h"

//	初期化
void SceneGame::Initialize()
{
	// ----- オーディオ初期化 -----
	std::shared_ptr<AudioSource> gameBGM = AudioManager::Instance().LoadAudioSource("./Resources/Audio/BGM/Game.wav", Audio::AudioType::BGMNormal, "GameScene");
	gameBGM->SetVolume(0.3f, false);
	gameBGM->SetAudioName("GameBGM");
	AudioManager::Instance().AudioRegister(gameBGM);

	// ----- スプライト初期化 -----
	sprites_[static_cast<int>(SPRITE_GAME::Clear)]	  = std::make_unique<Sprite>(L"./Resources/Image/Clear.png");
	sprites_[static_cast<int>(SPRITE_GAME::GameOver)] = std::make_unique<Sprite>(L"./Resources/Image/GameOver.png");

	// ----- UI初期化 -----
	std::unique_ptr<UIHealth> uiHealth = std::make_unique<UIHealth>();
	UIManager::Instance().Register(std::move(uiHealth));

	std::unique_ptr<UITempo> uiTempo = std::make_unique<UITempo>();
	UIManager::Instance().RegisterUITempo(uiTempo.get());
	UIManager::Instance().Register(std::move(uiTempo));
	
	std::unique_ptr<UIRank> uiRank = std::make_unique<UIRank>();
	UIManager::Instance().RegisterUIRank(uiRank.get());
	UIManager::Instance().Register(std::move(uiRank));
	//	登録し終わってから初期化処理をする
	UIManager::Instance().Initialize();

	// ----- Rhythmクラス初期化 -----
	JudgeRhythm::Instance().Initialize();

	// ----- ステージ初期化 -----
	stage_ = std::make_unique<Stage>();

	// ----- シーン定数バッファ -----
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = (sizeof(Graphics::SceneConstants));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, sceneConstantBuffer_.GetAddressOf());

	// ----- カメラ初期化 -----
	Camera::Instance().Initialize();

	// ----- プレイヤー初期化 -----
	player_ = std::make_unique<Player>();
	player_->Initialize();

	// ----- エネミー初期化 -----
	dragonkin_ = std::make_unique<Dragonkin>();
	dragonkin_->Initialize();
	EnemyManager::Instance().Register(std::move(dragonkin_));

	// ----- テクスチャ読み込み -----
	D3D11_TEXTURE2D_DESC texture2dDesc = {};
	ID3D11Device* device = Graphics::Instance().GetDevice();
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/sunset_jhbcentral_4k/sunset_jhbcentral_4k.dds",
		shaderResourceViews_[0].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/sunset_jhbcentral_4k/diffuse_iem.dds",
		shaderResourceViews_[1].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/sunset_jhbcentral_4k/specular_pmrem.dds",
		shaderResourceViews_[2].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/lut_charlie.dds",
		shaderResourceViews_[3].GetAddressOf(), &texture2dDesc);

	//	----- ブルーム -----
	framebuffers_[0] = std::make_unique<FrameBuffer>(device, SCREEN_WIDTH, SCREEN_HEIGHT);
	framebuffers_[1] = std::make_unique<FrameBuffer>(device, SCREEN_WIDTH, SCREEN_HEIGHT);
	fullScreenQuad_ = std::make_unique<FullScreenQuad>(device);
	bloomer_ = std::make_unique<Bloom>(device, SCREEN_WIDTH, SCREEN_HEIGHT);
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/FinalPassPs.cso", pixelShaders_[static_cast<int>(PixelShaderType::FinalPass)].ReleaseAndGetAddressOf());

	//	----- シャドウ -----
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/CascadedShadowPs.cso", pixelShaders_[static_cast<int>(PixelShaderType::Shadow)].GetAddressOf());
	cascadedShadowMaps_ = std::make_unique<CascadedShadowMaps>(device, 1024 * 4, 1024 * 4);

	//	----- カラーフィルター -----
	colorFilter_ = std::make_unique<ColorFilter>();

	//	----- 色収差 -----
	chromaticAberration_ = std::make_unique<ChromaticAberration>();

	//	----- 露出フィルター -----
	exposureFilter_ = std::make_unique<ExposureFilter>();

	//	----- シャープネスフィルター -----
	sharpenFilter_ = std::make_unique<SharpenFilter>();

	//	----- コントラストフィルター -----
	contrastFilter_ = std::make_unique<ContrastFilter>();

	//	----- ステート登録 -----
	stateMachine_.reset(new StateMachine<State<SceneGame>>());
	stateMachine_->RegisterState(new GameState::Wave1State(this));		//	Wave1
	stateMachine_->RegisterState(new GameState::Wave2State(this));		//	Wave2
	stateMachine_->RegisterState(new GameState::Wave3State(this));		//	Wave3
	stateMachine_->RegisterState(new GameState::GameClearState(this));	//	ゲームクリア
	stateMachine_->RegisterState(new GameState::GameOverState(this));	//	ゲームオーバー
	//	初期ステート設定
	stateMachine_->SetState(static_cast<int>(SceneGameState::Wave1));	//	初期ステートセット

}

//	更新処理
void SceneGame::Update(const float& elapsedTime)
{
	GamePad& gamePad = Input::Instance().GetGamePad();

	// ----- カメラ更新処理 -----
	DirectX::XMFLOAT3 cameraTarget = player_->GetTransform()->GetPosition();
	cameraTarget.y += player_->GetHeight() / 2.0f;
	Camera::Instance().SetTargetPos(cameraTarget);
	Camera::Instance().Update(elapsedTime);

	// ----- ステートマシン更新処理 -----
	stateMachine_->Update(elapsedTime);

	// ----- ステージ更新処理 -----
	stage_->Update(elapsedTime);

	// ----- プレイヤー更新処理 -----
	player_->Update(elapsedTime);

	// ----- エネミー更新処理 -----
	EnemyManager::Instance().Update(elapsedTime);

	// ----- エフェクト更新処理 -----
	EffectManager::Instance().Update(elapsedTime);

	// ----- UI更新処理 -----
	UIManager::Instance().Update(elapsedTime);

	// ----- Rhythm更新処理 -----
	JudgeRhythm::Instance().Update();

	// ----- Collision更新処理 -----
	CollisionManager::Instance().Update(elapsedTime);

	//	----- ヴィネット更新処理 -----
	Vignette::Instance().LerpVignetteIntensity(elapsedTime);

	//	----- カラーフィルター更新処理 -----
	colorFilter_->Update();

	//	----- 色収差更新処理 -----
	chromaticAberration_->Update();

	//	----- コントラストフィルター更新処理 -----
	contrastFilter_->Update();

	//	----- 露出フィルター更新処理 -----
	exposureFilter_->Update();

	//	----- シャープネスフィルター更新処理 -----
	sharpenFilter_->Update();

	//	ゲームクリアへの遷移はWeve3 State内で行っている	
	//	ゲームオーバーへの遷移
	float playerHp = player_->GetHp();
	if (playerHp <= 0.0f && stateMachine_->GetCurrentStateIndex() != static_cast<int>(SceneGameState::GameOver))
	{
		AudioManager::Instance().GetAudioResource("GameBGM")->SetVolume(0.1f, false);
		ChangeState(SceneGameState::GameOver);
	}

	//	タイトルへ遷移
	if (changeTitleFlag_)
	{
		SceneManager::Instance().ChangeScene(new SceneTitle);
	}
	
}

//	ウェーブ画像読み込み
void SceneGame::LoadWaveSprite(const wchar_t* filename)
{
	sprites_[SPRITE_GAME::WAVE] = std::make_unique<Sprite>(filename);
}

//	描画処理
void SceneGame::Render()
{
	//	レンダー初期設定
	SetupRender();
	
	//	シーン定数バッファ更新
	UpdateSceneConstants();

	// メインの3Dシーンとポストエフェクトの描画
	Render3DScene();

	// ----- エフェクト描画 -----
	RenderEffect();

	// ----- デバッグプリミティブ描画 -----
	RenderDebugPrimitive();

	// ----- スプライト描画 -----
	RenderSprite();

	// ----- UI描画 -----
	UIManager::Instance().Render();

}

//	レンダー初期設定
void SceneGame::SetupRender()
{
	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
	ID3D11ShaderResourceView* nullShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	deviceContext->VSSetShaderResources(0, _countof(nullShaderResourceViews), nullShaderResourceViews);
	deviceContext->PSSetShaderResources(0, _countof(nullShaderResourceViews), nullShaderResourceViews);

	//	パースペクティブ設定
	Camera::Instance().SetPerspectiveFov();

}

//	シーン定数バッファ更新
void SceneGame::UpdateSceneConstants()
{
	Graphics::Instance().SetViewProjection(Camera::Instance().CalcViewProjectionMatrix());
	Graphics::Instance().SetLightDirection(lightDirection_);
	Graphics::Instance().SetCameraPosition({ 0,0,1,0 });
	Graphics::Instance().SetInvViewProjection(Camera::Instance().CalcInvViewProjectionMatrix());
	Graphics::Instance().SetInvProjection(Camera::Instance().CalcInvProjectionMatrix());

	Graphics::SceneConstants sceneConstants = Graphics::Instance().GetSceneConstant();
	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
	deviceContext->UpdateSubresource(sceneConstantBuffer_.Get(), 0, 0, &sceneConstants, 0, 0);
	deviceContext->VSSetConstantBuffers(1, 1, sceneConstantBuffer_.GetAddressOf());
	deviceContext->PSSetConstantBuffers(1, 1, sceneConstantBuffer_.GetAddressOf());

}

// メインの3Dシーンとポストエフェクトの描画
void SceneGame::Render3DScene()
{
	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();

	//	ステート設定（モデル描画用）
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

	//	シーン定数バッファのカメラポジション設定
	DirectX::XMFLOAT4 cameraPosition = { Camera::Instance().GetEye().x, Camera::Instance().GetEye().y, Camera::Instance().GetEye().z, 1.0f };
	Graphics::Instance().SetCameraPosition(cameraPosition);

	//	ビューポートの取得は、通常は描画ターゲットのアクティベート時に設定されるため、
	//	ここで取得する必要があるか再確認 (SetViewportsで設定し直す場合は意味がある)
	D3D11_VIEWPORT viewport;
	UINT numViewports{ 1 };
	deviceContext->RSGetViewports(&numViewports, &viewport);

	//	カメラの投影行列を再度設定（冗長な可能性あり）
	Camera::Instance().SetPerspectiveFov();
	DirectX::XMMATRIX Projection = Camera::Instance().GetProjectionMatrix();
	DirectX::XMVECTOR Eye{ DirectX::XMLoadFloat3(&Camera::Instance().GetEye()) };
	DirectX::XMVECTOR Focus{ DirectX::XMLoadFloat3(&Camera::Instance().GetFocus()) };
	DirectX::XMVECTOR Up{ DirectX::XMLoadFloat3(&Camera::Instance().GetUp()) };
	DirectX::XMMATRIX V{ DirectX::XMMatrixLookAtLH(Eye, Focus, Up) };
	Graphics::Instance().SetViewProjection(V * Projection);

	//	----- 最初のオフスクリーンパス: シーンの3Dモデルを描画	-----
	framebuffers_[0]->Clear(deviceContext);
	framebuffers_[0]->Activate(deviceContext); // framebuffers_[0]をレンダーターゲットに設定

	//	環境マップなどのシェーダーリソースをピクセルシェーダーにバインド
	deviceContext->PSSetShaderResources(32, 1, shaderResourceViews_[0].GetAddressOf());
	deviceContext->PSSetShaderResources(33, 1, shaderResourceViews_[1].GetAddressOf());
	deviceContext->PSSetShaderResources(34, 1, shaderResourceViews_[2].GetAddressOf());
	deviceContext->PSSetShaderResources(35, 1, shaderResourceViews_[3].GetAddressOf());

	//	各モデルの描画
	//	もしモデル個別に異なるステートが必要なら、そのモデルのRender関数内で設定しておく
	stage_->Render();
	player_->Render();
	EnemyManager::Instance().Render();

	//	元のレンダーターゲットに戻す
	framebuffers_[0]->Deactivate(deviceContext);

	//	----- ポストエフェクト処理 -----
	//	ブルームの生成
	bloomer_->Make(deviceContext, framebuffers_[0]->shaderResourceViews_[0].Get());

	//	シャドウマップの生成
	MakeShadow(); //	この関数内でシャドウマップ用のレンダーパスが行われる

	//	シャドウ描画パス（framebuffers_[1]へ）
	framebuffers_[1]->Clear(deviceContext);
	framebuffers_[1]->Activate(deviceContext);
	DrawShadow(); // framebuffers_[0]の結果とシャドウマップを合成し、framebuffers_[1]に描画
	framebuffers_[1]->Deactivate(deviceContext);

	//	ヴィネット効果の生成
	Vignette::Instance().Make();

	//	----- 最終パス: 全てのポストエフェクトを合成して画面に描画 -----
	//	最終合成用のレンダー設定
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

	//	合成に必要なすべてのテクスチャをシェーダーにバインド
	ID3D11ShaderResourceView* shaderResourceViews[] =
	{
		framebuffers_[1]->shaderResourceViews_[0].Get(),	//	colorMap (シャドウ適用後のメインシーンカラー)
		bloomer_->ShaderResourceView(),						//	bloom (ブルーム効果)
		framebuffers_[1]->shaderResourceViews_[1].Get(),	//	depthMap (メインシーンの深度)
		cascadedShadowMaps_->DepthMap().Get()				//	cascadedShadowMap (シャドウマップ)
	};
	//	フルスクリーンクアッドで最終ピクセルシェーダーを適用し、画面に描画
	fullScreenQuad_->Blit(deviceContext, shaderResourceViews, 0, _countof(shaderResourceViews), pixelShaders_[static_cast<int>(PixelShaderType::FinalPass)].Get());

}

//	エフェクト描画
void SceneGame::RenderEffect()
{
	//	ステート設定
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::SOLID);
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

	//	描画
	DirectX::XMFLOAT4X4 view;
	DirectX::XMStoreFloat4x4(&view, Camera::Instance().GetViewMatrix());
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMStoreFloat4x4(&projection, Camera::Instance().GetProjectionMatrix());
	EffectManager::Instance().Render(view, projection);

}

//	デバッグプリミティブ描画
void SceneGame::RenderDebugPrimitive()
{
#if _DEBUG
	player_->DrawDebugPrimitive();
	EnemyManager::Instance().DrawDebugPrimitive();
	Graphics::Instance().GetDebugRenderer()->Render();
#endif
}

//	スプライト描画
void SceneGame::RenderSprite()
{
	//	手前にスプライト出すならZON_ON、奥に描画ならOFF_OFF
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

	//	ゲームクリア
	if (isGameClear_)
	{
		sprites_[static_cast<int>(SPRITE_GAME::Clear)]->GetTransform()->SetPosition(320, 180);
		sprites_[static_cast<int>(SPRITE_GAME::Clear)]->Render();
	}

	//	ゲームオーバー
	if (isGameOver_)
	{
		sprites_[static_cast<int>(SPRITE_GAME::GameOver)]->GetTransform()->SetPosition(320, 180);
		sprites_[static_cast<int>(SPRITE_GAME::GameOver)]->Render();
	}

}


//	シャドウ生成
void SceneGame::MakeShadow()
{
	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
	DirectX::XMFLOAT4X4 cameraView;
	DirectX::XMStoreFloat4x4(&cameraView, Camera::Instance().GetViewMatrix());
	DirectX::XMFLOAT4X4 cameraProjection;
	DirectX::XMStoreFloat4x4(&cameraProjection, Camera::Instance().GetProjectionMatrix());
	cascadedShadowMaps_->Clear(deviceContext);
	static constexpr int CsmCBIndex = 3; //	カスケードシャドウマップ用定数バッファのレジスタ番号
	cascadedShadowMaps_->Activate(deviceContext, cameraView, cameraProjection, lightDirection_, criticalDepthValue_, CsmCBIndex);
	
	//	ステート設定
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::NONE);

	stage_->CastShadows();
	player_->CastShadows();
	EnemyManager::Instance().CastShadows();
	BulletManager::Instance().CastShadows();

	cascadedShadowMaps_->Deactivate(deviceContext);
}

//	シャドウ描画
void SceneGame::DrawShadow()
{
	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::NONE);
	ID3D11ShaderResourceView* shaderResourceViews[]
	{
		framebuffers_[0]->shaderResourceViews_[0].Get(),	//	colorMap
		framebuffers_[0]->shaderResourceViews_[1].Get(),	//	DepthMap
		cascadedShadowMaps_->DepthMap().Get()				//	cascadedShadowMaps
	};
	fullScreenQuad_->Blit(deviceContext, shaderResourceViews, 0, _countof(shaderResourceViews), pixelShaders_[static_cast<int>(PixelShaderType::Shadow)].Get());

}

//	終了化
void SceneGame::Finalize()
{
	//	エネミーマネージャー終了化
	EnemyManager::Instance().Clear();

	//	UIマネージャー終了化
	UIManager::Instance().Finalize();

	//	テクスチャ解放
	ReleaseAllTextures();
}

//	デバッグ描画
void SceneGame::DrawDebug()
{
	D3D11_VIEWPORT viewport;
	UINT numViewports{ 1 };
	Graphics::Instance().GetDeviceContext()->RSGetViewports(&numViewports, &viewport);

	//	----- DebugRenderer -----
	Graphics::Instance().GetDebugRenderer()->DrawDebugGUI();

	//	----- SceneConstant -----
	if (ImGui::TreeNode("SceneConstant"))
	{
		ImGui::DragFloat4("LightDirection", &lightDirection_.x, 0.01f, -1.0f, 1.0f);	//	ライトの向き
		ImGui::TreePop();
	}

	//	----- ブルーム -----
	if (bloomer_)bloomer_->DrawDebug();

	//	----- シャドウ -----
	if (ImGui::TreeNode(u8"Shadow シャドウ"))
	{
		ImGui::DragFloat("CriticalDepthValue", &criticalDepthValue_, 0.1f);
		cascadedShadowMaps_->DrawDebug();
		ImGui::TreePop();
	}

	//	----- ヴィネット -----
	Vignette::Instance().DrawDebug();

	//	----- カラーフィルター -----
	colorFilter_->DrawDebug();

	//	----- 色収差 -----
	chromaticAberration_->DrawDebug();
 
	//	----- 露出フィルター -----
	exposureFilter_->DrawDebug();

	//	----- シャープネスフィルター -----
	sharpenFilter_->DrawDebug();

	//	----- コントラストフィルター -----
	contrastFilter_->DrawDebug();

	//	----- カメラ -----
	Camera::Instance().DrawDebug();
	
	//	----- プレイヤー -----
	player_->DrawDebug();

	//	----- ステージ -----
	stage_->DrawDebug();

	//	-----エネミー -----
	EnemyManager::Instance().DrawDebug();
	
	//	----- UI -----
	UIManager::Instance().DrawDebug();

	//	----- Rhythm -----
	JudgeRhythm::Instance().DrawDebug();

}