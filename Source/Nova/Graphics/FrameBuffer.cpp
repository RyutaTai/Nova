#include "Framebuffer.h"

#include "../Others/Misc.h"

//	コンストラクタ	
FrameBuffer::FrameBuffer(ID3D11Device* device, uint32_t width, uint32_t height, bool hasDepthstencil/*BLOOM*/)
{
	HRESULT hr{ S_OK };

	Microsoft::WRL::ComPtr <ID3D11Texture2D> renderTargetBuffer;
	D3D11_TEXTURE2D_DESC texture2dDesc{};
	texture2dDesc.Width = width;
	texture2dDesc.Height = height;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2dDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texture2dDesc.CPUAccessFlags = 0;
	texture2dDesc.MiscFlags = 0;
	hr = device->CreateTexture2D(&texture2dDesc, 0, renderTargetBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	レンダーターゲットビューを作成
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = texture2dDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	hr = device->CreateRenderTargetView(renderTargetBuffer.Get(), &renderTargetViewDesc,
		renderTargetView_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	シェーダーリソースビューを作成
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = texture2dDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(renderTargetBuffer.Get(), &shaderResourceViewDesc,
		shaderResourceViews_[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	// 深度ステンシルバッファを作成
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	texture2dDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	hr = device->CreateTexture2D(&texture2dDesc, 0, depthStencilBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	深度ステンシルビューを作成
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	シェーダーリソースビューを作成
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &shaderResourceViewDesc, shaderResourceViews_[1].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	if (hasDepthstencil)
	{
		texture2dDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		hr = device->CreateTexture2D(&texture2dDesc, 0, depthStencilBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Flags = 0;
		hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &shaderResourceViewDesc, shaderResourceViews_[1].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//	ビューポートの設定
	viewport_.Width = static_cast<float>(width);
	viewport_.Height = static_cast<float>(height);
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;

}

//	このフレームバッファにバインドされているレンダーターゲットビューと深度ステンシルビューを指定された色と深度値でクリアする
void FrameBuffer::Clear(ID3D11DeviceContext* deviceContext, const float& r, const float& g, const float& b, const float& a, const float& depth)
{
	float color[4]{ r,g,b,a };
	//	レンダーターゲットビューをクリア
	deviceContext->ClearRenderTargetView(renderTargetView_.Get(), color);
	//	深度ステンシルビューが存在する場合にクリア
	if (depthStencilView_)
	{
		deviceContext->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH, depth, 0);
	}
}

//	現在のレンダーターゲットとビューポートの設定をキャッシュし、このFrameBufferのレンダーターゲットビュー、深度ステンシルビュー、
//	ビューポートをデバイスコンテキストに設定する
//	これにより以降の描画命令はこのフレームバッファに対して実行されるようになる
void FrameBuffer::Activate(ID3D11DeviceContext* deviceContext)
{
	//	現在のビューポートの数を取得
	viewportCount_ = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	//	現在のビューポート設定をキャッシュ
	deviceContext->RSGetViewports(&viewportCount_, cachedViewports_);
	//	現在のレンダーターゲットビューと深度ステンシルビューをキャッシュ
	deviceContext->OMGetRenderTargets(1, cachedRenderTargetView_.ReleaseAndGetAddressOf(), cachedDepthStencilView_.ReleaseAndGetAddressOf());

	//	このフレームバッファのビューポートを設定
	deviceContext->RSSetViewports(1, &viewport_);
	//	このフレームバッファのレンダーターゲットビューと深度ステンシルビューを設定
	deviceContext->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());

}

//	Activate関数でキャッシュされた元のレンダーターゲットとビューポートの設定に戻す
//	これによりフレームバッファへの描画を終了し、以前の描画ターゲットに戻る
void FrameBuffer::Deactivate(ID3D11DeviceContext* deviceContext)
{
	//	キャッシュされたビューポート設定を復元
	deviceContext->RSSetViewports(viewportCount_, cachedViewports_);
	//	キャッシュされたレンダーターゲットビューと深度ステンシルビューを復元
	deviceContext->OMSetRenderTargets(1, cachedRenderTargetView_.GetAddressOf(), cachedDepthStencilView_.Get());
}