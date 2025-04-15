#pragma once
#include <x3daudio.h>

struct Reverb_ex
{
	float WetDryMix;
	int ReflectionsDelay;		//UINT32
	int ReverbDelay;			//UCHAR
	int RearDelay;				//UCHAR
	int SideDelay;				//UCHAR
	int PositionLeft;			//UCHAR
	int PositionRight;			//UCHAR
	int PositionMatrixLeft;		//UCHAR
	int PositionMatrixRight;	//UCHAR
	int EarlyDiffusion;			//UCHAR
	int LateDiffusion;			//UCHAR
	int LowEQGain;				//UCHAR
	int LowEQCutoff;			//UCHAR
	int HighEQGain;				//UCHAR
	int HighEQCutoff;			//UCHAR
	float RoomFilterFreq;
	float RoomFilterMain;
	float RoomFilterHF;
	float ReflectionsGain;
	float ReverbGain;
	float DecayTime;
	float Density;
	float RoomSize;
	int DisableLateField;		//BOOL
};

//	I3DL2形式のプリセット
enum I3DL2_Preset
{
	NONE = 0,
	DEFAULT,
	GENERIC,
	PADDEDCELL,
	ROOM,
	BATHROOM,
	LIVINGROOM,
	STONEROOM,
	AUDITORIUM,
	CONCERTHALL,
	CAVE,
	ARENA,
	HANGAR,
	CARPETEDHALLWAY,
	HALLWAY,
	STONECORRIDOR,
	ALLEY,
	FOREST,
	CITY,
	MOUNTAINS,
	QUARRY,
	PLAIN,
	PARKINGLOT,
	SEWERPIPE,
	UNDERWATER,
	SMALLROOM,
	MEDIUMROOM,
	LARGEROOM,
	MEDIUMHALL,
	LARGEHALL,
	PLATE,
};


static const UINT32 STANDARD_SAMPLINGRATE = 44100;

struct AudioCurve
{
	X3DAUDIO_DISTANCE_CURVE_POINT* volCurvepoint_;
	X3DAUDIO_DISTANCE_CURVE volCurve_;

	X3DAUDIO_DISTANCE_CURVE_POINT* lfeCurvepoint_;
	X3DAUDIO_DISTANCE_CURVE lfeCurve_;

	X3DAUDIO_DISTANCE_CURVE_POINT* lpfDirectcurvepoint_;
	X3DAUDIO_DISTANCE_CURVE lpfDirectcurve_;

	X3DAUDIO_DISTANCE_CURVE_POINT* lpfRevcurvepoint_;
	X3DAUDIO_DISTANCE_CURVE lpfRevcurve_;

	X3DAUDIO_DISTANCE_CURVE_POINT* revCurvepoint_;
	X3DAUDIO_DISTANCE_CURVE revCurve_;
};