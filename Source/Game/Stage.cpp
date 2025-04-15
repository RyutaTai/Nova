#include "Stage.h"

#include <algorithm>

#include "../Nova/Graphics/Graphics.h"
#include "../Nova/Audio/AudioManager.h"
#include "../Nova/Resources/Texture.h"

#include "Player.h"

Stage* Stage::instance_ = nullptr;

Stage::Stage()
{
	//	インスタンスポイント設定
	_ASSERT_EXPR(instance_ == instance_, L"already instance");
	instance_ = this;

	//	コリジョンメッシュ生成
#if 0
	//collisionMesh_ = std::make_unique<decltype(collisionMesh_)::element_type>(Graphics::Instance().GetDevice(), "./Resources/Model/syougiban.glb");
	collisionMesh_ = std::make_unique<decltype(collisionMesh_)::element_type>(Graphics::Instance().GetDevice(), "./Resources/Model/cybercity-2099-v2/city_collision_ground2_correct.glb");
#else
	
	gltfStaticModelResource_ = ResourceManager::Instance().LoadGltfModelStaticResource("./Resources/Model/cybercity-2099-v2/city.gltf");

	GetTransform()->SetPosition(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
	GetTransform()->SetScaleFactor(0.0025f);	//	シティモデル

	collisionMesh_ = std::make_unique<decltype(collisionMesh_)::element_type>(Graphics::Instance().GetDevice(), "./Resources/Model/cybercity-2099-v2/city.gltf");

	//DirectX::XMFLOAT4X4 transform = {};
	//DirectX::XMStoreFloat4x4(&transform, GetTransform()->CalcWorldMatrix(GetTransform()->GetScaleFactor()));
	//collisionMesh_->Transform(transform);
#endif

	//GetTransform()->SetPosition(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

	//	エミッシブ定数バッファ生成
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(EmissiveConstant);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr;
	hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, emissiveConstantBuffer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	FFT定数バッファ生成
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(FFTConstant);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, fftConstantBuffer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//	オーディオスペクトラムカラー初期化
	fftConstant_.color_[1] = { 0.0f, 0.325f, 1.0f, 1.0f };
	//fftConstant_.color_[1] = { 0.1f, 0.2f, 0.3f, 1.0f };


	//	音の周波数データ生成、初期化
	frequency_ = std::make_unique<Frequency>();
	frequency_->Initialize();

	//	オーディオスペクトラム関連初期化
	//	フレームバッファ
	bitBlockTransfer_ = std::make_unique<FullScreenQuad>(Graphics::Instance().GetDevice());
	spectrumFramebuffer_[static_cast<int>(ProjectionMappingType::Circle)] = std::make_unique<FrameBuffer>(Graphics::Instance().GetDevice(), SPECTRUM_WIDTH, SPECTRUM_HEIGHT);
	spectrumFramebuffer_[static_cast<int>(ProjectionMappingType::Waveform)] = std::make_unique<FrameBuffer>(Graphics::Instance().GetDevice(), SPECTRUM_WIDTH, SPECTRUM_HEIGHT);
	Graphics::Instance().GetShader()->CreatePsFromCso(Graphics::Instance().GetDevice(), "./Resources/Shader/SpectrumPS.cso", spectrumWaveformPS_.GetAddressOf());
#if SPECTRUM_CIRCLE
	Graphics::Instance().GetShader()->CreatePsFromCso(Graphics::Instance().GetDevice(), "./Resources/Shader/SpectrumCirclePS.cso", spectrumCirclePS_.GetAddressOf());
#endif	

	//	プロジェクションマッピング初期設定
	projectionMapping_[static_cast<int>(ProjectionMappingType::Waveform)].eye_		= { 72.0f,7.0f,8.8f};
	projectionMapping_[static_cast<int>(ProjectionMappingType::Waveform)].focus_	= { 33.0f,10.0f,-1.0f };
	projectionMapping_[static_cast<int>(ProjectionMappingType::Waveform)].rotation_ = -104.2f;
	projectionMapping_[static_cast<int>(ProjectionMappingType::Waveform)].fovy_		=	10.0f;
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(ProjectionMappingConstant);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, projectionMappingBuffer_[static_cast<int>(ProjectionMappingType::Waveform)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	projectionMapping_[static_cast<int>(ProjectionMappingType::Circle)].eye_		= { 0.0f,32.0f,0.0f };
	projectionMapping_[static_cast<int>(ProjectionMappingType::Circle)].focus_		= { 0.0f,0.0f,0.0f };
	projectionMapping_[static_cast<int>(ProjectionMappingType::Circle)].rotation_	= 0.0f;
	projectionMapping_[static_cast<int>(ProjectionMappingType::Circle)].fovy_		=	10.0f;
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(ProjectionMappingConstant);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, projectionMappingBuffer_[static_cast<int>(ProjectionMappingType::Circle)].ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

#if MAGIC_CIRCLE
	D3D11_TEXTURE2D_DESC texture2dDesc;
	LoadTextureFromFile(Graphics::Instance().GetDevice(), L"./Resources/Image/magic circle.png", projectionMappingTexture_.GetAddressOf(), &texture2dDesc);
#endif

}

//	インスタンス取得
Stage& Stage::Instance()
{
	return *instance_;
}

//	更新処理
void Stage::Update(const float& elapsedTime)
{
	//	エミッシブ更新処理
	UpdateEmissive(elapsedTime);

	//	FFT定数バッファ更新
	UpdateFFTConstantBuffer();

	//	オーディオスペクトラム更新
	UpdateAudioSpectrum(elapsedTime);

}

//	エミッシブ更新処理
void Stage::UpdateEmissive(const float& elapsedTime)
{
#if 1	//	通常
	//	frequencyを使用しないならemissiveIntensity_を1.0fに設定
	if (useFrequency_ == false)emissiveConstant_.emissiveIntensity_ = 1.0f;

	//	周波数データ更新
	frequency_->Update(elapsedTime, AudioManager::Instance().GetAudioResource("GameBGM"));

	//	frequencyData_更新(配列のデータをずらし、新しいデータを設定)
	for (int i = FrequencyDataMax - 1; 0 < i; --i)
	{
		frequencyData_[i] = frequencyData_[i - 1];
	}
	currentFrequencyValue_ = frequency_->GetAmplitudeSpectrum(frequencyIndex_);
	frequencyData_[0] = currentFrequencyValue_;

	//	frequencyの最大値と最小値を更新
	UpdateFrequencyMin();
	UpdateFrequencyMax();

	//	フーリエ変換で取得した振幅
	float frequencyValue = currentFrequencyValue_;
	if (frequencyMaxValue_ > 0.0f)frequencyValue = (frequencyValue - frequencyMinValue_) / frequencyMaxValue_;

	//	BPM取得
	//float bpm = AudioManager::Instance().GetAudioResource("Game.wav")->GetWaveFormat().GetBPM();

	//	emissiveIntensity_更新
	emissiveConstant_.emissiveIntensity_ = frequencyValue * emissiveFactor_;

#else	//	currentFrequencyValue_にcurrent閾値を設定し、それを越していないならエミッシブにデフォルト値を設定してreturnする

	//	frequencyを使用しないならemissiveIntensityを1.0fに設定
	if (useFrequency_ == false)emissiveConstant_.emissiveIntensity_ = 1.0f;

	//	周波数データ更新
	frequency_->Update(elapsedTime, AudioManager::Instance().GetAudioResource("Game.wav"));
	//frequency_->Update(elapsedTime, AudioManager::Instance().GetAudioResource("fourOnTheFloor_Basic_44100Hz_16bit.wav"));

	//	frequencyData_更新(配列のデータをずらし、新しいデータを設定)
	for (int i = FrequencyDataMax - 1; 0 < i; --i)
	{
		frequencyData_[i] = frequencyData_[i - 1];
	}
	currentFrequencyValue_ = frequency_->GetAmplitudeSpectrum(frequencyIndex_);

	//	閾値に達していなければデフォルト値を設定してreturn
	if (currentFrequencyValue_ < threshold_)
	{
		emissiveConstant_.emissiveIntensity_ = defaultEmissiveIntensity_;
		return;
	}

	frequencyData_[0] = currentFrequencyValue_;

	//	frequencyの最大値と最小値を更新
	UpdateFrequencyMin();
	UpdateFrequencyMax();

	//	フーリエ変換で取得した振幅
	float frequencyValue = currentFrequencyValue_;
	if (frequencyMaxValue_ > 0.0f)frequencyValue = (frequencyValue - frequencyMinValue_) / frequencyMaxValue_;

	//	BPM取得
	//float bpm = AudioManager::Instance().GetAudioResource("Game.wav")->GetWaveFormat().GetBPM();

	//	emissiveIntensity_更新
	emissiveConstant_.emissiveIntensity_ = frequencyValue * emissiveFactor_;
	/*emissiveConstant_.emissiveIntensity_ = std::clamp(currentFrequencyValue,
		emissiveIntencityMin_, emissiveIntencityMax_);*/

#endif
}

//	オーディオスペクトラムk更新
void Stage::UpdateAudioSpectrum(const float& elapsedTime)
{
	UpdateCircleAudioSpectrum(elapsedTime);
	UpdateWaveformAudioSpectrum();
}

//	波形オーディオスペクトラム更新
void Stage::UpdateWaveformAudioSpectrum()
{
	int projectionMappingIndex = static_cast<int>(ProjectionMappingType::Waveform);
	float projectionMappingRotation = projectionMapping_[projectionMappingIndex].rotation_;
	DirectX::XMFLOAT3 projectionMappingEye = projectionMapping_[projectionMappingIndex].eye_;
	DirectX::XMFLOAT3 projectionMappingFocus = projectionMapping_[projectionMappingIndex].focus_;
	float projectionMappingFovy = projectionMapping_[projectionMappingIndex].fovy_;
	DirectX::XMMATRIX ProjectionMappingTransform =
		DirectX::XMMatrixLookAtLH(
			DirectX::XMLoadFloat3(&projectionMappingEye),
			DirectX::XMLoadFloat3(&projectionMappingFocus),
			DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), DirectX::XMMatrixRotationRollPitchYaw(0, DirectX::XMConvertToRadians(projectionMappingRotation), 0))) *
		DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(projectionMappingFovy), 1.0f, 1.0f, 500.0f);
	DirectX::XMStoreFloat4x4(&projectionMappingConstants_[projectionMappingIndex].transform_, ProjectionMappingTransform);

	//	定数バッファをGPUに送る
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(projectionMappingBuffer_[projectionMappingIndex].Get(), 0, 0, &projectionMappingConstants_[projectionMappingIndex], 0, 0);
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(5, 1, projectionMappingBuffer_[projectionMappingIndex].GetAddressOf());

}

