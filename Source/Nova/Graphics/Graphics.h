#pragma once

#include <wrl.h>
#include <memory>
#include <d3d11.h>
#include <DirectXMath.h>
#include <mutex>
#include <dxgi1_6.h>

#include "../Graphics/Framebuffer.h"
#include "../Graphics/FullScreenQuad.h"
#include "../Graphics/Shader.h"
#include "../Debug/DebugRenderer.h"

CONST LONG SCREEN_WIDTH{ 1980 };
CONST LONG SCREEN_HEIGHT{ 1080 };
CONST BOOL FULLSCREEN{ TRUE };

class Graphics
{
public:
	//	シーン定数バッファ
	struct SceneConstants
	{
		DirectX::XMFLOAT4X4 viewProjection_ = {};
		DirectX::XMFLOAT4   lightDirection_ = {};
		DirectX::XMFLOAT4   cameraPosition_ = {};
		DirectX::XMFLOAT4X4 lightViewProjection_ = {};
		DirectX::XMFLOAT4X4 invViewProjection_ = {};
		DirectX::XMFLOAT4X4 invProjection_ = {};
	};

public:
	Graphics(HWND hwnd, bool fullscreen);
	~Graphics() = default;

	static Graphics& Instance()
	{
		return *instance_;
	}
	void DrawDebug();

	void	AcquireHighPerformanceAdapter(IDXGIFactory6* dxgiFactory6, IDXGIAdapter3** dxgiAdapter3);
	void	CreateSwapChain(IDXGIFactory6* dxgiFactory6);
	void	OnSizeChanged(UINT64 width, UINT height);
	void	StylizeWindow(const bool& fullscreen);
	void	PresentFrame();
	size_t	VideoMemoryUsage();

	void ClearSceneConstant() { sceneConstant_ = {}; }

	void SetSceneConstant(const SceneConstants& sceneConstant)					{ sceneConstant_ = sceneConstant; }
	void SetViewProjection(const DirectX::XMFLOAT4X4& viewProjection)			{ sceneConstant_.viewProjection_ = viewProjection; }
	void SetViewProjection(const DirectX::XMMATRIX& viewProjection)				{ DirectX::XMStoreFloat4x4(&sceneConstant_.viewProjection_, viewProjection); }
	void SetLightDirection(const DirectX::XMFLOAT4& lightDirection)				{ sceneConstant_.lightDirection_ = lightDirection; }
	void SetCameraPosition(const DirectX::XMFLOAT4& cameraPosition)				{ sceneConstant_.cameraPosition_ = cameraPosition; }
	void SetLightViewProjection(const DirectX::XMFLOAT4X4& lightViewProjection) { sceneConstant_.lightViewProjection_ = lightViewProjection; }
	void SetInvViewProjection(const DirectX::XMFLOAT4X4& invViewProjection)		{ sceneConstant_.invViewProjection_ = invViewProjection; }
	void SetInvViewProjection(const DirectX::XMMATRIX& invViewProjection)		{ DirectX::XMStoreFloat4x4(&sceneConstant_.invViewProjection_, invViewProjection); }
	void SetInvProjection(const DirectX::XMMATRIX& invProjection)				{ DirectX::XMStoreFloat4x4(&sceneConstant_.invProjection_, invProjection); }
	void SetIsVSync(const bool& isVSync);

	CONST HWND					GetHwnd()					CONST	{ return hwnd_; }
	ID3D11Device*				GetDevice()					const	{ return device_.Get(); }
	ID3D11DeviceContext*		GetDeviceContext()			const 	{ return deviceContext_.Get(); }
	IDXGISwapChain1*			GetSwapChain()				const	{ return swapChain_.Get(); }
	ID3D11RenderTargetView*		GetRenderTargetView()		const	{ return renderTargetView_.Get(); }
	ID3D11DepthStencilView*		GetDepthStencilView()		const	{ return depthStencilView_.Get(); }
	ID3D11Buffer*				GetConstantBuffer()			const	{ return constantBuffer_.Get(); }
	Shader*						GetShader()					const	{ return shader_.get(); }
	const SceneConstants		GetSceneConstant()			const	{ return sceneConstant_; }
	const DirectX::XMFLOAT4X4	GetViewProjection()			const	{ return sceneConstant_.viewProjection_; }
	const DirectX::XMFLOAT4		GetLightDirection()			const	{ return sceneConstant_.lightDirection_; }
	const DirectX::XMFLOAT4		GetCameraPosition()			const	{ return sceneConstant_.cameraPosition_; }
	const DirectX::XMFLOAT4X4	GetLightViewProjection()	const	{ return sceneConstant_.lightViewProjection_; }
	const DirectX::XMFLOAT4X4	GetInvViewProjection()		const	{ return sceneConstant_.invViewProjection_; }
	DebugRenderer*				GetDebugRenderer()			const	{ return debugRenderer_.get(); }
	std::mutex&					GetMutex()							{ return mutex_; }			//ミューテックス取得

private:
	static Graphics*								instance_;
	std::unique_ptr<Shader>							shader_				= nullptr;
	Microsoft::WRL::ComPtr<ID3D11Device>			device_				= nullptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		deviceContext_		= nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain1>			swapChain_			= nullptr;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	renderTargetView_	= nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView_	= nullptr;

	SceneConstants	sceneConstant_ = {};

	Microsoft::WRL::ComPtr<ID3D11Buffer>	constantBuffer_		= nullptr;
	std::unique_ptr<FrameBuffer>			frameBuffers_[8]	= { nullptr };
	std::unique_ptr<FullScreenQuad>			fullScreenQuad_	= nullptr;

	//	垂直同期
	bool	isVSync_ = true;			//	垂直同期フラグ
	UINT	vSyncInterval_ = 1;			//	垂直同期間隔(0なら垂直同期オフ、1ならオン)

	//	Fullscreen
	CONST HWND	hwnd_;
	Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter_;
	bool	fullScreenMode_		= false;
	bool	tearingSupported_	= false;
	RECT	windowedRect_;
	DWORD	windowedStyle_;
	SIZE	frameBufferDimensions_;

private:
	//	デバッグプリミティブ
	std::unique_ptr<DebugRenderer>	debugRenderer_ = nullptr;	//	デバッグプリミティブ描画用
	std::mutex						mutex_;

};

