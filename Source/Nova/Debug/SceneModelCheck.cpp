#include "SceneModelCheck.h"

#include "../Core/Framework.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Camera.h"
#include "../Graphics/ShadowMap.h"
#include "../Scenes/SceneManager.h"
#include "../Scenes/SceneTitle.h"
#include "../Scenes/SceneLoading.h"
#include "../Input/GamePad.h"

//	初期化
void SceneModelCheck::Initialize()
{
	//	スプライト初期化
	sprite_[static_cast<int>(SPRITE_MODEL_CHECK::BACK)] = std::make_unique<Sprite>(L"./Resources/Image/Game.png");

	//	モデル初期化
#if 1//Shadow
	//object_[static_cast<int>(MODEL_ModelCheck::PLANTUNE)] = std::make_unique<GameObject>("./Resources/Model/old/plantune.fbx", true);//	プランチューン
	//object_[static_cast<int>(MODEL_ModelCheck::SHOGI_BOARD)] = std::make_unique<GameObject>("./Resources/Model/old/Shogi/shogiboard.fbx", true);//	将棋盤
#endif
	//object_[static_cast<int>(MODEL_ModelCheck::CITY)] = std::make_unique<GameObject>("./Resources/Model/cybercity-2099-v2/city.fbx", true);//	シティモデル
	//object_[static_cast<int>(MODEL_ModelCheck::Drone)] = std::make_unique<GameObject>("./Resources/Model/Drone/Drone.fbx",true);
	//object_[(int)MODEL_CHECK::Dragonkin] = std::make_unique<GameObject>("./Resources/Model/silver-dragonkin-mir4/source/Silver_Dragonkin/Mon_BlackDragon31_Skeleton2.fbx");
	//object_[(int)MODEL_ModelCheck::PLAYER] = std::make_unique<GameObject>("./Resources/Model/free-mixamo-retextured-model/source/model2.fbx");
	//object_[(int)MODEL_CHECK::CUBE] = std::make_unique<GameObject>("./Resources/Model/Cube/source/Cube.000.fbx", true);

	//	TODO:ModelPixelShaderモデルごとにピクセルシェーダー設定 ここで作ったものを描画前にセットする(別で書く)
	//Graphics::Instance().GetShader()->CreatePsFromCso(Graphics::Instance().GetDevice(), "./Resources/Shader/CityPS.cso", modelPixelShader_[(int)MODEL_CHECK::CITY].ReleaseAndGetAddressOf());
	//Graphics::Instance().GetShader()->CreatePsFromCso(Graphics::Instance().GetDevice(), "./Resources/Shader/DronePS.cso", modelPixelShader_[(int)MODEL_CHECK::Drone].ReleaseAndGetAddressOf());

	//	SceneConstant
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = (sizeof(Graphics::SceneConstants));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	Graphics::Instance().GetDevice()->CreateBuffer(&desc, nullptr, sceneConstantBuffer_.GetAddressOf());

	//	カメラ初期設定
	Camera::Instance().Initialize();

}

//	リセット
void SceneModelCheck::Reset()
{
	//	カメラ初期設定
	Camera::Instance().Initialize();

}

//	更新処理
void SceneModelCheck::Update(const float& elapsedTime)
{
	GamePad& gamePad = Input::Instance().GetGamePad();

	//Camera::Instance().SetTargetPos(object_[(int)MODEL_ModelCheck::CITY]->GetTransform()->GetPosition());
	Camera::Instance().Update(elapsedTime);

#if 1//実験用
	if (gamePad.GetButtonDown() & GamePad::BTN_A)//Zキー
		Camera::Instance().LaunchCameraMove(DirectX::XMFLOAT3(0.0f, 25.0f, 3.0f), DirectX::XMFLOAT3(-1.5f, 0.0f, 0.0f), 3.0f);

#endif

}

//	Shadow描画
void SceneModelCheck::ShadowRender()
{
	//object_[(int)MODEL_ModelCheck::CITY]->ShadowRender(1.0f);
	/*if (object_[(int)MODEL_CHECK::PLANTUNE])
	{
		object_[(int)MODEL_CHECK::PLANTUNE]->ShadowRender(1.0f);
	}
	if (object_[(int)MODEL_CHECK::SHOGI_BOARD])
	{
		object_[(int)MODEL_CHECK::SHOGI_BOARD]->ShadowRender(1.0f);
	}*/
	
	//object_[(int)MODEL_ModelCheck::Drone]->ShadowRender(1.0f);

	//for (int i = 0; i < static_cast<int>(MODEL_ModelCheck::Max); i++)
	//{
	//	if (object_[i] != nullptr)
	//	{
	//		object_[i]->ShadowRender(1.0f);
	//	}
	//}

}