//	円形オーディオスペクトラム更新
void Stage::UpdateCircleAudioSpectrum(const float& elapsedTime)
{
	//	座標更新
	int projectionMappingIndex = static_cast<int>(ProjectionMappingType::Circle);
	DirectX::XMFLOAT3 projectionMappingEye = Player::Instance().GetTransform()->GetPosition();					//	プレイヤーの位置
	DirectX::XMFLOAT3 projectionMappingFocus = Player::Instance().GetTransform()->GetPosition();	//	注視点
	projectionMapping_[projectionMappingIndex].focus_ = projectionMappingFocus;

	projectionMappingEye.y += eyeHeight_;								//	視点をプレイヤーの真上から投影するように設定
	
	projectionMapping_[projectionMappingIndex].eye_ = projectionMappingEye;

	//	回転値更新
	float projectionMappingRotation = projectionMapping_[projectionMappingIndex].rotation_;
	//projectionMappingRotation += 90.0f * elapsedTime;
	if (projectionMappingRotation > 360.0f)
	{
		projectionMappingRotation = 0.0f;
	}
	projectionMapping_[projectionMappingIndex].rotation_ = projectionMappingRotation;

	float projectionMappingFovy = projectionMapping_[projectionMappingIndex].fovy_;
	DirectX::XMMATRIX ProjectionMappingTransform =
		DirectX::XMMatrixLookAtLH(	
			DirectX::XMLoadFloat3(&projectionMappingEye),
			DirectX::XMLoadFloat3(&projectionMappingFocus),
			DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), DirectX::XMMatrixRotationRollPitchYaw(0, DirectX::XMConvertToRadians(projectionMappingRotation), 0))) *
		DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(projectionMappingFovy), 1.0f, 1.0f, 500.0f);
	DirectX::XMStoreFloat4x4(&projectionMappingConstants_[projectionMappingIndex].transform_, ProjectionMappingTransform);

	//	定数バッファをGPUに送る
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(projectionMappingBuffer_[projectionMappingIndex].Get(), 0, 0, &projectionMappingConstants_[projectionMappingIndex], 0, 0);
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(4, 1, projectionMappingBuffer_[projectionMappingIndex].GetAddressOf());
	
}

