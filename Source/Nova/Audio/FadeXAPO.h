#pragma once

#pragma comment(lib,"xapobase.lib")

#include <xapobase.h>

//	フェードエフェクトXAPO
class __declspec(uuid("{FD16C679-5181-428C-8180-F42268EDA607}"/*GUID*/))
	FadeXAPO :public CXAPOParametersBase
{
public:
	FadeXAPO();
	~FadeXAPO() {}

	//	LOckForProcess関数をオーバーライド
	STDMETHOD(LockForProcess)(UINT32 InputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pInputLockedParameters, UINT32 OutputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pOutputLockedParameters)override;

	//	Process関数をオーバーライド
	STDMETHOD_(void, Process)(UINT32 InputProcessParameterCount, const XAPO_PROCESS_BUFFER_PARAMETERS* pInputProcessParameters, UINT32 OutputProcessParameterCount, XAPO_PROCESS_BUFFER_PARAMETERS* pOutputProcessParameters, BOOL IsEnabled)override;

	//	SetParameters関数をオーバーライド(Voice->SetEffectParameters()が呼ばれたら呼び出される)
	STDMETHOD_(void, SetParameters)(const void* pParameters, UINT32 ParameterByteSize)override;

	//	GetParameters関数をオーバーライド
	STDMETHOD_(void, GetParameters)(void* pParameters, UINT32 ParameterByteSize)override;

private:
	static XAPO_REGISTRATION_PROPERTIES xapoRegProp_;	//	プロパティ

	WAVEFORMATEX inputFmt_;		//	入力フォーマットの記憶場所
	WAVEFORMATEX outputFmt_;	//	出力フォーマットの記憶場所

private:
	struct FadeParam			//	フェード処理に必要な情報
	{
		float targetVolume_;	//	到達目標ボリューム
		float targetTime_;		//	フェード時間[秒]
	};
	FadeParam FadeParam_[3];	//	XAPO受信ワークエリア

private:
	float currentVolume_ = 1.0f;	//	現在のボリューム
	float slopeVolume_ = 0.0f;		//	ボリューム変化量

};

