#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

typedef float FLOAT32;

#define SPEED_OF_SOUND 340.0f

//	リスナー情報
struct SoundListener
{
	DirectX::XMFLOAT3	position_ = {};		//	位置
	DirectX::XMFLOAT3	velocity_ = {};		//	リスナーの速度
	DirectX::XMFLOAT3	frontVec_ = {};		//	前方向
	DirectX::XMFLOAT3	rightVec_ = {};		//	右方向
	FLOAT32				innerRadius_ = {};	//	内半径
	FLOAT32				outerRadius_ = {};	//	外半径
	FLOAT32				filterParam_ = {};	//	ローパスに適応する値
};

//	エミッター情報
struct SoundEmitter
{
	DirectX::XMFLOAT3	position_ = {};		//	位置
	DirectX::XMFLOAT3	velocity_ = {};		//	エミッターの速度
	FLOAT32				minDistance_ = {};	//	距離減衰が始まる距離
	FLOAT32				maxDistance_ = {};	//	音が聞こえる最大距離
	float				volume_ = 1.0f;		//	ボリューム
};

//	
struct SoundDSPSetting
{
	UINT32   srcChannelCount_ = 0;				//	音源のチャンネル数
	UINT32   dstChannelCount_ = 0;				//	アウトプットチャンネル数
	FLOAT32  distanceListnerToEmitter_ = {};	//	リスナーとエミッターの距離
	FLOAT32  dopplerScale_ = {};				//	ドップラー効果
	FLOAT32  radianListenerToEmitter_ = {};		//	リスナーからエミッターまでの角度
	FLOAT32* outputMatrix_ = nullptr;				//	SetOutputMatrix()に渡す
	FLOAT32  filterParam_ = {};					//	ローパスに適用する値
};


void DSP(SoundDSPSetting& dspSetting, const SoundListener& listener, const SoundEmitter& emitter);