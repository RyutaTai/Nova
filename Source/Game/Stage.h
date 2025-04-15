#pragma once

#include <memory>

#include "../Nova/Resources/GltfModelStaticBatching.h"
#include "../Nova/Collision/CollisionMesh.h"
#include "../Nova/Audio/Frequency.h"
#include "../Nova/Graphics/FrameBuffer.h"
#include "../Nova/Graphics/FullScreenQuad.h"

#define MAGIC_CIRCLE 0
#define SPECTRUM_CIRCLE 1

class Stage
{
public:
	Stage();
	~Stage() {}

	static Stage& Instance();

	void ShadowRender(const float& scale = 1.0f);
	void Update(const float& elapsedTime);
	void Render();
	void DrawDebug();

	void UpdateFFTConstantBuffer();					//	FFT定数バッファ更新処理
	void UpdateEmissive(const float& elapsedTime);	//	エミッシブ更新処理
	void UpdateFrequencyMin();
	void UpdateFrequencyMax();
	float CalculateAutocorrelation(const float data[], const int& lag);

	void UpdateAudioSpectrum(const float& elapsedTime);			//	オーディオスペクトラム更新
	void UpdateCircleAudioSpectrum(const float& elapsedTime);	//	円形オーディオスペクトラム更新
	void UpdateWaveformAudioSpectrum();	//	波形オーディオスペクトラム更新

	bool Collision(_In_ const DirectX::XMFLOAT3& rayPosition, _In_ const DirectX::XMFLOAT3& rayDirection, _In_ const DirectX::XMFLOAT4X4& stageTransform, _Out_ DirectX::XMFLOAT3& intersectionPosition, _Out_ DirectX::XMFLOAT3& intersectionNormal,
		_Out_ std::string& intersectionMesh, _Out_ std::string& intersectionMaterial, _In_ float rayLengthLimit = 1.0e+7f, _In_ bool skipIf = false/*Once the first intersection is found, the process is interrupted.*/) const;

	Transform* GetTransform() { return gltfStaticModelResource_->GetTransform(); }
	Frequency* GetFrequency() { return frequency_.get(); }	//	音の周波数データ取得

	void SetProjectionMappingEye(const DirectX::XMFLOAT3& eye, const int& index) { projectionMapping_[index].eye_ = eye; }
	void SetProjectionMappingFocus(const DirectX::XMFLOAT3& focus, const int& index) { projectionMapping_[index].focus_ = focus; }
	void SetProjectionMappingRotation(const float& rotation,const int& index) { projectionMapping_[index].rotation_= rotation; }
	void SetProjectionMappingFovy(const float& fovy, const int& index) { projectionMapping_[index].fovy_ = fovy; }
	void SetProjectionMappingTransform(const DirectX::XMMATRIX& projectionMappingTransform, const int& index) { DirectX::XMStoreFloat4x4(&projectionMappingConstants_[index].transform_, projectionMappingTransform); }
	void SetProjectionMappingTransform(const DirectX::XMFLOAT4X4& projectionMappingTransform, const int& index) { projectionMappingConstants_[index].transform_, projectionMappingTransform; }
	const DirectX::XMFLOAT3		GetProjectionMappingEye(const int& index)	const { return projectionMapping_[index].eye_; }
	const DirectX::XMFLOAT3		GetProjectionMappingFocus(const int& index) const { return projectionMapping_[index].focus_; }
	const float					GetProjectionMappingRotation(const int& index) { return projectionMapping_[index].rotation_; }
	const float					GetProjectionMappingFovy(const int& index)	const { return projectionMapping_[index].fovy_; }
	const DirectX::XMFLOAT4X4	GetProjectionMappingTransform(const int& index)	const { return projectionMappingConstants_[index].transform_; }

private:
	enum class CollisionModel
	{
		Ground,
		Building,
		Max,
	};

	enum class ProjectionMappingType
	{
		Waveform,	//	波形
		Circle,		//	円形	
		Max
	};

	//	エミッシブの強度
	struct EmissiveConstant
	{
		float emissiveIntensity_;
		float dummy_[3];
	};
	EmissiveConstant emissiveConstant_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> emissiveConstantBuffer_;

	//	FFTデータ
	static constexpr int KindOfColor = 2;
	struct FFTConstant
	{
		float fftData_[Frequency::BlockCount];	//	FFTのデータを分割数分GPUに渡す
		DirectX::XMFLOAT4 color_[KindOfColor];	//	オーディオスペクトラムの数だけcolorを設定
	};
	FFTConstant fftConstant_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> fftConstantBuffer_;

	//	プロジェクションマッピング
	struct ProjectionMappingConstant
	{
		DirectX::XMFLOAT4X4 transform_ = {};
	};
	ProjectionMappingConstant			 projectionMappingConstants_[static_cast<int>(ProjectionMappingType::Max)];
	Microsoft::WRL::ComPtr<ID3D11Buffer> projectionMappingBuffer_[static_cast<int>(ProjectionMappingType::Max)];

private:
	static Stage* instance_;

	std::shared_ptr<GltfModelStaticBatching>	gltfStaticModelResource_;		//	Gltfモデル
	std::unique_ptr<CollisionMesh>				collisionMesh_;

	//	----- プロジェクションマッピング -----
	std::unique_ptr<FullScreenQuad>				bitBlockTransfer_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	spectrumWaveformPS_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	spectrumCirclePS_;
	std::unique_ptr<FrameBuffer>				spectrumFramebuffer_[static_cast<int>(ProjectionMappingType::Max)];
#if MAGIC_CIRCLE
	const int SPECTRUM_WIDTH = 512;
	const int SPECTRUM_HEIGHT = 512;
#else
	const int SPECTRUM_WIDTH = 256;
	const int SPECTRUM_HEIGHT = 256;
#endif
	struct ProjectionMapping
	{
		DirectX::XMFLOAT3	eye_		= { 0, 50, 0 };
		DirectX::XMFLOAT3	focus_		= { 0, 0, 0 };
		float				rotation_	= 0.0f;
		float				fovy_		= 10.0f;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;	//	ピクセルシェーダーでここに書き出す
	};
	ProjectionMapping projectionMapping_[static_cast<int>(ProjectionMappingType::Max)];
	float fftDivisionValue_ = 10000.0f;	//	GPUに渡すFFTデータを割る値

	bool				useFrequency_			= true;
	static const int	FrequencyDataMax		= 120;
	float				frequencyData_[FrequencyDataMax];
	int					frequencyIndex_			= 265;
	//int					frequencyIndex_			= 509;
	float				currentFrequencyValue_	= 0.0f;
	float				frequencyMinValue_		= 0.0f;
	float				frequencyMaxValue_		= 0.0f;
	float				emissiveFactor_			= 15.0f;
	float				emissiveIntencityMin_	= 0.1f;
	float				emissiveIntencityMax_	= 15.0f;

	std::unique_ptr<Frequency> frequency_ = nullptr;	//	音の周波数データ(emissiveIntencityの計算に使う)

	float threshold_ = 2100.0f;
	float defaultEmissiveIntensity_ = 10.0f;
	float eyeHeight_ = 30.0f;	//	focusからeyeまでの高さ
};