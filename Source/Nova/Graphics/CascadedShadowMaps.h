#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

#include <vector>
#include <functional>

class CascadedShadowMaps
{
public:
	CascadedShadowMaps(ID3D11Device* device, UINT width, UINT height, UINT cascadeCount = 4);
	virtual ~CascadedShadowMaps() = default;
	CascadedShadowMaps(const CascadedShadowMaps&) = delete;
	CascadedShadowMaps& operator =(const CascadedShadowMaps&) = delete;
	CascadedShadowMaps(CascadedShadowMaps&&) noexcept = delete;
	CascadedShadowMaps& operator =(CascadedShadowMaps&&) noexcept = delete;

public:
	void Activate(ID3D11DeviceContext* deviceContext,
		const DirectX::XMFLOAT4X4& cameraView,
		const DirectX::XMFLOAT4X4& camerProjection,
		const DirectX::XMFLOAT4& lightDirection,
		float criticalDepthValue /*If this value is 0, the camera's far panel distance is used.*/,
		UINT cbSlot);
	void Deactivate(ID3D11DeviceContext* deviceContext);
	void Clear(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH, 1, 0);
	}
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& DepthMap()
	{
		return shaderResourceView_;
	}

	void DrawDebug();

public:
	const UINT cascadeCount_;
	float splitSchemeWeight_ = 0.55f;
	bool fitToCascade_ = true;
	float zMult_ = 1.5f;

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer_;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;
	D3D11_VIEWPORT viewport_;

	std::vector<DirectX::XMFLOAT4X4> cascadedMatrices_;
	std::vector<float> cascadedPlaneDistances_;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;

	struct ShadowConstants
	{
		DirectX::XMFLOAT4X4 cascadedMatrices_[4];
		float				cascadedPlaneDistances_[4];
		float				shadowColor_ = 0.58f;
		float				shadowDepthBias_ = 0.0001f;
		bool				colorizeCascadedLayer_ = true;
		float				pad_;							//	16バイトアライメントに合わせるため
	};
	ShadowConstants shadowConstants_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;

private:
	D3D11_VIEWPORT cachedViewports_[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	UINT viewportCount_ = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cachedRenderTargetView_;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cachedDepthStencilView_;

};

