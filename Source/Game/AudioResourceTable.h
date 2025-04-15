#pragma once

#include <string>

//	BGMのデータテーブル
struct AudioData
{
	std::string filePath_	= {};		//	リソースのファイルパス
	int			bpm_		= 120;		//	BPM
	float		volume_		= 1.0f;		//	再生時のボリューム
};

static const int BGMCount = 3;		//	BGMの数

enum class BGMLabel
{
	Normal,		//	普通　			120
	Up,			//	アップテンポ	
	Low,		//	ローテンポ		
	Max,
};

AudioData AudioResourceTable[static_cast<int>(BGMLabel::Max)]
{
	{},
	{},
	{}
};