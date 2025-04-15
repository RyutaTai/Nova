#include "Framework.h"

#include <dxgi.h>

#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "../Graphics/ShadowMap.h"
#include "../Scenes/SceneManager.h"
#include "../Scenes/SceneTitle.h"
#include "../Scenes/SceneGame.h"
#include "../Scenes/SceneDemo.h"
#include "../Debug/SceneModelCheck.h"
#include "../Resources/EffectManager.h"
#include "../Audio/AudioManager.h"

HighResolutionTimer Framework::tictoc_ = {};

//	コンストラクタ
Framework::Framework(HWND hwnd)
	: graphics_(hwnd, FULLSCREEN/*fullscreen*/),
	input_(hwnd)
{
	//	XAUDIO2デバッグ用
	{
		HRESULT hr = S_OK;
		IXAudio2* xaudio = nullptr;

		hr = XAudio2Create(&xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// create masteringvoice
		IXAudio2MasteringVoice* masteringVoice = nullptr;
#if 0
		hr = xaudio->CreateMasteringVoice(&masteringVoice, 2, 44100/*サンプリングレート*/, 0U, NULL, 0, AudioCategory_GameEffects);
#else
		hr = xaudio->CreateMasteringVoice(&masteringVoice, XAUDIO2_DEFAULT_CHANNELS, 44100/*サンプリングレート*/, 0U, NULL, 0, AudioCategory_GameEffects);
#endif
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		XAUDIO2_VOICE_DETAILS masterDetails;
		masteringVoice->GetVoiceDetails(&masterDetails);

		//	マスタリングボイス情報(デバッグ用)
		int masterInputChannel = masterDetails.InputChannels;
		DWORD ChannelMask = {};
		masteringVoice->GetChannelMask(&ChannelMask);
		int masterSampleRate = masterDetails.InputSampleRate;
	}

}

//	デストラクタ
Framework::~Framework()
{
	////	シーン終了化
	//SceneManager::Instance().Clear();

	////	エフェクトマネージャー終了化
	//EffectManager::Instance().Finalize();
}

//	初期化
bool Framework::Initialize()
{
	// TODO:シーン初期化切り替え
	SceneManager::Instance().ChangeScene(new SceneTitle());
	//SceneManager::Instance().ChangeScene(new SceneGame());
	//SceneManager::Instance().ChangeScene(new SceneDemo());
	//SceneManager::Instance().ChangeScene(new SceneModelCheck());

	//	エフェクトマネージャー初期化
	EffectManager::Instance().Initialize();

	//	オーディオマネージャー初期化
	AudioManager::Instance().Initialize();

	return true;
}

//	更新処理
void Framework::Update(const float& elapsedTime)
{
	IMGUI_CTRL_CLEAR_FRAME();

	//	Input更新処理
	input_.Update(elapsedTime);

	//	オーディオ更新処理
	AudioManager::Instance().Update(elapsedTime);

	//	シーンの更新
	SceneManager::Instance().Update(elapsedTime);

}

//	アプリケーションループ
int Framework::Run()
{
	MSG msg{};

	if (!Initialize())
	{
		return 0;
	}

	IMGUI_CTRL_INITIALIZE(graphics_.GetHwnd(), graphics_.GetDevice(), graphics_.GetDeviceContext());

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			tictoc_.Tick();
			CalculateFrameStats();
			Update(tictoc_.GetDeltaTime());
#if _DEBUG
			DrawDebug();	//	Update()とRender()の間で呼ぶ。(ImGui::NewFrame();とImGui::Render();で挟む必要があるが、これをUpdateとRenderで呼んでいるため。)
#endif
			Render();
		}
	}

	IMGUI_CTRL_UNINITIALIZE();

#if 0
	BOOL fullscreen = 0;
	graphics_.GetSwapChain()->GetFullscreenState(&fullscreen, 0);
	if (fullscreen)
	{
		graphics_.GetSwapChain()->SetFullscreenState(FALSE, 0);
	}
#endif

	return Uninitialize() ? static_cast<int>(msg.wParam) : 0;
}

