#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>

//	オフスクリーンレンダリングを行う際の、カラーや深度情報を書き込むバッファとそのビューを管理するクラス
class FrameBuffer
{
public:
	FrameBuffer(ID3D11Device* device, uint32_t width, uint32_t height, bool hasDepthstencil = true/*深度ステンシルバッファを持つかどうか*/);
	virtual ~FrameBuffer() = default;

	void Clear(ID3D11DeviceContext* deviceContext,
		const float& r = 0, const float& g = 0, const float& b = 0, const float& a = 1, const float& depth = 1);
	void Activate  (ID3D11DeviceContext* deviceContext);
	void Deactivate(ID3D11DeviceContext* deviceContext);

public:
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView>		renderTargetView_;
	Microsoft::WRL::ComPtr <ID3D11DepthStencilView>		depthStencilView_;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView>	shaderResourceViews_[2];
	D3D11_VIEWPORT										viewport_;

private:
	UINT viewportCount_{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };			//	キャッシュするビューポートの数
	D3D11_VIEWPORT cachedViewports_[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];	//	キャッシュしたビューポートの配列
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView>		cachedRenderTargetView_;				//	キャッシュしたレンダーターゲットビュー
	Microsoft::WRL::ComPtr <ID3D11DepthStencilView>		cachedDepthStencilView_;				//	キャッシュした深度ステンシルビュー

};