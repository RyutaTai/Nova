#include "Stage.h"

#include <algorithm>

#include "../../Nova/Graphics/Graphics.h"
#include "../../Nova/Audio/AudioManager.h"
#include "../../Nova/Resources/Texture.h"
#include "../../Nova/Resources/ResourceManager.h"
#include "../../Nova/Others/Easing.h"
#include "../Character/Player/Player.h"
#include "../../Nova/Others/MathHelper.h"

Stage* Stage::instance_ = nullptr;

Stage::Stage()
{
	//	インスタンス設定
	_ASSERT_EXPR(instance_ == instance_, L"already instance");
	instance_ = this;

	//	モデル読み込み
	gltfStaticModelResource_ = ResourceManager::Instance().LoadGltfModelStaticResource("./Resources/Model/Stage/stage.gltf");
	collisionMesh_ = std::make_unique<CollisionMesh>(Graphics::Instance().GetDevice(), "./Resources/Model/Stage/stage.gltf");
	//	位置設定
	GetTransform()->SetPosition(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
	//	スケール設定
	GetTransform()->SetScaleFactor(0.0025f);

	//	音の周波数データ生成、初期化
	frequency_ = std::make_unique<Frequency>();
	frequency_->Initialize();

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

	//	オーディオスペクトラム初期化
	{
		//	オーディオスペクトラムカラー初期化
		fftConstant_.color_[static_cast<int>(AudioSpectrumType::Waveform)] = { 1.0f, 0.0f, 0.0f, 1.0f };
		projectionMapping_[static_cast<int>(AudioSpectrumType::Waveform)].defaultSpectrumColor_ = { 1.0f, 0.0f, 0.0f, 1.0f };
		projectionMapping_[static_cast<int>(AudioSpectrumType::Waveform)].currentSpectrumColor_ = { 1.0f, 0.0f, 0.0f, 1.0f };

		fftConstant_.color_[static_cast<int>(AudioSpectrumType::Circle)] = { 0.0f, 0.325f, 1.0f, 1.0f };
		projectionMapping_[static_cast<int>(AudioSpectrumType::Circle)].eyeOffset_.y = 30.0f;
		projectionMapping_[static_cast<int>(AudioSpectrumType::Circle)].defaultSpectrumColor_ = { 0.0f, 0.325f, 1.0f, 1.0f };
		projectionMapping_[static_cast<int>(AudioSpectrumType::Circle)].currentSpectrumColor_ = { 0.0f, 0.325f, 1.0f, 1.0f };

		for (int i = 0; i < static_cast<int>(AudioSpectrumType::Max); ++i)
		{
			projectionMapping_[i].isTemporaryColorActive_ = false;
			projectionMapping_[i].colorTimer_ = 0.0f;
			projectionMapping_[i].colorDuration_ = 0.8f;

			projectionMapping_[i].isFovyScaleActive_ = false;
			projectionMapping_[i].fovy_ = 10.0f;
			projectionMapping_[i].defaultFovy_ = 10.0f;
			projectionMapping_[i].fovyTimer_ = 0.0f;
			projectionMapping_[i].fovyDuration_ = 0.8f;
		}

		//	フレームバッファ
		fullScreenQuad_ = std::make_unique<FullScreenQuad>(Graphics::Instance().GetDevice());
		spectrumFramebuffer_[static_cast<int>(AudioSpectrumType::Circle)] = std::make_unique<FrameBuffer>(Graphics::Instance().GetDevice(), SPECTRUM_WIDTH, SPECTRUM_HEIGHT);
		spectrumFramebuffer_[static_cast<int>(AudioSpectrumType::Waveform)] = std::make_unique<FrameBuffer>(Graphics::Instance().GetDevice(), SPECTRUM_WIDTH, SPECTRUM_HEIGHT);
		//	オーディオスペクトラムのピクセルシェーダー設定
		Graphics::Instance().GetShader()->CreatePsFromCso(Graphics::Instance().GetDevice(), "./Resources/Shader/SpectrumPS.cso", spectrumWaveformPS_.GetAddressOf());
		Graphics::Instance().GetShader()->CreatePsFromCso(Graphics::Instance().GetDevice(), "./Resources/Shader/SpectrumCirclePS.cso", spectrumCirclePS_.GetAddressOf());

		//	プロジェクションマッピング初期設定
		projectionMapping_[static_cast<int>(AudioSpectrumType::Waveform)].eye_ = { 72.0f,7.0f,8.8f };
		projectionMapping_[static_cast<int>(AudioSpectrumType::Waveform)].defaultEye_ = { 72.0f,7.0f,8.8f };
		projectionMapping_[static_cast<int>(AudioSpectrumType::Waveform)].focus_ = { 33.0f,10.0f,-1.0f };
		projectionMapping_[static_cast<int>(AudioSpectrumType::Waveform)].rotation_ = -104.12f;
		projectionMapping_[static_cast<int>(AudioSpectrumType::Waveform)].fovy_ = 10.0f;
		bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(ProjectionMappingConstant);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, projectionMappingBuffer_[static_cast<int>(AudioSpectrumType::Waveform)].ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		projectionMapping_[static_cast<int>(AudioSpectrumType::Circle)].eye_ = { 0.0f,32.0f,0.0f };
		projectionMapping_[static_cast<int>(AudioSpectrumType::Circle)].defaultEye_ = { 0.0f,32.0f,0.0f };
		projectionMapping_[static_cast<int>(AudioSpectrumType::Circle)].focus_ = { 0.0f,0.0f,0.0f };
		projectionMapping_[static_cast<int>(AudioSpectrumType::Circle)].rotation_ = 0.0f;
		projectionMapping_[static_cast<int>(AudioSpectrumType::Circle)].fovy_ = 10.0f;
		bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(ProjectionMappingConstant);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, projectionMappingBuffer_[static_cast<int>(AudioSpectrumType::Circle)].ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//	モデルのピクセルシェーダーセット
	gltfStaticModelResource_->SetPixelShader("./Resources/Shader/StagePS.cso");

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
	UpdateFFTConstantBuffer(elapsedTime);

	//	オーディオスペクトラム更新
	UpdateAudioSpectrum(elapsedTime);

}

//	エミッシブ更新処理
void Stage::UpdateEmissive(const float& elapsedTime)
{
	//	補完フラグが経っていなければ更新しない
	if (emissiveIsLerp_ == false)return;

	//	補完タイマー更新
	emissiveLerpTimer_ += elapsedTime;

	//	補完しきったらリセット
	if (emissiveLerpTimer_ >= emissiveLerpTimerMax_)
	{
		emissiveIsFadeIn_ = !emissiveIsFadeIn_;
		if (emissiveIsFadeIn_ == false)emissiveIsLerp_ = false;
		emissiveLerpTimer_ = 0.0f;
	}

	//	ヴィネットの強度を強めるか弱めるかで補完する最大値、最小値を切り替える
	if (emissiveIsFadeIn_)
	{
		emissiveData_.currentEmissiveIntensity_ =
			Mathf::Lerp(emissiveData_.emissiveIntensityMin_, emissiveData_.emissiveIntensityMax_, emissiveLerpTimer_ / emissiveLerpTimerMax_);
	}
	else
	{
		emissiveData_.currentEmissiveIntensity_ =
			Mathf::Lerp(emissiveData_.emissiveIntensityMax_, emissiveData_.emissiveIntensityMin_, emissiveLerpTimer_ / emissiveLerpTimerMax_);
	}

	//	リズムUIに合わせてエミッシブを変化させる
	emissiveConstant_.emissiveIntensity_ = emissiveData_.currentEmissiveIntensity_;

}

//	オーディオスペクトラム更新
void Stage::UpdateAudioSpectrum(const float& elapsedTime)
{
	UpdateSpectrumColor(elapsedTime);		//	オーディオスペクトラムの色更新
	UpdateSpectrumFovy(elapsedTime);		//	オーディオスペクトラムのスケール更新
	UpdateCircleAudioSpectrum(elapsedTime);	//	波形オーディオスペクトラム更新
	UpdateWaveformAudioSpectrum();			//	円形オーディオスペクトラム更新
}

//	波形オーディオスペクトラム更新
void Stage::UpdateWaveformAudioSpectrum()
{
	int projectionMappingIndex = static_cast<int>(AudioSpectrumType::Waveform);
	float projectionMappingRotation				= projectionMapping_[projectionMappingIndex].rotation_;
	DirectX::XMFLOAT3 projectionMappingEye		= projectionMapping_[projectionMappingIndex].eye_;
	projectionMappingEye = projectionMappingEye + projectionMapping_[projectionMappingIndex].eyeOffset_;
	DirectX::XMFLOAT3 projectionMappingFocus	= projectionMapping_[projectionMappingIndex].focus_;
	float projectionMappingFovy					= projectionMapping_[projectionMappingIndex].fovy_;
	DirectX::XMMATRIX ProjectionMappingTransform =
		DirectX::XMMatrixLookAtLH(
			DirectX::XMLoadFloat3(&projectionMappingEye),
			DirectX::XMLoadFloat3(&projectionMappingFocus),
			DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), DirectX::XMMatrixRotationRollPitchYaw(0, DirectX::XMConvertToRadians(projectionMappingRotation), 0))) *
		DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(projectionMappingFovy), 1.0f, 1.0f, 500.0f);
	DirectX::XMStoreFloat4x4(&projectionMappingConstants_[projectionMappingIndex].transform_, ProjectionMappingTransform);

	//	定数バッファをGPUに送る
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(projectionMappingBuffer_[projectionMappingIndex].Get(), 0, 0, &projectionMappingConstants_[projectionMappingIndex], 0, 0);
	static constexpr int SpectrumProjectionMappingCBIndex = 5; //	波形プロジェクションマッピング定数バッファのレジスタ番号
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(SpectrumProjectionMappingCBIndex, 1, projectionMappingBuffer_[projectionMappingIndex].GetAddressOf());

}