//	フレームレート計算
void Framework::CalculateFrameStats()
{
	if (++framesPerSecond_, (tictoc_.TimeStamp() - elapsedTime_) >= 1.0f)
	{
		fps_ = static_cast<float>(framesPerSecond_);
		std::wostringstream outs;
		outs.precision(6);
		outs << APPLICATION_NAME << L" : FPS : " << fps_ << L" / " << L"Frame Time : " << 1000.0f / fps_ << L" (ms)";
		SetWindowTextW(graphics_.GetHwnd(), outs.str().c_str());

		//	FPS値をバッファに追加
		if (fpsBuffer_.size() >= maxHistorySize_)
		{
			fpsBuffer_.erase(fpsBuffer_.begin());
		}
		fpsBuffer_.push_back(fps_);

		framesPerSecond_ = 0;
		elapsedTime_ += 1.0f;
	}
}

//	メッセージハンドラ
LRESULT CALLBACK Framework::HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef USE_IMGUI
	IMGUI_CTRL_WND_PRC_HANDLER(hwnd, msg, wparam, lparam);
#endif
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps{};
		BeginPaint(hwnd, &ps);

		EndPaint(hwnd, &ps);
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)	//	Escキーで落とす
		{
			PostMessage(hwnd, WM_CLOSE, 0, 0);
		}
		break;
	case WM_ENTERSIZEMOVE:
		tictoc_.Stop();
		break;
	case WM_EXITSIZEMOVE:
		tictoc_.Start();
		break;
	case WM_SIZE:
	{
#if 1
		RECT clientRect{};
		GetClientRect(hwnd, &clientRect);
		graphics_.OnSizeChanged(static_cast<UINT64>(clientRect.right - clientRect.left), clientRect.bottom - clientRect.top);
#endif
		break;
	}
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

//	描画処理
void Framework::Render()
{
	//	別スレッド中にデバイスコンテキストが使われていた場合に
	//	同時アクセスしないように排他制御する
	std::lock_guard<std::mutex>lock(graphics_.GetMutex());

	//	サンプラーステート設定
	graphics_.GetShader()->SetSamplerState(graphics_.GetDeviceContext());

	//	ShadowMap生成
	//	Activate
	ShadowMap::Instance().Activate();
	//	ShadowRender
	SceneManager::Instance().ShadowRender();
	//	Deactivate
	ShadowMap::Instance().Deactivate();

	//	Scene描画
	FLOAT color[]{ 1, 0, 0, 1 };
	ID3D11RenderTargetView* renderTargetView = graphics_.GetRenderTargetView();
	graphics_.GetDeviceContext()->ClearRenderTargetView(graphics_.GetRenderTargetView(), color);
	graphics_.GetDeviceContext()->ClearDepthStencilView(graphics_.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	graphics_.GetDeviceContext()->OMSetRenderTargets(1, &renderTargetView, graphics_.GetDepthStencilView());

	SceneManager::Instance().Render();

	//	ImGui描画
	IMGUI_CTRL_DISPLAY();

	// 実行
	graphics_.PresentFrame();

	//	※これ以下は描画されない
}

//	終了化
bool Framework::Uninitialize()
{
	//	シーン終了化
	SceneManager::Instance().Clear();

	//	エフェクトマネージャー終了化
	EffectManager::Instance().Finalize();


	return true;
}

//	デバッグ描画
void Framework::DrawDebug()
{
	//	デバッグウィンドウを作成
	ImGui::Begin("Debug");

	ImGui::Text("Framework");
	//	fpsのグラフを描画
	ImGui::PlotLines("FPS Graph", fpsBuffer_.data(), static_cast<int>(fpsBuffer_.size()), 0, nullptr, 0.0f, FLT_MAX, ImVec2(0, 80));
	// 現在のFPSを数値として表示
	ImGui::Text("Current FPS: %.2f", fps_);
	
	AudioManager::Instance().DrawDebug();
	graphics_.DrawDebug();
	input_.DrawDebug();


	SceneManager::Instance().DrawDebug();	//	シーンごとのDrawDebug()
	ImGui::End();

}