//	振幅最小値更新処理
void Stage::UpdateFrequencyMin()
{
	frequencyMinValue_ = FLT_MAX;				//	周波数の最小値
	for (int i = 0; i < FrequencyDataMax; ++i)
	{
		if (frequencyData_[i] < frequencyMinValue_)	//	最小値更新
		{
			frequencyMinValue_ = frequencyData_[i];
		}
	}
}

//	振幅最大値更新処理
void Stage::UpdateFrequencyMax()
{
	frequencyMaxValue_ = FLT_MIN;
	for (int i = 0; i < FrequencyDataMax; ++i)
	{
		float frequency = frequencyData_[i] - frequencyMinValue_;
		if (frequencyMaxValue_ < frequency)	//	最大値更新
		{
			frequencyMaxValue_ = frequency;
		}
	}
}

float Stage::CalculateAutocorrelation(const float data[], const int& lag)
{
	float result = 0.0f;

	for (int i = 0; i < FrequencyDataMax - lag; ++i)
	{
		result += data[i] * data[i + lag];
	}

	return result / (FrequencyDataMax - lag);
}

//	コリジョンメッシュの当たり判定
bool Stage::Collision(_In_ const DirectX::XMFLOAT3& rayStartPosition, _In_ const DirectX::XMFLOAT3& rayDirection, _In_ const DirectX::XMFLOAT4X4& stageTransform, _Out_ DirectX::XMFLOAT3& intersectionPosition, _Out_ DirectX::XMFLOAT3& intersectionNormal,
	_Out_ std::string& intersectionMesh, _Out_ std::string& intersectionMaterial, _In_ float rayLengthLimit, _In_ bool skipIf) const
{
#if 0
	if (collisionMesh_->Raycast(rayStartPosition, rayDirection, transform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial, rayLengthLimit, skipIf))
	{
		OutputDebugStringA("Position:");
		OutputDebugStringA("Intersected : ");
		OutputDebugStringA(intersectionMesh.c_str());
		OutputDebugStringA(" : ");
		OutputDebugStringA(intersectionMaterial.c_str());
		OutputDebugStringA("\n");
		return true;
	}
	else
	{
		OutputDebugStringA("Unintersected...\n");
		return false;
	}
#else
	//	空間分割
	if (collisionMesh_->RaycastWithSpaceDivision(rayStartPosition, rayDirection, stageTransform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial, rayLengthLimit, skipIf))
	{
		//OutputDebugStringA("Position:");
		//OutputDebugStringA("Intersected : ");
		//OutputDebugStringA(intersectionMesh.c_str());
		//OutputDebugStringA(" : ");
		//OutputDebugStringA(intersectionMaterial.c_str());
		//OutputDebugStringA("\n");
		return true;
	}
	else
	{
		//OutputDebugStringA("Unintersected...\n");
		return false;
	}
#endif
}