//	円形オーディオスペクトラム更新
void Stage::UpdateCircleAudioSpectrum(const float& elapsedTime)
{
	//	座標更新
	int projectionMappingIndex = static_cast<int>(AudioSpectrumType::Circle);
	DirectX::XMFLOAT3 projectionMappingEye				= Player::Instance().GetTransform()->GetPosition();					//	プレイヤーの位置
	DirectX::XMFLOAT3 projectionMappingFocus			= Player::Instance().GetTransform()->GetPosition();	//	注視点
	projectionMapping_[projectionMappingIndex].focus_	= projectionMappingFocus;

	//	視点をプレイヤーの真上から投影するように設定
	projectionMappingEye = projectionMappingEye + projectionMapping_[projectionMappingIndex].eyeOffset_;
	projectionMapping_[projectionMappingIndex].eye_ = projectionMappingEye;

	float projectionMappingFovy = projectionMapping_[projectionMappingIndex].fovy_;
#if 0
	DirectX::XMMATRIX ProjectionMappingTransform =
		DirectX::XMMatrixLookAtLH(
			DirectX::XMLoadFloat3(&projectionMappingEye),
			DirectX::XMLoadFloat3(&projectionMappingFocus),
			DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), DirectX::XMMatrixRotationRollPitchYaw(0, DirectX::XMConvertToRadians(projectionMappingRotation), 0))) *
		DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(projectionMappingFovy), 1.0f, 1.0f, 500.0f);
	DirectX::XMStoreFloat4x4(&projectionMappingConstants_[projectionMappingIndex].transform_, ProjectionMappingTransform);
