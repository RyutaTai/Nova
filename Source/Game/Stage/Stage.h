#pragma once

#include <memory>

#include "../../Nova/Resources/GltfModelStaticBatching.h"
#include "../../Nova/Collision/CollisionMesh.h"
#include "../../Nova/Audio/Frequency.h"
#include "../../Nova/Graphics/FrameBuffer.h"
#include "../../Nova/Graphics/FullScreenQuad.h"

//	ステージクラス
class Stage
{
public:
	//	----- オーディオスペクトラムの種類 -----
	enum class AudioSpectrumType
	{
		Waveform,	//	波形
		Circle,		//	円形	
		Max
	};

public:
	Stage();
	~Stage() = default;

	static Stage& Instance();

	void Update(const float& elapsedTime);
	void Render();
	void DrawDebug();

	void UpdateFFTConstantBuffer(const float& elapsedTime);					//	FFT定数バッファ更新処理
	void UpdateEmissive(const float& elapsedTime);	//	エミッシブ更新処理
	void UpdateFrequencyMin();
	void UpdateFrequencyMax();

	bool Collision(_In_ const DirectX::XMFLOAT3& rayPosition, _In_ const DirectX::XMFLOAT3& rayDirection, _In_ const DirectX::XMFLOAT4X4& stageTransform, _Out_ DirectX::XMFLOAT3& intersectionPosition, _Out_ DirectX::XMFLOAT3& intersectionNormal,
		_Out_ std::string& intersectionMesh, _Out_ std::string& intersectionMaterial, _In_ const float& rayLengthLimit = 1.0e+7f, _In_ const bool& skipIf = false/*Once the first intersection is found, the process is interrupted.*/) const;

	Transform* GetTransform() { return gltfStaticModelResource_->GetTransform(); }
	Frequency* GetFrequency() { return frequency_.get(); }	//	音の周波数データ取得

	//	オーディオスペクトラム
	void UpdateAudioSpectrum(const float& elapsedTime);			//	オーディオスペクトラム更新
	void UpdateCircleAudioSpectrum(const float& elapsedTime);	//	円形オーディオスペクトラム更新
	void UpdateWaveformAudioSpectrum();							//	波形オーディオスペクトラム更新

	//	オーディオスペクトラムの色
	void UpdateSpectrumColor(const float& elapsedTime);
	void SetSpectrumColor(const AudioSpectrumType& projectionMappingType, const DirectX::XMFLOAT4& color);

	//	オーディオスペクトラムを投影する視野角
	void UpdateSpectrumFovy(const float& elapsedTime);
	void SetCircleSpectrumFovy(const AudioSpectrumType& projectionMappingType, const float& fovy, const float& lerpTime);

	//	プロジェクションマッピング情報
	void SetProjectionMappingEye(const DirectX::XMFLOAT3& eye, const int& index) { projectionMapping_[index].eye_ = eye; }
	void SetProjectionMappingFocus(const DirectX::XMFLOAT3& focus, const int& index) { projectionMapping_[index].focus_ = focus; }
	void SetProjectionMappingRotation(const float& rotation, const int& index) { projectionMapping_[index].rotation_ = rotation; }
	void SetProjectionMappingFovy(const float& fovy, const int& index) { projectionMapping_[index].fovy_ = fovy; }
	void SetProjectionMappingTransform(const DirectX::XMMATRIX& projectionMappingTransform, const int& index) { DirectX::XMStoreFloat4x4(&projectionMappingConstants_[index].transform_, projectionMappingTransform); }
	void SetProjectionMappingTransform(const DirectX::XMFLOAT4X4& projectionMappingTransform, const int& index) { projectionMappingConstants_[index].transform_, projectionMappingTransform; }
	const DirectX::XMFLOAT3		GetProjectionMappingEye(const int& index)	const { return projectionMapping_[index].eye_; }
	const DirectX::XMFLOAT3		GetProjectionMappingFocus(const int& index) const { return projectionMapping_[index].focus_; }
	const float					GetProjectionMappingRotation(const int& index) const { return projectionMapping_[index].rotation_; }
	const float					GetProjectionMappingFovy(const int& index)		const { return projectionMapping_[index].fovy_; }
	const DirectX::XMFLOAT4X4	GetProjectionMappingTransform(const int& index)	const { return projectionMappingConstants_[index].transform_; }

	//	シャドウマップ
	void CastShadows() { gltfStaticModelResource_->CastShadows(); }

	//	エミッシブ変化用
	void SetIsFadeIn(const bool& isFadeIn) { emissiveIsFadeIn_ = isFadeIn; }
	void SetLerpFlag(const bool& isLerp) { emissiveIsLerp_ = isLerp; }

private:
	//	----- エミッシブ変化用 -----
	//	エミッシブ変化用定数バッファ
	struct EmissiveConstant
	{
		float emissiveIntensity_;
		float dummy_[3];
	};
	EmissiveConstant emissiveConstant_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> emissiveConstantBuffer_;

