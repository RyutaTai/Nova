#include "SceneGame.h"

#include "../../Nova/Core/Framework.h"
#include "../../Nova/Graphics/Graphics.h"
#include "../../Nova/Graphics/Camera.h"
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
#include "../../Game/JudgeRhythm.h"
#include "../../Nova/Collision/CollisionManager.h"

//	初期化
void SceneGame::Initialize()
{
	/* ----- オーディオ初期化 ----- */
#if 1
	//AudioSource* gameBGM = AudioManager::Instance().LoadAudioSource("./Resources/Audio/BGM/Game.wav", Audio::AudioType::BGMNormal, "GameScene");
	AudioSource* gameBGM = AudioManager::Instance().LoadAudioSource("./Resources/Audio/BGM/452_BPM140_2.wav", Audio::AudioType::BGMNormal, "GameScene");
#else
	//AudioSource* gameBGM = AudioManager::Instance().LoadAudioSource("./Resources/Audio/BGM/fourOnTheFloor_Basic_120BPM_44100Hz_16bit.wav", Audio::AudioType::BGMNormal, "GameScene");
	AudioSource* gameBGM = AudioManager::Instance().LoadAudioSource("./Resources/Audio/BGM/fourOnTheFloor_Basic_140BPM_44100Hz_16bit.wav", Audio::AudioType::BGMNormal, "GameScene");
#endif
	gameBGM->SetVolume(0.3f, false);
	gameBGM->SetAudioName("GameBGM");
	AudioManager::Instance().Register(gameBGM);


	/* ----- スプライト初期化 ----- */
	//sprite_[static_cast<int>(SPRITE_GAME::BACK)] = std::make_unique<Sprite>(Graphics::Instance().GetDevice(), L"./Resources/Image/Game.png");

	sprites_[static_cast<int>(SPRITE_GAME::Clear)]	 = std::make_unique<Sprite>(L"./Resources/Image/Clear.png");
	sprites_[static_cast<int>(SPRITE_GAME::GameOver)] = std::make_unique<Sprite>(L"./Resources/Image/GameOver.png");

	/* ----- UI初期化(生成したらUIクラスでマネージャーに登録される) ----- */
	std::unique_ptr<UIHealth> uiHealth = std::make_unique<UIHealth>();
	UIManager::Instance().Register(std::move(uiHealth));
	//UIInstructions* uiInstructions	= new UIInstructions();
	
	std::unique_ptr<UITempo> uiTempo = std::make_unique<UITempo>();
	UIManager::Instance().RegisterUITempo(uiTempo.get());
	UIManager::Instance().Register(std::move(uiTempo));
	
	std::unique_ptr<UIRank> uiRank = std::make_unique<UIRank>();
	UIManager::Instance().RegisterUIRank(uiRank.get());
	UIManager::Instance().Register(std::move(uiRank));
	UIManager::Instance().Initialize();					//	登録し終わってから初期化処理をする

	/* ----- Rhythmクラス初期化 ----- */
	JudgeRhythm::Instance().Initialize();

	/* ----- ステージ初期化 ----- */
	stage_ = std::make_unique<Stage>();					//	シティモデル

	/* ----- シーン定数バッファ ----- */
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = (sizeof(Graphics::SceneConstants));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	Graphics::Instance().GetDevice()->CreateBuffer(&desc, nullptr, sceneConstantBuffer_.GetAddressOf());

	/* ----- カメラ初期化 ----- */
	Camera::Instance().Initialize();

	/* ----- プレイヤー初期化 ----- */
	player_ = std::make_unique<Player>();
	//player_ = std::make_unique<Player>("./Resources/Model/free-mixamo-retextured-model/source/model4.fbx", false, 60.0f);
	player_->Initialize();

	/* ----- エネミー初期化 ----- */
	dragonkin_ = std::make_unique<Dragonkin>();
	dragonkin_->Initialize();
	EnemyManager::Instance().Register(std::move(dragonkin_));

	// ----- テクスチャ読み込み -----
	D3D11_TEXTURE2D_DESC texture2dDesc = {};
	ID3D11Device* device = Graphics::Instance().GetDevice();

#if 1
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/sunset_jhbcentral_4k/sunset_jhbcentral_4k.dds",
		shaderResourceViews_[0].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/sunset_jhbcentral_4k/diffuse_iem.dds",
		shaderResourceViews_[1].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/sunset_jhbcentral_4k/specular_pmrem.dds",
		shaderResourceViews_[2].GetAddressOf(), &texture2dDesc);
	/*LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/sunset_jhbcentral_4k/sheen_pmrem.dds",
		shaderResourceViews_[3].GetAddressOf(), &texture2dDesc);*/
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/lut_charlie.dds",
		shaderResourceViews_[3].GetAddressOf(), &texture2dDesc);