#else
	//	ビュー行列
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat3(&projectionMappingEye),
		DirectX::XMLoadFloat3(&projectionMappingFocus),
		DirectX::XMVector3Transform(
			DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
			DirectX::XMMatrixRotationRollPitchYaw(0, DirectX::XMConvertToRadians(projectionMapping_[projectionMappingIndex].rotation_), 0))
	);

	//	プロジェクション行列
	DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(projectionMappingFovy), 1.0f, 1.0f, 500.0f
	);

	//	変換行列にスケールを掛ける
	DirectX::XMMATRIX ProjectionMappingTransform = viewMatrix * projMatrix;
	DirectX::XMStoreFloat4x4(&projectionMappingConstants_[projectionMappingIndex].transform_, ProjectionMappingTransform);

#endif

	//	定数バッファをGPUに転送
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(projectionMappingBuffer_[projectionMappingIndex].Get(), 0, 0, &projectionMappingConstants_[projectionMappingIndex], 0, 0);
	static constexpr int SpectrumProjectionMappingCBIndex = 4; //	円形プロジェクションマッピング定数バッファのレジスタ番号
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(SpectrumProjectionMappingCBIndex, 1, projectionMappingBuffer_[projectionMappingIndex].GetAddressOf());

}

//	オーディオスペクトラムの色変更に関する更新処理
void Stage::UpdateSpectrumColor(const float& elapsedTime)
{
	for (int i = 0; i < static_cast<int>(AudioSpectrumType::Max); ++i)
	{
		if (projectionMapping_[i].isTemporaryColorActive_)	//	色変更フラグが立っていたら
		{
			projectionMapping_[i].colorTimer_ += elapsedTime;
			if (projectionMapping_[i].colorTimer_ >= projectionMapping_[i].colorDuration_)	//	一定時間経過したらデフォルト色にリセット
			{
				projectionMapping_[i].currentSpectrumColor_	= projectionMapping_[i].defaultSpectrumColor_;
				fftConstant_.color_[i]	= projectionMapping_[i].defaultSpectrumColor_;
				projectionMapping_[i].isTemporaryColorActive_	= false;
			}
		}
	}
}