//	描画処理
void SceneModelCheck::Render()
{
	Camera::Instance().SetPerspectiveFov();

	Graphics::Instance().SetViewProjection(Camera::Instance().GetViewMatrix() * Camera::Instance().GetProjectionMatrix());
	Graphics::Instance().SetLightDirection({ 0,-1,0,0 });
	Graphics::Instance().SetCameraPosition({ 0,0,1,0 });

	Graphics::SceneConstants sceneConstants = Graphics::Instance().GetSceneConstant();
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(sceneConstantBuffer_.Get(), 0, 0, &sceneConstants, 0, 0);
	Graphics::Instance().GetDeviceContext()->VSSetConstantBuffers(1, 1, sceneConstantBuffer_.GetAddressOf());
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(1, 1, sceneConstantBuffer_.GetAddressOf());

	// Model
	{
		//	ステージ
		//	ステート設定
		//	シティモデル用
		//Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
		//Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		//Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

		//	Drone	ステート設定
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

		//	Shadow描画はこの関数じゃなくてShadowRender()でやってる
		{
			Graphics::Instance().SetLightDirection(ShadowMap::Instance().GetLightDirection());
			DirectX::XMFLOAT4 cameraPosition_ = { Camera::Instance().GetEye().x,Camera::Instance().GetEye().y,Camera::Instance().GetEye().z,1.0f };
			Graphics::Instance().SetCameraPosition(cameraPosition_);

			D3D11_VIEWPORT viewport;
			UINT numViewports{ 1 };
			Graphics::Instance().GetDeviceContext()->RSGetViewports(&numViewports, &viewport);

			float aspectRatio{ viewport.Width / viewport.Height };
#if 1
			DirectX::XMMATRIX Projection{ DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60), aspectRatio, 0.1f, 1000.0f) };
			DirectX::XMVECTOR Eye{ DirectX::XMLoadFloat3(&Camera::Instance().GetEye()) };
			DirectX::XMVECTOR Focus{ DirectX::XMLoadFloat3(&Camera::Instance().GetFocus()) };
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

			ID3D11Buffer* shadowConstantBuffer = ShadowMap::Instance().GetConstantBuffer();
			Graphics::SceneConstants sceneConstant = Graphics::Instance().GetSceneConstant();
			Graphics::Instance().GetDeviceContext()->UpdateSubresource(shadowConstantBuffer, 0, 0, &sceneConstant, 0, 0);
			Graphics::Instance().GetDeviceContext()->VSSetConstantBuffers(1, 1, &shadowConstantBuffer);
			Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(1, 1, &shadowConstantBuffer);

			// SHADOW : bind shadow map at slot 8
			ID3D11ShaderResourceView* srv = ShadowMap::Instance().GetShaderResourceView();
			Graphics::Instance().GetDeviceContext()->PSSetShaderResources(8, 1, &srv);	//	TODO:出力で警告出てる
		}

		//	モデル描画
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);

