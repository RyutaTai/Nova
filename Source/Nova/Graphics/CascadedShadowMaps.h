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
	float splitSchemeWeight_ = 0.7f; // logarithmic_split_scheme * _split_scheme_weight + uniform_split_scheme * (1 - _split_scheme_weight)
	// https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps
	// Fit to scene vs.fit to cascade
	// - Fit to Scene
	//	All of the frusta can be created with the same near plane.This forces the cascades to overlap.
	// - Fit to Cascade
	//	Alternatively, frusta can be created with the actual partition interval being used as near and far planes.This causes a tighter fit, but degenerates to fit to scene in the case of dueling frusta.
	// Fit to cascade wastes less resolution.The problem with fit to cascade is that the orthographic projection grows and shrinks based on the orientation of the view frustum.
	// The fit to scene technique pads the orthographic projection by the max size of the view frustum removing the artifacts that appear when the view - camera moves.
		// Common Techniques to Improve Shadow Depth Maps addresses the artifacts that appear when the light moves in the section "Moving the light in texel sized increments."
	bool fitToCascade_ = true;
	// Before creating the actual projection matrix we are going to increase the size of the space covered by the nearand far plane of the light frustum.
	// We do this by "pulling back" the near plane, and "pushing away" the far plane.In the code we achieve this by dividing or multiplying by zMult.
	// This is because we want to include geometry which is behind or in front of our frustum in camera space. Think about it : not only geometry which 
	// is in the frustum can cast shadows on a surface in the frustum!
	float zMult_ = 10.0f;

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
		float				shadowColor_ = 0.2f;
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