//	オーディオスペクトラムの色変更
void Stage::SetSpectrumColor(const AudioSpectrumType& projectionMappingType, const DirectX::XMFLOAT4& color)
{
	fftConstant_.color_[static_cast<int>(projectionMappingType)] = color;
	projectionMapping_[static_cast<int>(projectionMappingType)].isTemporaryColorActive_ = true;
	projectionMapping_[static_cast<int>(projectionMappingType)].colorTimer_ = 0.0f;	//	タイマーをリセット
}

//	オーディオスペクトラムのスケール変更に関する更新処理
//	プロジェクションマッピングの視点を変化させることでスケールが変わったように見せる
void Stage::UpdateSpectrumFovy(const float& elapsedTime)
{
	for (int i = 0; i < static_cast<int>(AudioSpectrumType::Max); ++i)
	{
		if (projectionMapping_[i].isFovyScaleActive_)	//	色変更フラグが立っていたら
		{
			projectionMapping_[i].fovyTimer_ += elapsedTime;
			if (projectionMapping_[i].fovyTimer_ >= projectionMapping_[i].fovyDuration_)	//	一定時間経過したら視点をデフォルト位置にリセット
			{
				projectionMapping_[i].isFovyScaleActive_ = false;
				projectionMapping_[i].fovy_ = projectionMapping_[i].defaultFovy_;
			}
		}
	}
}