#endif

#if 0
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/tears_of_steel_bridge_4k/tears_of_steel_bridge_4k.dds",
		shaderResourceViews_[0].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/tears_of_steel_bridge_4k/diffuse_iem.dds",
		shaderResourceViews_[1].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/tears_of_steel_bridge_4k/specular_pmrem.dds",
		shaderResourceViews_[2].GetAddressOf(), &texture2dDesc);
	LoadTextureFromFile(device, L"./Resources/Model/GltfSample/environments/tears_of_steel_bridge_4k/sheen_pmrem.dds",
		shaderResourceViews_[3].GetAddressOf(), &texture2dDesc);
#endif
	//	----- ブルーム -----
	framebuffers_[0] = std::make_unique<FrameBuffer>(device, SCREEN_WIDTH, SCREEN_HEIGHT);
	framebuffers_[1] = std::make_unique<FrameBuffer>(device, SCREEN_WIDTH, SCREEN_HEIGHT);	//	sprite
	fullScreenQuad_ = std::make_unique<FullScreenQuad>(device);
	bloomer_ = std::make_unique<Bloom>(device, SCREEN_WIDTH, SCREEN_HEIGHT);
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/FinalPassPs.cso", pixelShaders_[0].ReleaseAndGetAddressOf());

	//	----- シャドウ -----
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/CascadedShadowPs.cso", pixelShaders_[2].GetAddressOf());
	cascadedShadowMaps_ = std::make_unique<decltype(cascadedShadowMaps_)::element_type>(device, 1024 * 4, 1024 * 4);

	//	----- カラーフィルター -----
	colorFilter_ = std::make_unique<ColorFilter>();

	//	----- ステート登録 -----
	stateMachine_.reset(new StateMachine<State<SceneGame>>());
	stateMachine_->RegisterState(new GameState::Wave1State(this));		//	Wave1
	stateMachine_->RegisterState(new GameState::Wave2State(this));		//	Wave2
	stateMachine_->RegisterState(new GameState::Wave3State(this));		//	Wave3
	stateMachine_->RegisterState(new GameState::GameClearState(this));	//	ゲームクリア
	stateMachine_->RegisterState(new GameState::GameOverState(this));	//	ゲームオーバー
	stateMachine_->RegisterState(new GameState::ContinueState(this));	//	コンティニュー
	//	初期ステート設定
	stateMachine_->SetState(static_cast<int>(SceneGameState::Wave1));	//	初期ステートセット

}

//	リセット
void SceneGame::Reset()
{
	/* ----- カメラ初期化 ----- */
	Camera::Instance().Initialize();

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
	//drone_->Update(elapsedTime);

	// ----- エフェクト更新処理 -----
	EffectManager::Instance().Update(elapsedTime);

	// ----- UI更新処理 -----
	UIManager::Instance().Update(elapsedTime);

	// ----- Rhythm更新処理 -----
	JudgeRhythm::Instance().Update();

	// ----- Collision更新処理 -----
	CollisionManager::Instance().Update(elapsedTime);

	//	----- カラーフィルター更新処理 -----
	colorFilter_->Update();

	//	ゲームクリアへの遷移はWeve3 State内で行っている	
	//	ゲームオーバー
	float playerHp = player_->GetHp();
	if (playerHp <= 0.0f)
	{
		ChangeState(SceneGameState::GameOver);
	}

	//	タイトルへ遷移
	if (changeTitle_)
	{
		SceneManager::Instance().ChangeScene(new SceneTitle);
	}
	
}

//	ポーズにする
void SceneGame::IsPose(const bool& isPose)
{
	player_->SetIsPose(isPose);
}

//	ウェーブ画像読み込み
void SceneGame::LoadWaveSprite(const wchar_t* filename)
{
	sprites_[SPRITE_GAME::WAVE] = std::make_unique<Sprite>(filename);
}

