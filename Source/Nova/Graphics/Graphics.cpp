#include "Graphics.h"

#include "../Core/Framework.h"
#include "../Graphics/Camera.h"

Graphics* Graphics::instance_ = nullptr;

//	コンストラクタ
Graphics::Graphics(HWND hwnd, bool fullscreen)
	: hwnd_(hwnd)
{
	_ASSERT_EXPR(instance_ == instance_, L"already instance");
	instance_ = this;

	if (fullscreen)
	{
		StylizeWindow(true);
	}

	RECT clientRect;
	GetClientRect(hwnd_, &clientRect);
	frameBufferDimensions_.cx = clientRect.right - clientRect.left;
	frameBufferDimensions_.cy = clientRect.bottom - clientRect.top;

	HRESULT hr = S_OK;

	UINT createFactoryFlags{};
#ifdef _DEBUG
	createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory6;
	hr = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(dxgiFactory6.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	AcquireHighPerformanceAdapter(dxgiFactory6.Get(), adapter_.GetAddressOf());

	UINT createDeviceFlags{ 0 };
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
#ifdef ENABLE_DIRECT2D
	createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif

	D3D_FEATURE_LEVEL featureLevels{ D3D_FEATURE_LEVEL_11_1 };
	hr = D3D11CreateDevice(adapter_.Get(), D3D_DRIVER_TYPE_UNKNOWN, 0, createDeviceFlags, &featureLevels, 1, D3D11_SDK_VERSION, device_.GetAddressOf(), NULL, deviceContext_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	CreateSwapChain(dxgiFactory6.Get());
#ifdef ENABLE_DIRECT2D
	CreateDirect2dObjects();
#endif

	//	シーン定数バッファ
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(SceneConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device_->CreateBuffer(&bufferDesc, nullptr, constantBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	
	//	FrameBuffer
	frameBuffers_[0] = std::make_unique<FrameBuffer>(device_.Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
	frameBuffers_[1] = std::make_unique<FrameBuffer>(device_.Get(), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	bitBlockTransfer_ = std::make_unique<FullScreenQuad>(device_.Get());

	shader_ = std::make_unique<Shader>();

	//	デバッグプリミティブ
	debugRenderer_ = std::make_unique<DebugRenderer>(device_.Get());

}

void Graphics::AcquireHighPerformanceAdapter(IDXGIFactory6* dxgiFactory6, IDXGIAdapter3** dxgiAdapter3)
{
	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr<IDXGIAdapter3> enumeratedAdapter;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(enumeratedAdapter.ReleaseAndGetAddressOf())); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		hr = enumeratedAdapter->GetDesc1(&adapterDesc);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		if (adapterDesc.VendorId == 0x1002/*AMD*/ || adapterDesc.VendorId == 0x10DE/*NVIDIA*/)
		{
			OutputDebugStringW((std::wstring(adapterDesc.Description) + L" has been selected.\n").c_str());
			OutputDebugStringA(std::string("\tVendorId:" + std::to_string(adapterDesc.VendorId) + '\n').c_str());
			OutputDebugStringA(std::string("\tDeviceId:" + std::to_string(adapterDesc.DeviceId) + '\n').c_str());
			OutputDebugStringA(std::string("\tSubSysId:" + std::to_string(adapterDesc.SubSysId) + '\n').c_str());
			OutputDebugStringA(std::string("\tRevision:" + std::to_string(adapterDesc.Revision) + '\n').c_str());
			OutputDebugStringA(std::string("\tDedicatedVideoMemory:" + std::to_string(adapterDesc.DedicatedVideoMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tDedicatedSystemMemory:" + std::to_string(adapterDesc.DedicatedSystemMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tSharedSystemMemory:" + std::to_string(adapterDesc.SharedSystemMemory) + '\n').c_str());
			OutputDebugStringA(std::string("\tAdapterLuid.HighPart:" + std::to_string(adapterDesc.AdapterLuid.HighPart) + '\n').c_str());
			OutputDebugStringA(std::string("\tAdapterLuid.LowPart:" + std::to_string(adapterDesc.AdapterLuid.LowPart) + '\n').c_str());
			OutputDebugStringA(std::string("\tFlags:" + std::to_string(adapterDesc.Flags) + '\n').c_str());
			break;
		}
	}
	*dxgiAdapter3 = enumeratedAdapter.Detach();
}

#ifdef ENABLE_DIRECT2D
void Graphics::CreateDirect2dObjects()
{
	HRESULT hr{ S_OK };

	Microsoft::WRL::ComPtr<IDXGIDevice2> dxgiDevice2;

	hr = device_->QueryInterface(__uuidof(IDXGIDevice2), reinterpret_cast<void**>(dxgiDevice2.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	Microsoft::WRL::ComPtr<ID2D1Factory1> d2dFactory1;
	D2D1_FACTORY_OPTIONS factoryOptions{};
#ifdef _DEBUG
	factoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factoryOptions, d2dFactory1.GetAddressOf());

	Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice;
	hr = d2dFactory1->CreateDevice(dxgiDevice2.Get(), d2dDevice.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	hr = d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2d1DeviceContext_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	hr = dxgiDevice2->SetMaximumFrameLatency(1);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	Microsoft::WRL::ComPtr<IDXGISurface2> dxgiSurface2;
	hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(dxgiSurface2.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dBitmap1;
	hr = d2d1DeviceContext_->CreateBitmapFromDxgiSurface(dxgiSurface2.Get(),
		D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)), d2dBitmap1.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	d2d1DeviceContext_->SetTargetPos(d2dBitmap1.Get());

	Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory;
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(dwriteFactory.GetAddressOf()));
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	hr = dwriteFactory->CreateTextFormat(L"Meiryo", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 11, L"", dwriteTextFormats_[0].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	hr = dwriteTextFormats_[0]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	hr = dwriteFactory->CreateTextFormat(L"Impact", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24, L"", dwriteTextFormats_[1].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	hr = dwriteTextFormats_[1]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	hr = d2d1DeviceContext_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), d2dSolidColorBrushes_[0].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	hr = d2d1DeviceContext_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue), d2dSolidColorBrushes_[1].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}
#endif

void Graphics::CreateSwapChain(IDXGIFactory6* dxgiFactory6)
{
	HRESULT hr = S_OK;

	if (swapChain_)
	{
		ID3D11RenderTargetView* nullRenderTargetView{};
		deviceContext_->OMSetRenderTargets(1, &nullRenderTargetView, NULL);
		renderTargetView_.Reset();
#if 1
		deviceContext_->Flush();
		deviceContext_->ClearState();
#endif
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChain_->GetDesc(&swapChainDesc);
		hr = swapChain_->ResizeBuffers(swapChainDesc.BufferCount, frameBufferDimensions_.cx, frameBufferDimensions_.cy, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetBuffer;
		hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(renderTargetBuffer.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		D3D11_TEXTURE2D_DESC texture2d_desc;
		renderTargetBuffer->GetDesc(&texture2d_desc);

		hr = device_->CreateRenderTargetView(renderTargetBuffer.Get(), NULL, renderTargetView_.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	else
	{
		BOOL allowTearing = FALSE;
		if (SUCCEEDED(hr))
		{
			hr = dxgiFactory6->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		}
		tearingSupported_ = SUCCEEDED(hr) && allowTearing;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc1{};
		swapChainDesc1.Width = frameBufferDimensions_.cx;
		swapChainDesc1.Height = frameBufferDimensions_.cy;
		swapChainDesc1.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc1.Stereo = FALSE;
		swapChainDesc1.SampleDesc.Count = 1;
		swapChainDesc1.SampleDesc.Quality = 0;
		swapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc1.BufferCount = 2;
		swapChainDesc1.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc1.Flags = tearingSupported_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		hr = dxgiFactory6->CreateSwapChainForHwnd(device_.Get(), hwnd_, &swapChainDesc1, NULL, NULL, swapChain_.ReleaseAndGetAddressOf());
#if 1
		swapChainDesc1.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
#endif
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		hr = dxgiFactory6->MakeWindowAssociation(hwnd_, DXGI_MWA_NO_ALT_ENTER);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetBuffer;
		hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(renderTargetBuffer.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		hr = device_->CreateRenderTargetView(renderTargetBuffer.Get(), NULL, renderTargetView_.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer{};
	D3D11_TEXTURE2D_DESC texture2dDesc{};
	texture2dDesc.Width = frameBufferDimensions_.cx;
	texture2dDesc.Height = frameBufferDimensions_.cy;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;
	hr = device_->CreateTexture2D(&texture2dDesc, NULL, depthStencilBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = device_->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(frameBufferDimensions_.cx);
	viewport.Height = static_cast<float>(frameBufferDimensions_.cy);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deviceContext_->RSSetViewports(1, &viewport);
}

void Graphics::OnSizeChanged(UINT64 width, UINT height)
{
	HRESULT hr = S_OK;
	if (width != frameBufferDimensions_.cx || height != frameBufferDimensions_.cy)
	{
		frameBufferDimensions_.cx = static_cast<LONG>(width);
		frameBufferDimensions_.cy = height;

		// Release all objects that hold shader resource views here.
#ifdef ENABLE_DIRECT2D
		d2d1DeviceContext_.Reset();
#endif

		//	FrameBufferリセット(3番目はリセットしない)
		frameBuffers_[0].reset();
		frameBuffers_[1].reset();

		//	Bloomを作り直す


		Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory6;
		hr = swapChain_->GetParent(IID_PPV_ARGS(dxgiFactory6.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		CreateSwapChain(dxgiFactory6.Get());

		// Recreate all objects that hold shader resource views here.
#ifdef ENABLE_DIRECT2D
		CreateDirect2dObjects();
#endif
		//	サイズ再設定
		frameBuffers_[0] = std::make_unique<FrameBuffer>(device_.Get(), SCREEN_WIDTH, SCREEN_HEIGHT);
		frameBuffers_[1] = std::make_unique<FrameBuffer>(device_.Get(), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	}
}

void Graphics::StylizeWindow(bool fullscreen)
{
	fullScreenMode_ = fullscreen;

	if (fullscreen)
	{
		GetWindowRect(hwnd_, &windowedRect_);
		SetWindowLongPtrA(hwnd_, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		RECT fullscreenWindowRect;

		HRESULT hr{ E_FAIL };
		if (swapChain_)
		{
			Microsoft::WRL::ComPtr<IDXGIOutput> dxgiOutput;
			hr = swapChain_->GetContainingOutput(&dxgiOutput);
			if (hr == S_OK)
			{
				DXGI_OUTPUT_DESC outputDesc;
				hr = dxgiOutput->GetDesc(&outputDesc);
				if (hr == S_OK)
				{
					fullscreenWindowRect = outputDesc.DesktopCoordinates;
				}
			}
		}
		if (hr != S_OK)
		{
			DEVMODE devmode = {};
			devmode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);

			fullscreenWindowRect = {
				devmode.dmPosition.x,
				devmode.dmPosition.y,
				devmode.dmPosition.x + static_cast<LONG>(devmode.dmPelsWidth),
				devmode.dmPosition.y + static_cast<LONG>(devmode.dmPelsHeight)
			};
		}
		SetWindowPos(
			hwnd_,
#ifdef _DEBUG
			NULL,
#else
			NULL/*HWND_TOPMOST*/,
#endif
			fullscreenWindowRect.left,
			fullscreenWindowRect.top,
			fullscreenWindowRect.right,
			fullscreenWindowRect.bottom,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(hwnd_, SW_MAXIMIZE);
	}
	else
	{
		SetWindowLongPtrA(hwnd_, GWL_STYLE, windowedStyle_);
		SetWindowPos(
			hwnd_,
			HWND_NOTOPMOST,
			windowedRect_.left,
			windowedRect_.top,
			windowedRect_.right - windowedRect_.left,
			windowedRect_.bottom - windowedRect_.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(hwnd_, SW_NORMAL);
	}
}

//	垂直同期設定
void Graphics::SetIsVSync(bool isVSync)
{
	isVSync_ = isVSync;
	if (isVSync_ == false)vSyncInterval_ = 0;
}

//	バックバッファに描画された内容をフロントバッファに表示するためにスワップする
void Graphics::PresentFrame()
{
	HRESULT hr = S_OK;
	vSyncInterval_= { isVSync_ ? 1U : 0U };
	UINT flags = (tearingSupported_ && !fullScreenMode_ && !isVSync_) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	hr = swapChain_->Present(vSyncInterval_, flags);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

size_t Graphics::VideoMemoryUsage()
{
	DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
	adapter_->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
	return videoMemoryInfo.CurrentUsage / 1024 / 1024;
}

//	デバッグ描画
void Graphics::DrawDebug()
{
	if (ImGui::TreeNode(u8"Graphics"))
	{
		//	垂直同期のオンオフ
		if (ImGui::Checkbox(u8"垂直同期設定", &isVSync_))
		{
			SetIsVSync(isVSync_);	//	チェックボックスの状態に応じて isVSync_ を切り替える
		}
		int vSyncInterval = vSyncInterval_;
		ImGui::DragInt(u8"垂直同期間隔", &vSyncInterval);	//	垂直同期間隔を確認
		bool previousFullScreenMode = fullScreenMode_;
		if (ImGui::Checkbox(u8"フルスクリーン切替", &fullScreenMode_))
		{
			if (fullScreenMode_ != previousFullScreenMode)
			{
				StylizeWindow(fullScreenMode_);
			}
			//	ディスプレイ設定の拡大縮小率に対応したい
			//if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) == FALSE)
			//{
			//	// 失敗した場合のエラーハンドリング
			//	MessageBox(NULL, L"DPIスケーリングの設定に失敗しました", L"エラー", MB_OK);
			//}
			
			//	フルスクリーンをオフにしたときにウィンドウの上にバーが追加される
			if (!fullScreenMode_)
			{
				// ウィンドウサイズを取得
				RECT rect;
				GetClientRect(hwnd_, &rect);
				ImVec2 clientSize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

				RECT windowRect = { 0, 0, (LONG)clientSize.x, (LONG)clientSize.y };
				AdjustWindowRect(&windowRect, windowedStyle_, FALSE);

				int windowWidth = windowRect.right - windowRect.left;
				int windowHeight = windowRect.bottom - windowRect.top;

				SetWindowLongPtr(hwnd_, GWL_STYLE, WS_OVERLAPPEDWINDOW);
				SetWindowPos(hwnd_, HWND_TOP, 0, 0, windowWidth, windowHeight, SWP_FRAMECHANGED | SWP_SHOWWINDOW);

			}

		}

		{
			ImVec2 displaySize = ImGui::GetIO().DisplaySize;
			ImVec2 framebufferScale = ImGui::GetIO().DisplayFramebufferScale;
			ImGui::Text("Display Size: %.2f, %.2f", displaySize.x, displaySize.y);
			ImGui::Text("Framebuffer Scale: %.2f, %.2f", framebufferScale.x, framebufferScale.y);
		}

		ImGui::DragFloat4("LightDirection", &sceneConstant_.lightDirection_.x);
		ImGui::TreePop();
	}


}
