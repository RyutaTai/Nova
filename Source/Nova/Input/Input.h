#pragma once

#include <vector>

#include "../Input/GamePad.h"
#include "../Input/Mouse.h"

using InputKey = uint32_t;
//	入力キー(押している状態)
static const InputKey Key1 = (1 << 0);		//	左下
static const InputKey Key2 = (1 << 1);		//	下
static const InputKey Key3 = (1 << 2);		//	右下
static const InputKey Key4 = (1 << 3);		//	左
static const InputKey Key5 = (1 << 4);		//	中
static const InputKey Key6 = (1 << 5);		//	右
static const InputKey Key7 = (1 << 6);		//	左上
static const InputKey Key8 = (1 << 7);		//	上
static const InputKey Key9 = (1 << 8);		//	右上
static const InputKey KeyP = (1 << 9);		//	パンチ
static const InputKey KeyK = (1 << 10);		//	キック
//	入力キー(押していない状態)
static const InputKey Not1 = (1 << 11);		//	左下
static const InputKey Not2 = (1 << 12);		//	下
static const InputKey Not3 = (1 << 13);		//	右下
static const InputKey Not4 = (1 << 14);		//	左
static const InputKey Not5 = (1 << 15);		//	中
static const InputKey Not6 = (1 << 16);		//	右
static const InputKey Not7 = (1 << 17);		//	左上
static const InputKey Not8 = (1 << 18);		//	上
static const InputKey Not9 = (1 << 19);		//	右上
static const InputKey NotP = (1 << 20);		//	パンチ
static const InputKey NotK = (1 << 21);		//	キック

//	最大キー入力数
static const int MaxInputKey = 256;
//	入力を保持する時間(SaveFrame分のフレーム数保持)
static const int SaveFrame = 256;

//	コマンド
using Command = std::vector<InputKey>;

struct InputData
{
	InputKey	key_	= 0;		//	入力されたキー情報
	//float		frame_	= 0;		//	入力されて何フレーム経過したか
	int			frame_	= 0;		//	入力されて何フレーム経過したか
};

//	インプット
class Input
{
public:
	Input(HWND hWnd);
	~Input() {}

public:
	static Input& Instance() { return *instance_; }	//	インスタンス取得

	void Update(const float& elapsedTime);			//	更新処理
	void UpdateKeyData(const float& elapsedTime);	//	入力情報更新処理
	void DrawDebug();								//	デバッグ描画

	void SetInputKey(const InputKey& key);			//	入力キー設定
	void ResetInputData();							//	入力情報リセット
	bool CommandConfirm(const Command& command, const float& frame);	//	猶予フレーム内にコマンドが入力されているか

	GamePad& GetGamePad() { return gamePad_; }		//	ゲームパッド取得
	Mouse& GetMouse() { return mouse_; }			//	マウス取得

private:
	static Input*		instance_;
	GamePad				gamePad_;
	Mouse				mouse_;
	InputData			inputKeys_[MaxInputKey];	//	入力したキー情報をMaxInputKeyの数だけ保持
};