//	描画処理
void SceneGame::Render()
{
	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
	ID3D11ShaderResourceView* nullShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	deviceContext->VSSetShaderResources(0, _countof(nullShaderResourceViews), nullShaderResourceViews);
	deviceContext->PSSetShaderResources(0, _countof(nullShaderResourceViews), nullShaderResourceViews);
	
	Camera::Instance().SetPerspectiveFov();

	//	シーン定数バッファ更新
	Graphics::Instance().SetViewProjection(Camera::Instance().CalcViewProjectionMatrix());
	Graphics::Instance().SetLightDirection(lightDirection_);
	Graphics::Instance().SetCameraPosition({ 0,0,1,0 });
	Graphics::Instance().SetInvViewProjection(Camera::Instance().CalcInvViewProjectionMatrix());
	Graphics::Instance().SetInvProjection(Camera::Instance().CalcInvProjectionMatrix());

	Graphics::SceneConstants sceneConstants = Graphics::Instance().GetSceneConstant();
	deviceContext->UpdateSubresource(sceneConstantBuffer_.Get(), 0, 0, &sceneConstants, 0, 0);
	deviceContext->VSSetConstantBuffers(1, 1, sceneConstantBuffer_.GetAddressOf());
	deviceContext->PSSetConstantBuffers(1, 1, sceneConstantBuffer_.GetAddressOf());

	/* ----- モデル描画 ----- */
	{
#if 1
		//	ステート設定
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

		{
			DirectX::XMFLOAT4 cameraPosition = { Camera::Instance().GetEye().x,Camera::Instance().GetEye().y,Camera::Instance().GetEye().z,1.0f };
			Graphics::Instance().SetCameraPosition(cameraPosition);

			D3D11_VIEWPORT viewport;
			UINT numViewports{ 1 };
			deviceContext->RSGetViewports(&numViewports, &viewport);

#if 1
			Camera::Instance().SetPerspectiveFov();
			DirectX::XMMATRIX Projection = Camera::Instance().GetProjectionMatrix();
			
			DirectX::XMVECTOR Eye{ DirectX::XMLoadFloat3(&Camera::Instance().GetEye()) };
			DirectX::XMVECTOR Focus{DirectX::XMLoadFloat3(&Camera::Instance().GetFocus()) };
			DirectX::XMVECTOR Up{ DirectX::XMLoadFloat3(&Camera::Instance().GetUp()) };
			DirectX::XMMATRIX V{ DirectX::XMMatrixLookAtLH(Eye, Focus, Up) };
#else
			DirectX::XMMATRIX P{ DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(30), aspectRatio, 0.1f, 100.0f) };
			DirectX::XMVECTOR eye{ DirectX::XMLoadFloat4(&ShadowMap::Instance().GetCameraPosition()) };
			DirectX::XMVECTOR focus{ DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) };
			DirectX::XMVECTOR up{ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) };
			DirectX::XMMATRIX V{ DirectX::XMMatrixLookAtLH(eye, focus, up) };
#endif
			Graphics::Instance().SetViewProjection(V * Projection);

		}
#endif

		/* ----- モデル描画 ----- */
		framebuffers_[0]->Clear(deviceContext);
		framebuffers_[0]->Activate(deviceContext);

		deviceContext->PSSetShaderResources(32, 1, shaderResourceViews_[0].GetAddressOf());
		deviceContext->PSSetShaderResources(33, 1, shaderResourceViews_[1].GetAddressOf());
		deviceContext->PSSetShaderResources(34, 1, shaderResourceViews_[2].GetAddressOf());
		deviceContext->PSSetShaderResources(35, 1, shaderResourceViews_[3].GetAddressOf());

		/* ----- ステージ ----- */
		//	ステート設定
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
		stage_->Render();

		/* ----- プレイヤー ----- */
		//	ステート設定
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::SOLID);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
		player_->Render();

		/* ----- エネミー ----- */
		//	ステート設定
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
		EnemyManager::Instance().Render();

		framebuffers_[0]->Deactivate(deviceContext);

		//	ブルーム
		if (bloomer_)
		{
			bloomer_->Make(deviceContext, framebuffers_[0]->shaderResourceViews_[0].Get());
		}

		//	シャドウマップ
		MakeShadow();
		framebuffers_[1]->Clear(deviceContext);
		framebuffers_[1]->Activate(deviceContext); 
		DrawShadow();
		framebuffers_[1]->Deactivate(deviceContext);