#if 0
		//	シティモデル
		if (object_[(int)MODEL_CHECK::CITY])
		{
			object_[(int)MODEL_CHECK::CITY]->GetTransform()->SetScaleFactor(0.001f);
			object_[(int)MODEL_CHECK::CITY]->SetPixelShader(modelPixelShader_[(int)MODEL_CHECK::CITY].Get());
			object_[(int)MODEL_CHECK::CITY]->Render();
		}

		//	ドローン
		if (object_[(int)MODEL_CHECK::DRONE])
		{
			object_[(int)MODEL_CHECK::DRONE]->GetTransform()->SetColor({ 1.0f,1.1f,1.3f,1.0f });
			object_[(int)MODEL_CHECK::DRONE]->SetPixelShader(modelPixelShader_[(int)MODEL_CHECK::DRONE].Get());
			object_[(int)MODEL_CHECK::DRONE]->Render();
		}

		//	竜人
		if (object_[(int)MODEL_CHECK::DRAGONKIN])
		{
			object_[(int)MODEL_CHECK::DRAGONKIN]->GetTransform()->SetColor({ 1.0f,1.1f,1.3f,1.0f });
			object_[(int)MODEL_CHECK::DRAGONKIN]->SetPixelShader(modelPixelShader_[(int)MODEL_CHECK::DRONE].Get());
			object_[(int)MODEL_CHECK::DRAGONKIN]->Render();
		}

		//	プレイヤー
		if (object_[(int)MODEL_CHECK::PLAYER])
		{
			object_[(int)MODEL_CHECK::PLAYER]->GetTransform()->SetColor({ 1.0f,1.1f,1.3f,1.0f });
			object_[(int)MODEL_CHECK::PLAYER]->SetPixelShader(modelPixelShader_[(int)MODEL_CHECK::DRONE].Get());
			object_[(int)MODEL_CHECK::PLAYER]->Render();
		}

		//	将棋盤とプランチューン(Shadow)
		if (object_[(int)MODEL_CHECK::PLANTUNE])
		{
			object_[(int)MODEL_CHECK::PLANTUNE]->GetTransform()->SetScaleFactor(0.01f);//	プランチューン
			//object_[(int)MODEL_ModelCheck::CITY]->GetTransform()->SetScaleFactor(0.001f);	//	シティモデル
			object_[(int)MODEL_CHECK::PLANTUNE]->Render();
		}
		if (object_[(int)MODEL_CHECK::SHOGI_BOARD])
		{
			Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::SOLID);
			Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
			Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
			object_[(int)MODEL_CHECK::SHOGI_BOARD]->GetTransform()->SetColor({ 1, 1, 1, 1 });
			object_[(int)MODEL_CHECK::SHOGI_BOARD]->Render();	//	将棋盤
		}

		//	キューブ
		if (object_[(int)MODEL_CHECK::CUBE])
		{
			object_[(int)MODEL_CHECK::CUBE]->Render();
		}
#endif


	}

	//	Sprite 手前に描画するもの
	{
		//	手前にスプライト出すならZON_ON、奥に描画ならOFF_OFF
		Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
		//Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_OFF_ZW_OFF);
		Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
		Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);//	各ステート毎のスプライト描画

		sprite_[static_cast<int>(SPRITE_MODEL_CHECK::BACK)]->GetTransform()->SetSize(SCREEN_WIDTH, SCREEN_HEIGHT);
		//sprite_[static_cast<int>(SPRITE_ModelCheck::BACK)]->Render();	//	ゲームスプライト描画

	}

	//	TODO:デバッグプリミティブ
	{
		// デバッグレンダラ描画実行
		//Graphics::Instance().GetDebugRenderer()->Render();
	}

}

//	終了化
void SceneModelCheck::Finalize()
{
	//	スプライト終了化
	for (int i = 0; i < static_cast<int>(SPRITE_MODEL_CHECK::Max); i++)
	{
		if (sprite_[i] != nullptr)
		{
			sprite_[i] = nullptr;
		}
	}
	//	ステージ終了化
	for (int i = 0; i < static_cast<int>(MODEL_CHECK::Max); i++)
	{
		if (object_[i] != nullptr)
		{
			object_[i] = nullptr;
		}
	}

}

//	デバッグ描画
void SceneModelCheck::DrawDebug()
{
	D3D11_VIEWPORT viewport;
	UINT numViewports{ 1 };
	Graphics::Instance().GetDeviceContext()->RSGetViewports(&numViewports, &viewport);

	auto srv = ShadowMap::Instance().GetShaderResourceView();
	ImGui::Image(reinterpret_cast<void*>(srv), ImVec2(viewport.Width / 5.0f, viewport.Height / 5.0f));

	if (ImGui::TreeNode(u8"Cameraカメラ"))	//	カメラ
	{
		Camera::Instance().DrawDebug();
		ImGui::TreePop();
	}

	if (object_[(int)MODEL_CHECK::CUBE])	//	キューブ
	{
		if (ImGui::TreeNode(u8"Cube"))
		{
			object_[(int)MODEL_CHECK::CUBE]->GetTransform()->DrawDebug();
			ImGui::DragFloat3("Scale", &modelScale_, 0.1f, -FLT_MAX, FLT_MAX);
			ImGui::TreePop();
		}
	}

	ShadowMap::Instance().DrawDebug();	//	SHADOW

}
