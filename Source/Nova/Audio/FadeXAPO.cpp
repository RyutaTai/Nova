#include "FadeXAPO.h"

//	XAPOプロパティの定義
XAPO_REGISTRATION_PROPERTIES FadeXAPO::xapoRegProp_ =
{
	__uuidof(FadeXAPO),		//	クラスに設定されているGUIDを渡す
	L"FadeXAPO",			//	フレンドリ名(このクラスの名前)
	L"(c) Ryuta Tai",		//	著作権情報(著作者名・会社名など)
	1,0,					//	メジャーバージョン番号:1、マイナーバージョン番号:0
	XAPOBASE_DEFAULT_FLAG,	//	動作指定フラグ(このクラスがどのようなデータを受け付けられるかの動作指定)
	1,1,1,1
};

//	コンストラクタ
FadeXAPO::FadeXAPO() :CXAPOParametersBase(&xapoRegProp_, (BYTE*)FadeParam_, sizeof(FadeParam), FALSE/*受信可能*/)
{

}

//	LockForProcess関数(事前処理をここに書き、Process関数の処理を少しでも軽くする)
HRESULT FadeXAPO::LockForProcess(
	UINT32 InputLockedParameterCount,const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pInputLockedParameters, 
	UINT32 OutputLockedParameterCount,	const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pOutputLockedParameters)
{
	inputFmt_ = *pInputLockedParameters[0].pFormat;
	outputFmt_ = *pOutputLockedParameters[0].pFormat;

	return CXAPOBase::LockForProcess(InputLockedParameterCount, pInputLockedParameters, OutputLockedParameterCount, pOutputLockedParameters);
}

//	Process関数
void FadeXAPO::Process(UINT32 InputProcessParameterCount, const XAPO_PROCESS_BUFFER_PARAMETERS* pInputProcessParameters, 
	UINT32 OutputProcessParameterCount, XAPO_PROCESS_BUFFER_PARAMETERS* pOutputProcessParameters, BOOL IsEnabled)
{
	//const XAPO_PROCESS_BUFFER_PARAMETERS& inputParam = pInputProcessParameters[0];
	//XAPO_PROCESS_BUFFER_PARAMETERS& outputParam = pOutputProcessParameters[0];
	//
	//memcpy(outputParam.pBuffer, inputParam.pBuffer, outputFmt_.nBlockAlign * inputParam.ValidFrameCount);
	//
	//outputParam.ValidFrameCount = inputParam.ValidFrameCount;
	//outputParam.BufferFlags = inputParam.BufferFlags;

	float* in = (float*)pInputProcessParameters[0].pBuffer;
	float* out = (float*)pOutputProcessParameters[0].pBuffer;
	FadeParam* param = (FadeParam*)BeginProcess();
	for (int i = 0; i < pInputProcessParameters[0].ValidFrameCount; i++)
	{
		float volume = *in;

		//	現在のボリュームを乗算した後、volumeにはslope分足す
		volume *= currentVolume_;
		currentVolume_ += slopeVolume_;

		//	目標まで到達したら終了(浮動小数点の誤差のため正確にやる)
		if (slopeVolume_ < 0.0f)
		{
			if (currentVolume_ <= param->targetVolume_)
			{
				currentVolume_ = param->targetVolume_;
				slopeVolume_ = 0.0f;
			}
		}
		else if (slopeVolume_ > 0.0f)
		{
			if (currentVolume_ >= param->targetVolume_)
			{
				currentVolume_ = param->targetVolume_;
				slopeVolume_ = 0.0f;
			}
		}
		*out = volume;
		in++;
		out++;
	}
	EndProcess();
}

//	SetParameters関数(渡されたパラメータの正当性をチェックする。OnSetParameters()とは違うので注意。)
void FadeXAPO::SetParameters(const void* pParameters, UINT32 ParameterByteSize)
{
	//if (ParameterByteSize == sizeof(float))	//	サイズが同じならOK
	//{
	//	CXAPOParametersBase::SetParameters(pParameters, ParameterByteSize);
	//}

	FadeParam* param = (FadeParam*)pParameters;

	if (param->targetTime_ == 0)	//	0なら即実行
	{
		currentVolume_ = param->targetVolume_;
		slopeVolume_ = 0.0f;
	}
	else
	{
		slopeVolume_ = param->targetVolume_ - currentVolume_;
		slopeVolume_ /= inputFmt_.nSamplesPerSec * param->targetTime_;
	}
	CXAPOParametersBase::SetParameters(pParameters, ParameterByteSize);

}

//	GetParameters関数
void FadeXAPO::GetParameters(void* pParameters, UINT32 ParameterByteSize)
{
	*(float*)pParameters = 1.0f;	//	今は1.0fを返している
}