	//	エミッシブ変化用データ(CPU側)
	struct EmissiveData
	{
		float	currentEmissiveIntensity_	= 3.0f;		//	現在のエミッシブ値
		float	emissiveIntensityMin_		= 0.1f;		//	エミッシブ最小値
		float	emissiveIntensityMax_		= 3.0f;		//	エミッシブ最大値
	};
	EmissiveData emissiveData_;

	//	----- FFTデータ -----
	struct FFTConstant
	{
		float				fftData_[Frequency::BlockCount_];					//	FFTのデータを分割数分GPUに渡す
		DirectX::XMFLOAT4	color_[static_cast<int>(AudioSpectrumType::Max)];	//	オーディオスペクトラムの数だけcolorを設定
	};
	FFTConstant fftConstant_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> fftConstantBuffer_;

	//	----- プロジェクションマッピング用定数バッファ -----
	struct ProjectionMappingConstant
	{
		DirectX::XMFLOAT4X4 transform_ = {};
	};
	ProjectionMappingConstant			 projectionMappingConstants_[static_cast<int>(AudioSpectrumType::Max)];
	Microsoft::WRL::ComPtr<ID3D11Buffer> projectionMappingBuffer_[static_cast<int>(AudioSpectrumType::Max)];

	//	----- プロジェクションマッピングに関するデータ(CPU側でのみ使用) -----
	struct ProjectionMapping
	{
		//	----- 定数バッファの transform_ の計算に利用
		DirectX::XMFLOAT3	eye_		= { 0.0f, 50.0f, 0.0f };
		DirectX::XMFLOAT3   eyeOffset_	= { 0.0f,  0.0f, 0.0f };
		DirectX::XMFLOAT3	defaultEye_ = { 0.0f, 50.0f, 0.0f };
		DirectX::XMFLOAT3	focus_		= { 0.0f,  0.0f, 0.0f };
		float				rotation_	= 0.0f;
		float				fovy_		= 10.0f;

		//	----- テクスチャ -----
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;	//	ピクセルシェーダーでここに書き出す

		//	----- 色の変更に使用 -----
		DirectX::XMFLOAT4 defaultSpectrumColor_ = { 1.0f, 0.0f, 0.0f, 1.0f };	//	デフォルトの色
		DirectX::XMFLOAT4 currentSpectrumColor_ = { 1.0f, 0.0f, 0.0f, 1.0f };	//	現在の色
		bool	isTemporaryColorActive_ = false;	//	オーディオスペクトラムの一時的な色変更フラグ
		float	colorTimer_				= 0.0f;		//	変化して何秒経過したか
		float	colorDuration_			= 0.8f;		//	何秒間色を変更するか

		// ----- 視野角変更に使用(スケールが変わったように見せる) -----
		bool	isFovyScaleActive_	= false;
		float	defaultFovy_		= 10.0f;
		float	fovyTimer_			= 0.0f;
		float	fovyDuration_		= 0.8f;

	};
	ProjectionMapping projectionMapping_[static_cast<int>(AudioSpectrumType::Max)];

private:
	static Stage* instance_;

	//	----- モデル -----
	std::shared_ptr<GltfModelStaticBatching>	gltfStaticModelResource_;
	std::unique_ptr<CollisionMesh>				collisionMesh_;

	//	----- プロジェクションマッピング -----
	std::unique_ptr<FullScreenQuad>				fullScreenQuad_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	spectrumWaveformPS_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	spectrumCirclePS_;
	std::unique_ptr<FrameBuffer>				spectrumFramebuffer_[static_cast<int>(AudioSpectrumType::Max)];

	const int SPECTRUM_WIDTH = 512;
	const int SPECTRUM_HEIGHT = 512;

	//	----- エミッシブ値補間用変数 -----
	float	emissiveLerpTimerMax_ = 0.17f;
	float	emissiveLerpTimer_ = 0.0f;
	bool	emissiveIsFadeIn_ = false;
	bool	emissiveIsLerp_ = false;

	bool				useFrequency_ = true;
	static const int	FrequencyDataMax = 120;
	float				frequencyData_[FrequencyDataMax];
	int					frequencyIndex_ = 25;
	float				currentFrequencyValue_ = 0.0f;
	float				frequencyMinValue_ = 0.0f;
	float				frequencyMaxValue_ = 0.0f;

	std::unique_ptr<Frequency> frequency_;	//	音の周波数データ(emissiveIntencityの計算に使う)

	float threshold_ = 2100.0f;

};