//	オーディオスペクトラムの視野角
void Stage::SetCircleSpectrumFovy(const AudioSpectrumType& projectionMappingType, const float& fovy,const float& lerpTime/*イージング用*/)
{
	projectionMapping_[static_cast<int>(projectionMappingType)].isFovyScaleActive_= true;
	projectionMapping_[static_cast<int>(projectionMappingType)].fovyTimer_ = 0.0f;	//	タイマーをリセット
	projectionMapping_[static_cast<int>(projectionMappingType)].fovy_ = fovy;	//	円形オーディオスペクトラムの大きさを変化
	//eyeOffsetY_ += 2.5f;	//	円形オーディオスペクトラムの大きさを変化
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

//	コリジョンメッシュの当たり判定
bool Stage::Collision(_In_ const DirectX::XMFLOAT3& rayStartPosition, _In_ const DirectX::XMFLOAT3& rayDirection, _In_ const DirectX::XMFLOAT4X4& stageTransform, _Out_ DirectX::XMFLOAT3& intersectionPosition, _Out_ DirectX::XMFLOAT3& intersectionNormal,
	_Out_ std::string& intersectionMesh, _Out_ std::string& intersectionMaterial, _In_ const float& rayLengthLimit, _In_ const bool& skipIf) const
{
	//	空間分割レイキャスト
	if (collisionMesh_->RaycastWithSpaceDivision(rayStartPosition, rayDirection, stageTransform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial, rayLengthLimit, skipIf))
	{
#if 0	//	結果を出力画面で確認するため
		OutputDebugStringA("Position:");
		OutputDebugStringA("Intersected : ");
		OutputDebugStringA(intersectionMesh.c_str());
		OutputDebugStringA(" : ");
		OutputDebugStringA(intersectionMaterial.c_str());
		OutputDebugStringA("\n");
#endif
		return true;
	}
	else
	{
		//	結果を出力画面で確認するため
#if 0
		OutputDebugStringA("Unintersected...\n");
#endif
		return false;
	}
}

//	描画処理
void Stage::Render()
{
	//	----- ステート設定 -----
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
	 
	//	----- 円形のオーディオスペクトラム -----
	int spectrumIndex = static_cast<int>(AudioSpectrumType::Circle);
	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
	spectrumFramebuffer_[spectrumIndex]->Clear(deviceContext, 0, 0, 0, 1);
	spectrumFramebuffer_[spectrumIndex]->Activate(deviceContext);
	fullScreenQuad_->Blit(deviceContext, projectionMapping_[static_cast<int>(AudioSpectrumType::Circle)].texture_.GetAddressOf(), 1, 0, spectrumCirclePS_.Get());
	spectrumFramebuffer_[spectrumIndex]->Deactivate(deviceContext);
	Graphics::Instance().GetDeviceContext()->PSSetShaderResources(15, 1, spectrumFramebuffer_[spectrumIndex]->shaderResourceViews_[0].GetAddressOf());
	
	//	----- 波形のオーディオスペクトラム -----
	spectrumIndex = static_cast<int>(AudioSpectrumType::Waveform);
	spectrumFramebuffer_[spectrumIndex]->Clear(deviceContext, 0, 0, 0, 1);
	spectrumFramebuffer_[spectrumIndex]->Activate(deviceContext);
	fullScreenQuad_->Blit(deviceContext, projectionMapping_[static_cast<int>(AudioSpectrumType::Waveform)].texture_.GetAddressOf(), 1, 0, spectrumWaveformPS_.Get());
	spectrumFramebuffer_[spectrumIndex]->Deactivate(deviceContext);
	Graphics::Instance().GetDeviceContext()->PSSetShaderResources(16, 1, spectrumFramebuffer_[spectrumIndex]->shaderResourceViews_[0].GetAddressOf());

	//	----- エミッシブ定数バッファをGPUに転送 -----
	deviceContext->UpdateSubresource(emissiveConstantBuffer_.Get(), 0, 0, &emissiveConstant_, 0, 0);
	deviceContext->PSSetConstantBuffers(3, 1, emissiveConstantBuffer_.GetAddressOf());

	//	----- モデル描画 -----
	gltfStaticModelResource_->Render();

}

//	FFT定数バッファ更新
void Stage::UpdateFFTConstantBuffer(const float& elapsedTime)
{
	//	----- FFT結果を取得 -----
	frequency_->Update(elapsedTime, AudioManager::Instance().GetAudioResource("GameBGM"));
	std::vector<float> fftData = frequency_->GetAmplitudeSpectrum();

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

	//	----- FFT定数バッファ更新 -----
	for (int index = 0; index < Frequency::BlockCount_; ++index)
	{
		fftConstant_.fftData_[index] = fftData.at(index);
	}

	//	----- FFT定数バッファをGPUに転送 -----
	Graphics::Instance().GetDeviceContext()->UpdateSubresource(fftConstantBuffer_.Get(), 0, 0, &fftConstant_, 0, 0);
	static constexpr int FFTCBIndex = 10; //	FFTデータ転送用定数バッファのレジスタ番号
	Graphics::Instance().GetDeviceContext()->PSSetConstantBuffers(FFTCBIndex, 1, fftConstantBuffer_.GetAddressOf());

}

//	デバッグ描画
void Stage::DrawDebug()
{
	if (ImGui::TreeNode(u8"Stage ステージ"))
	{
		GetTransform()->DrawDebug();

		//	コリジョンメッシュ
		collisionMesh_->DrawDebug();

		//	----- 円形のオーディオスペクトラムテクスチャ -----
		ImGui::Text(u8"CircleSpectrumSRV_Slot15");
		{
			D3D11_VIEWPORT viewport;
			UINT numViewports{ 1 };
			Graphics::Instance().GetDeviceContext()->RSGetViewports(&numViewports, &viewport);
			auto srv = spectrumFramebuffer_[static_cast<int>(AudioSpectrumType::Circle)]->shaderResourceViews_[0].Get();
			ImGui::Image(reinterpret_cast<void*>(srv), ImVec2(viewport.Width / 5.0f, viewport.Height / 5.0f));
		}
		//	----- 波形のオーディオスペクトラムテクスチャ -----
		ImGui::Text(u8"WaveFormSpectrumSRV_Slot16");
		{
			D3D11_VIEWPORT viewport;
			UINT numViewports{ 1 };
			Graphics::Instance().GetDeviceContext()->RSGetViewports(&numViewports, &viewport);
			auto srv = spectrumFramebuffer_[static_cast<int>(AudioSpectrumType::Waveform)]->shaderResourceViews_[0].Get();
			ImGui::Image(reinterpret_cast<void*>(srv), ImVec2(viewport.Width / 5.0f, viewport.Height / 5.0f));
		}

		//	----- プロジェクションマッピング -----
		if (ImGui::TreeNode("ProjectionMapping"))
		{
			int projectionMappingIndex = static_cast<int>(AudioSpectrumType::Waveform);
			//	----- 波形オーディオスペクトラム -----
			if (ImGui::TreeNode("Waveform"))
			{
				ImGui::PushID(projectionMappingIndex);
				ImGui::ColorEdit4("Color", &fftConstant_.color_[projectionMappingIndex].x);
				ImGui::DragFloat3("Eye", &projectionMapping_[projectionMappingIndex].eye_.x, 0.01f);
				ImGui::DragFloat3("Focus", &projectionMapping_[projectionMappingIndex].focus_.x, 0.01f);
				ImGui::DragFloat("Rotation", &projectionMapping_[projectionMappingIndex].rotation_, 0.01f);
				ImGui::SliderFloat("Fovy", &projectionMapping_[projectionMappingIndex].fovy_, 10.0f, 180.0f);
				ImGui::PopID();
				ImGui::TreePop();
			}

			//	----- 円形オーディオスペクトラム -----
			if (ImGui::TreeNode("Circle"))
			{
				projectionMappingIndex = static_cast<int>(AudioSpectrumType::Circle);
				ImGui::ColorEdit4("Color", &fftConstant_.color_[projectionMappingIndex].x);
				ImGui::PushID(projectionMappingIndex);
				ImGui::DragFloat3("Eye", &projectionMapping_[projectionMappingIndex].eye_.x, 0.01f);
				ImGui::DragFloat3("Focus", &projectionMapping_[projectionMappingIndex].focus_.x, 0.01f);
				ImGui::DragFloat("Rotation", &projectionMapping_[projectionMappingIndex].rotation_, 0.01f);
				ImGui::SliderFloat("Fovy", &projectionMapping_[projectionMappingIndex].fovy_, 10.0f, 180.0f);
				ImGui::PopID();
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		//	----- 周波数データのデバッグ描画 -----
		if (ImGui::TreeNode("Frequency Data"))
		{
			frequency_->DrawDebug();
			ImGui::Checkbox("UseFrequency", &useFrequency_);
			ImGui::DragInt("FrequencyIndex", &frequencyIndex_, 1.0f, 0, (Frequency::BlockCount_ - 1));
			ImGui::DragFloat("CurrentFrequency", &currentFrequencyValue_, 1.0f, 0.0f);
			ImGui::DragFloat("FrequencyMin", &frequencyMinValue_, 1.0f, 0.0f);
			ImGui::DragFloat("FrequencyMax", &frequencyMaxValue_, 1.0f, 0.0f);
			ImGui::DragFloat("EmissiveIntencity", &emissiveConstant_.emissiveIntensity_, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat("EmissiveThreshold", &threshold_, 0.1f, 0.0f, FLT_MAX);


			ImGui::TreePop();
		}

		//	----- エミッシブ変化用データ -----
		if (ImGui::TreeNode("Emissive Data"))
		{
			ImGui::DragFloat("CurrentEmissiveIntensity", &emissiveData_.currentEmissiveIntensity_, 0.01f);
			ImGui::DragFloat("EmissiveIntensityMax", &emissiveData_.emissiveIntensityMax_, 0.01f);
			ImGui::DragFloat("EmissiveIntensityMin", &emissiveData_.emissiveIntensityMin_, 0.01f);

			ImGui::TreePop();
		}

		//	モデルのデバッグ描画
		gltfStaticModelResource_->DrawDebug();

		ImGui::TreePop();
	}
}