//	Shadow描画
void Stage::ShadowRender(const float& scale)
{
#if 0
	ID3D11RenderTargetView* null_render_target_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	Graphics::Instance().GetDeviceContext()->OMSetRenderTargets(_countof(null_render_target_views), null_render_target_views, 0);
	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	Graphics::Instance().GetDeviceContext()->VSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
	Graphics::Instance().GetDeviceContext()->PSSetShaderResources(0, _countof(null_shader_resource_views), null_shader_resource_views);
#endif
	ID3D11PixelShader* nullPixelShader{ NULL };

	//gltfStaticModelResource_->Render(GetTransform()->CalcWorldMatrix(scale));
	//gltfStaticModelResource_->Render(GetTransform()->CalcWorldMatrix(scale), GetTransform()->GetColor(), nullptr, &nullPixelShader);

}

//	描画処理
void Stage::Render()
{
	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
	// PROJECTION_MAPPING
	UpdateFFTConstantBuffer();
	
#if MAGIC_CIRCLE
	Graphics::Instance().GetDeviceContext()->PSSetShaderResources(15, 1, projectionMappingTexture_.GetAddressOf());
#else
	//	円形のオーディオスペクトラム
	int spectrumIndex = static_cast<int>(ProjectionMappingType::Circle);
	spectrumFramebuffer_[spectrumIndex]->Clear(deviceContext, 0, 0, 0, 1);
	spectrumFramebuffer_[spectrumIndex]->Activate(deviceContext);
	bitBlockTransfer_->Blit(deviceContext, projectionMapping_[static_cast<int>(ProjectionMappingType::Circle)].texture_.GetAddressOf(), 1, 0, spectrumCirclePS_.Get());
	spectrumFramebuffer_[spectrumIndex]->Deactivate(deviceContext);
	Graphics::Instance().GetDeviceContext()->PSSetShaderResources(15, 1, spectrumFramebuffer_[spectrumIndex]->shaderResourceViews_[0].GetAddressOf());
	
	//	波形のオーディオスペクトラム
	spectrumIndex = static_cast<int>(ProjectionMappingType::Waveform);
	spectrumFramebuffer_[spectrumIndex]->Clear(deviceContext, 0, 0, 0, 1);
	spectrumFramebuffer_[spectrumIndex]->Activate(deviceContext);
	bitBlockTransfer_->Blit(deviceContext, projectionMapping_[static_cast<int>(ProjectionMappingType::Waveform)].texture_.GetAddressOf(), 1, 0, spectrumWaveformPS_.Get());
	spectrumFramebuffer_[spectrumIndex]->Deactivate(deviceContext);
	Graphics::Instance().GetDeviceContext()->PSSetShaderResources(16, 1, spectrumFramebuffer_[spectrumIndex]->shaderResourceViews_[0].GetAddressOf());


#endif

	//	エミッシブ定数バッファをGPUに送る
	deviceContext->UpdateSubresource(emissiveConstantBuffer_.Get(), 0, 0, &emissiveConstant_, 0, 0);
	deviceContext->PSSetConstantBuffers(3, 1, emissiveConstantBuffer_.GetAddressOf());

	//	ピクセルシェーダーセット
	gltfStaticModelResource_->SetPixelShaderFromName("./Resources/Shader/CityPS.cso");

	gltfStaticModelResource_->Render();		//	描画

}