#if 0

		//	ヴィネット
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);//	各ステート毎のスプライト描画
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
		//Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

		framebuffers_[0]->Clear(deviceContext);
		framebuffers_[0]->Activate(deviceContext);
		vignette_->Make();
		fullScreenQuad_->Blit(deviceContext, framebuffers_[1]->shaderResourceViews_[0].GetAddressOf(), 0, 1, vignette_->GetVignettePixelShader());
		framebuffers_[0]->Deactivate(deviceContext);
#endif		

		Vignette::Instance().Make();

		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
		ID3D11ShaderResourceView* shaderResourceViews[] =
		{
			framebuffers_[1]->shaderResourceViews_[0].Get(),	//	colorMap
			bloomer_->ShaderResourceView(),						//	boom
			framebuffers_[1]->shaderResourceViews_[1].Get(),	//	depthMap
			cascadedShadowMaps_->DepthMap().Get()				//	cascadedShadowMap

		};
		fullScreenQuad_->Blit(deviceContext, shaderResourceViews, 0, _countof(shaderResourceViews), pixelShaders_[0].Get());

	}

	/* ----- エフェクト描画 ----- */
	{
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::SOLID);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

		DirectX::XMFLOAT4X4 view;
		DirectX::XMStoreFloat4x4(&view, Camera::Instance().GetViewMatrix());
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMStoreFloat4x4(&projection, Camera::Instance().GetProjectionMatrix());
		EffectManager::Instance().Render(view, projection);
	}

	/* ----- デバッグプリミティブ描画 ----- */
	{
		//	デバッグレンダラ描画実行
#if _DEBUG
		player_->DrawDebugPrimitive();
		EnemyManager::Instance().DrawDebugPrimitive();
		Graphics::Instance().GetDebugRenderer()->Render();
#endif
	}

	/* ----- スプライト描画 ----- */
	{
		//	手前にスプライト出すならZON_ON、奥に描画ならOFF_OFF
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);//	各ステート毎のスプライト描画
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		//Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

		//	ウェーブ数描画
		if (sprites_[SPRITE_GAME::WAVE] && waveStartTimer_ > 0)
		{
			//sprite_[static_cast<int>(SPRITE_GAME::WAVE)]->Render();
		}

		//	操作方法描画
		if (waveStartTimer_ <= 0.0f && isResult_ == false)
		{
			//ui_[static_cast<int>(UI_GAME::Instructions)]->SetRenderFlag(true);
		}

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

	/* ----- UI描画 ----- */
	UIManager::Instance().Render();

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
	cascadedShadowMaps_->Activate(deviceContext, cameraView, cameraProjection, lightDirection_, criticalDepthValue_, 3/*cb_slot*/);
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
		framebuffers_[0]->shaderResourceViews_[0].Get(),	// colorMap
		framebuffers_[0]->shaderResourceViews_[1].Get(),	// DepthMap
		cascadedShadowMaps_->DepthMap().Get()				// cascadedShadowMaps
	};
	fullScreenQuad_->Blit(deviceContext, shaderResourceViews, 0, _countof(shaderResourceViews), pixelShaders_[2].Get());

}

//	終了化
void SceneGame::Finalize()
{
	//	エネミーマネージャー終了化
	EnemyManager::Instance().Clear();

	//	UIマネージャー終了化
	UIManager::Instance().Finalize();

	//	オーディオ終了化
	AudioManager::Instance().RemoveBySceneName("GameScene");

}

//	デバッグ描画
void SceneGame::DrawDebug()
{
	D3D11_VIEWPORT viewport;
	UINT numViewports{ 1 };
	Graphics::Instance().GetDeviceContext()->RSGetViewports(&numViewports, &viewport);

	//	----- DebugRenderer -----
	Graphics::Instance().GetDebugRenderer()->DrawDebugGUI();

	//	SceneConstant
	if (ImGui::TreeNode("SceneConstant"))
	{
		ImGui::DragFloat4("LightDirection", &lightDirection_.x, 0.01f, -1.0f, 1.0f);	//	ライトの向き
		ImGui::TreePop();
	}

	//	----- ブルーム -----
	if (bloomer_)bloomer_->DrawDebug();

	//	----- シャドウ -----
	if (ImGui::TreeNode("Shadow"))
	{
		ImGui::DragFloat("CriticalDepthValue", &criticalDepthValue_, 0.1f);
		cascadedShadowMaps_->DrawDebug();
		ImGui::TreePop();
	}

	//	----- ヴィネット -----
	Vignette::Instance().DrawDebug();

	//	----- カラーフィルター -----
	colorFilter_->DrawDebug();

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