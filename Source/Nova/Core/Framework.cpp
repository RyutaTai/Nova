#include "Framework.h"

#include <dxgi.h>
#include <dxgidebug.h>

#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "../../Game/Scenes/SceneManager.h"
#include "../../Game/Scenes/SceneTitle.h"
#include "../../Game/Scenes/SceneGame.h"
#include "../Resources/EffectManager.h"
#include "../Audio/AudioManager.h"

HighResolutionTimer Framework::tictoc_ = {};

Framework::Framework(HWND hwnd)
	: graphics_(hwnd, FULLSCREEN/*フルスクリーンのオン・オフ*/),
	input_(hwnd)
{

}

//	初期化
bool Framework::Initialize()
{
	//	シーン初期化
	SceneManager::Instance().ChangeScene(new SceneTitle());

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

	IMGUI_CTRL_INITIALIZE(graphics_.GetWindowHandle(), graphics_.GetDevice(), graphics_.GetDeviceContext());

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
		SetWindowTextW(graphics_.GetWindowHandle(), outs.str().c_str());

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
#ifdef _DEBUG
	//	D3D11Debug オブジェクトを取得
	Microsoft::WRL::ComPtr<ID3D11Debug> d3dDebug;
	HRESULT hr = Graphics::Instance().GetDevice()->QueryInterface(__uuidof(ID3D11Debug), &d3dDebug);

	if (SUCCEEDED(hr))
	{
		// DebugRenderer のように、明示的に Reset() が必要な ComPtr ではないリソースがあれば、ここで解放を試みる。
		// ただし、通常 ComPtr で管理されていれば、明示的な解放は不要。
		// ここでは、未解放のオブジェクトをレポートする目的。

		// LiveObjects をレポートする前に、デバイスコンテキストの状態をクリーンアップ
		// これがないと、ReportLiveObjects が誤った参照カウントを報告することがある。
		Graphics::Instance().GetDeviceContext()->ClearState();
		Graphics::Instance().GetDeviceContext()->Flush();

		// ReportLiveObjects を呼び出し
		d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_SUMMARY);
	}

	// DXGIDebug オブジェクトを取得（必要であれば）
	Microsoft::WRL::ComPtr<IDXGIDebug> dxgiDebug;
	hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)); // DXGIGetDebugInterface は dxgi.h にある
	if (SUCCEEDED(hr))
	{
		dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	}
#endif
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