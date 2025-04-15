#include "Bloom.h"

#include <vector>

#include "../Graphics/Graphics.h"
#include "../Others/Misc.h"
#include "../../imgui/imgui.h"

Bloom::Bloom(ID3D11Device* device, uint32_t width, uint32_t height)
{
	bitBlockTransfer_ = std::make_unique<FullScreenQuad>(device);

	glowExtraction_ = std::make_unique<FrameBuffer>(device, width, height, false);
	for (size_t downsampledIndex = 0; downsampledIndex < downsampledCount_; ++downsampledIndex)
	{
		gaussianBlur_[downsampledIndex][0] = std::make_unique<FrameBuffer>(device, width >> downsampledIndex, height >> downsampledIndex, false);
		gaussianBlur_[downsampledIndex][1] = std::make_unique<FrameBuffer>(device, width >> downsampledIndex, height >> downsampledIndex, false);
	}
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/GlowExtractionPs.cso",			glowExtractionPs_.GetAddressOf());
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/GaussianBlurDownsamplingPs.cso",	gaussianBlurDownsamplingPs_.GetAddressOf());
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/GaussianBlurHorizontalPs.cso",	gaussianBlurHorizontalPs_.GetAddressOf());
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/GaussianBlurVerticalPs.cso",		gaussianBlurVerticalPs_.GetAddressOf());
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/GaussianBlurUpsamplingPs.cso",	gaussianBlurUpsamplingPs_.GetAddressOf());

	HRESULT hr{ S_OK };

	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	hr = device->CreateRasterizerState(&rasterizerDesc, rasterizerState_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blendDesc, blendState_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(BloomConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void Bloom::Make(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* colorMap)
{
	//	Store current states
	ID3D11ShaderResourceView* nullShaderResourceView{};
	ID3D11ShaderResourceView* cachedShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	deviceContext->PSGetShaderResources(0, downsampledCount_, cachedShaderResourceViews);

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>	cachedDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>	cachedRasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState>		cachedBlendState;
	FLOAT blendFactor[4];
	UINT  sampleMask;
	deviceContext->OMGetDepthStencilState(cachedDepthStencilState.GetAddressOf(), 0);
	deviceContext->RSGetState(cachedRasterizerState.GetAddressOf());
	deviceContext->OMGetBlendState(cachedBlendState.GetAddressOf(), blendFactor, &sampleMask);

	Microsoft::WRL::ComPtr<ID3D11Buffer>  cachedConstantBuffer;
	deviceContext->PSGetConstantBuffers(8, 1, cachedConstantBuffer.GetAddressOf());

	//	Bind states
	deviceContext->OMSetDepthStencilState(depthStencilState_.Get(), 0);
	deviceContext->RSSetState(rasterizerState_.Get());
	deviceContext->OMSetBlendState(blendState_.Get(), nullptr, 0xFFFFFFFF);

	BloomConstants data{};
	data.bloomExtractionThreshold_ = bloomExtractionThreshold_;
	data.bloomIntensity_ = bloomIntensity_;
	deviceContext->UpdateSubresource(constantBuffer_.Get(), 0, 0, &data, 0, 0);
	deviceContext->PSSetConstantBuffers(8, 1, constantBuffer_.GetAddressOf());

	//	Extracting bright color
	glowExtraction_->Clear(deviceContext, 0, 0, 0, 1);
	glowExtraction_->Activate(deviceContext);
	bitBlockTransfer_->Blit(deviceContext, &colorMap, 0, 1, glowExtractionPs_.Get());
	glowExtraction_->Deactivate(deviceContext);
	deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

	//	Gaussian blur
	//	Efficient Gaussian blur with linear sampling
	//	Downsampling
	gaussianBlur_[0][0]->Clear(deviceContext, 0, 0, 0, 1);
	gaussianBlur_[0][0]->Activate(deviceContext);
	bitBlockTransfer_->Blit(deviceContext, glowExtraction_->shaderResourceViews_[0].GetAddressOf(), 0, 1, gaussianBlurDownsamplingPs_.Get());
	gaussianBlur_[0][0]->Deactivate(deviceContext);
	deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

	//	Ping-pong gaussian blur
	gaussianBlur_[0][1]->Clear(deviceContext, 0, 0, 0, 1);
	gaussianBlur_[0][1]->Activate(deviceContext);
	bitBlockTransfer_->Blit(deviceContext, gaussianBlur_[0][0]->shaderResourceViews_[0].GetAddressOf(), 0, 1, gaussianBlurHorizontalPs_.Get());
	gaussianBlur_[0][1]->Deactivate(deviceContext);
	deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

	gaussianBlur_[0][0]->Clear(deviceContext, 0, 0, 0, 1);
	gaussianBlur_[0][0]->Activate(deviceContext);
	bitBlockTransfer_->Blit(deviceContext, gaussianBlur_[0][1]->shaderResourceViews_[0].GetAddressOf(), 0, 1, gaussianBlurVerticalPs_.Get());
	gaussianBlur_[0][0]->Deactivate(deviceContext);
	deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

	for (size_t downsampledIndex = 1; downsampledIndex < downsampledCount_; ++downsampledIndex)
	{
		//	Downsampling
		gaussianBlur_[downsampledIndex][0]->Clear(deviceContext, 0, 0, 0, 1);
		gaussianBlur_[downsampledIndex][0]->Activate(deviceContext);
		bitBlockTransfer_->Blit(deviceContext, gaussianBlur_[downsampledIndex - 1][0]->shaderResourceViews_[0].GetAddressOf(), 0, 1, gaussianBlurDownsamplingPs_.Get());
		gaussianBlur_[downsampledIndex][0]->Deactivate(deviceContext);
		deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

		//	Ping-pong gaussian blur
		gaussianBlur_[downsampledIndex][1]->Clear(deviceContext, 0, 0, 0, 1);
		gaussianBlur_[downsampledIndex][1]->Activate(deviceContext);
		bitBlockTransfer_->Blit(deviceContext, gaussianBlur_[downsampledIndex][0]->shaderResourceViews_[0].GetAddressOf(), 0, 1, gaussianBlurHorizontalPs_.Get());
		gaussianBlur_[downsampledIndex][1]->Deactivate(deviceContext);
		deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

		gaussianBlur_[downsampledIndex][0]->Clear(deviceContext, 0, 0, 0, 1);
		gaussianBlur_[downsampledIndex][0]->Activate(deviceContext);
		bitBlockTransfer_->Blit(deviceContext, gaussianBlur_[downsampledIndex][1]->shaderResourceViews_[0].GetAddressOf(), 0, 1, gaussianBlurVerticalPs_.Get());
		gaussianBlur_[downsampledIndex][0]->Deactivate(deviceContext);
		deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);
	}

	//	Downsampling
	glowExtraction_->Clear(deviceContext, 0, 0, 0, 1);
	glowExtraction_->Activate(deviceContext);
	std::vector<ID3D11ShaderResourceView*> shaderResourceViews;
	for (size_t downsampledIndex = 0; downsampledIndex < downsampledCount_; ++downsampledIndex)
	{
		shaderResourceViews.emplace_back(gaussianBlur_[downsampledIndex][0]->shaderResourceViews_[0].Get());
	}
	bitBlockTransfer_->Blit(deviceContext, shaderResourceViews.data(), 0, downsampledCount_, gaussianBlurUpsamplingPs_.Get());
	glowExtraction_->Deactivate(deviceContext);
	deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);

	//	Restore states
	deviceContext->PSSetConstantBuffers(8, 1, cachedConstantBuffer.GetAddressOf());

	deviceContext->OMSetDepthStencilState(cachedDepthStencilState.Get(), 0);
	deviceContext->RSSetState(cachedRasterizerState.Get());
	deviceContext->OMSetBlendState(cachedBlendState.Get(), blendFactor, sampleMask);

	deviceContext->PSSetShaderResources(0, downsampledCount_, cachedShaderResourceViews);
	for (ID3D11ShaderResourceView* cachedShaderResourceView : cachedShaderResourceViews)
	{
		if (cachedShaderResourceView) cachedShaderResourceView->Release();
	}
}

void Bloom::DrawDebug()
{
	if (ImGui::TreeNode(u8"Bloom"))
	{
		ImGui::SliderFloat("BloomExtractionThreshold", &bloomExtractionThreshold_, +0.0f, +5.0f);
		ImGui::SliderFloat("BloomIntensity", &bloomIntensity_, +0.0f, +5.0f);
		ImGui::TreePop();
	}
}