//	FFT定数バッファ更新
void Stage::UpdateFFTConstantBuffer()
{
	std::vector<float> fftData = frequency_->GetAmplitudeSpectrum(); // FFT結果を取得

#if 1	//	正規化
	std::vector<float> copy = fftData;
	std::sort(copy.begin(), copy.end(), [](float l, float r) {return l > r; });
	float max = copy.at(30);
	/*for (int index = 5; index < fftData.size() - 5 ; ++index)
	{
		if (max < fftData.at(index))max = fftData.at(index);
	}*/

	for (auto& fft : fftData)
	{
		fft /= max;
	}
#else
	for (auto& fft : fftData)
	{
		fft = fft * fft *0.000004f;
		//if (fft < 0.09f)fft *= 10;
	}
#endif


	//	FFT定数バッファ更新
	for (int index = 0; index < Frequency::BlockCount; ++index)
	{
		fftConstant_.fftData_[index] = fftData.at(index);
	}

	int size = sizeof(FFTConstant);

	//	FFT定数バッファをGPUに送る
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(fftConstantBuffer_.Get(), 0, 0, &fftConstant_, 0, 0);
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(10, 1, fftConstantBuffer_.GetAddressOf());

}

//	デバッグ描画
void Stage::DrawDebug()
{
	if (ImGui::TreeNode(u8"Stageステージ"))
	{
		//	コリジョンメッシュ
		collisionMesh_->DrawDebug();

		//	円形のオーディオスペクトラムテクスチャ
		ImGui::Text(u8"CircleSpectrumSRV_Slot15");
		{
			D3D11_VIEWPORT viewport;
			UINT numViewports{ 1 };
			Graphics::Instance().GetDeviceContext()->RSGetViewports(&numViewports, &viewport);
			auto srv = spectrumFramebuffer_[static_cast<int>(ProjectionMappingType::Circle)]->shaderResourceViews_[0].Get();
			ImGui::Image(reinterpret_cast<void*>(srv), ImVec2(viewport.Width / 5.0f, viewport.Height / 5.0f));
		}
		//	波形のオーディオスペクトラムテクスチャ
		ImGui::Text(u8"fftSRV_Slot16");
		{
			D3D11_VIEWPORT viewport;
			UINT numViewports{ 1 };
			Graphics::Instance().GetDeviceContext()->RSGetViewports(&numViewports, &viewport);
			auto srv = spectrumFramebuffer_[static_cast<int>(ProjectionMappingType::Waveform)]->shaderResourceViews_[0].Get();
			ImGui::Image(reinterpret_cast<void*>(srv), ImVec2(viewport.Width / 5.0f, viewport.Height / 5.0f));
		}

		//	プロジェクションマッピング
		if (ImGui::TreeNode("ProjectionMapping"))
		{
			int projectionMappingIndex = static_cast<int>(ProjectionMappingType::Waveform);
			if (ImGui::TreeNode("Waveform"))
			{
				ImGui::PushID(projectionMappingIndex);
				ImGui::ColorEdit4("Color", &fftConstant_.color_[0].x);
				ImGui::DragFloat3("Eye", &projectionMapping_[projectionMappingIndex].eye_.x);
				ImGui::DragFloat3("Focus", &projectionMapping_[projectionMappingIndex].focus_.x);
				ImGui::DragFloat("Rotation", &projectionMapping_[projectionMappingIndex].rotation_);
				ImGui::SliderFloat("Fovy", &projectionMapping_[projectionMappingIndex].fovy_, 10.0f, 180.0f);
				ImGui::PopID();
				ImGui::TreePop();
			}


			if (ImGui::TreeNode("Circle"))
			{
				projectionMappingIndex = static_cast<int>(ProjectionMappingType::Circle);
				ImGui::ColorEdit4("Color", &fftConstant_.color_[1].x);
				ImGui::PushID(projectionMappingIndex);
				ImGui::DragFloat3("Eye", &projectionMapping_[projectionMappingIndex].eye_.x);
				ImGui::DragFloat3("Focus", &projectionMapping_[projectionMappingIndex].focus_.x);
				ImGui::DragFloat("Rotation", &projectionMapping_[projectionMappingIndex].rotation_);
				ImGui::SliderFloat("Fovy", &projectionMapping_[projectionMappingIndex].fovy_, 10.0f, 180.0f);
				ImGui::PopID();
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		//	周波数データのデバッグ描画
		if (ImGui::TreeNode("Frequency Data"))
		{
			ImGui::DragFloat("FFTDivisionValue", &fftDivisionValue_);
			frequency_->DrawDebug();
			ImGui::Checkbox("UseFrequency", &useFrequency_);
			ImGui::DragInt("FrequencyIndex", &frequencyIndex_, 1.0f, 0);
			ImGui::DragFloat("CurrentFrequency", &currentFrequencyValue_, 1.0f, 0.0f);
			ImGui::DragFloat("FrequencyMin", &frequencyMinValue_, 1.0f, 0.0f);
			ImGui::DragFloat("FrequencyMax", &frequencyMaxValue_, 1.0f, 0.0f);
			ImGui::DragFloat("EmissiveIntencity", &emissiveConstant_.emissiveIntensity_, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat("DefaultEmissiveIntencity", &defaultEmissiveIntensity_, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat("EmissiveThreshold", &threshold_, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat("EmissiveFactor", &emissiveFactor_, 1.0f, 0.0f);
			ImGui::DragFloat("EmissiveIntencityMin", &emissiveIntencityMin_, 1.0f, 0.0f);
			ImGui::DragFloat("EmissiveIntencityMax", &emissiveIntencityMax_, 1.0f, 0.0f);
			ImGui::DragFloat("EyeHeight", &eyeHeight_, 1.0f, 0.0f);

			ImGui::TreePop();
		}

		gltfStaticModelResource_->DrawDebug();
		GetTransform()->DrawDebug();
		ImGui::TreePop();
	}
}
