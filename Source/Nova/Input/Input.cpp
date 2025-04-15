#include "../Input/Input.h"

#include <windows.h>

#include "../../imgui/imgui.h"

Input* Input::instance_ = nullptr;

// コンストラクタ
Input::Input(HWND hWnd)
	: mouse_(hWnd)
{
	instance_ = this;
}

// 更新処理
void Input::Update(const float& elapsedTime)
{
	gamePad_.Update(elapsedTime);
	mouse_.Update();
	UpdateKeyData(elapsedTime);
}

//	入力キー設定
void Input::SetInputKey(const InputKey& key)
{
	//	古い入力キーを一つずつ隣にずらす
	for (int i = MaxInputKey - 1; i > 0; i--)
	{
		inputKeys_[i] = inputKeys_[i - 1];
	}
	//	新しい入力キーを設定
	inputKeys_[0].key_ = key;
	inputKeys_[0].frame_ = 0;
}

//	猶予フレーム内にコマンドが押されているか
#if 1
bool Input::CommandConfirm(const Command& command, const float& frame)
{
	int count = 0;
	//	command配列の後ろからイテレータで入力コマンドと比較
	for (Command::const_reverse_iterator it = command.rbegin(); it != command.rend(); it++)
	{
		InputKey key = *it;
		//	現在のイテレータがさしてるkeyを検索して
		//	見つかるまでループして、inputKeys_[count].key_にkeyのフラグが立っていない場合カウントを増やす
		while (count < MaxInputKey && (inputKeys_[count].key_ & key) != key)
		{
			count++;
		}
		//	時間での判断、カウントが判定フレーム以内じゃなかったらfalse
		//if (inputKeys_[count].frame_ >= frame || count == MaxInputKey)
		if (count >= frame || count == MaxInputKey)
		{
			return false;
		}
		//	カウントを増やして次のcommand配列のキーと比較
		count++;
	}

	return true;
}
#else
bool Input::CommandConfirm(const Command& command, const float& frame)
{
	int count = 0;
	std::vector<int> currectCommandIndex;	//	入力が正しければその入力の要素番号を保存
	//	command配列の後ろからイテレータで入力コマンドと比較
	for (Command::const_reverse_iterator it = command.rbegin(); it != command.rend(); it++)
	{
		InputKey key = *it;
		//	現在のイテレータがさしてるkeyを検索して
		//	見つかるまでループしてカウントを増やす
		while (count < MaxInputKey && (inputKeys_[count].key_ & key) != false)
		{
			count++;
		}
		//	時間での判断、カウントが判定フレーム以内じゃなかったらfalse
		if (inputKeys_[count].frame_ >= frame || count == MaxInputKey)
		{
			return false;
		}
		else
		{
			currectCommandIndex.emplace_back(count);	//	正しい入力の要素番号を保存
		}
		//	カウントを増やして次のcommand配列のキーと比較
		count++;
	}

	//	コマンドが揃っていたらその入力を0にしてtrueを返す
	for (const int& index : currectCommandIndex)
	{
		inputKeys_[index].key_ = 0;
		inputKeys_[index].frame_ = 0;
	}
	return true;
}
#endif

//	入力情報更新処理
void Input::UpdateKeyData(const float& elapsedTime)
{
	//	キー入力状態を取得
	bool up		= (gamePad_.GetButton()	& GamePad::BTN_UP) != 0;
	bool right	= (gamePad_.GetButton()	& GamePad::BTN_RIGHT) != 0;
	bool down	= (gamePad_.GetButton()	& GamePad::BTN_DOWN) != 0;
	bool left	= (gamePad_.GetButton()	& GamePad::BTN_LEFT) != 0;
	bool punch	= (gamePad_.GetButton()	& GamePad::BTN_Y) != 0;
	bool kick	= (gamePad_.GetButton()	& GamePad::BTN_B) != 0;
	//	スティック入力値をキー入力に割り当て
	if (gamePad_.GetAxisLX() < -0.5f)	left = true;
	if (gamePad_.GetAxisLX() > 0.5f)	right = true;
	if (gamePad_.GetAxisLY() < -0.5f)	down = true;
	if (gamePad_.GetAxisLY() > 0.5f)	up	= true;

	//	キー入力設定
	InputKey key = 0;
	key |= (down && left) ? Key1 : Not1;
	key |= (down) ? Key2 : Not2;
	key |= (down && right) ? Key3 : Not3;
	key |= (left) ? Key4 : Not4;
	key |= (!up && !down && !left && !right) ? Key5 : Not5;
	key |= (right) ? Key6 : Not6;
	key |= (up && left) ? Key7 : Not7;
	key |= (up) ? Key8 : Not8;
	key |= (up && right) ? Key9 : Not9;
	key |= (punch) ? KeyP : NotP;
	key |= (kick) ? KeyK : NotK;

	//	斜めキー入力がある場合は重複する上下左右キーの入力を外す
	if (key & Key1)key &= ~(Key2 | Key4);
	if (key & Key3)key &= ~(Key2 | Key6);
	if (key & Key7)key &= ~(Key8 | Key4);
	if (key & Key9)key &= ~(Key8 | Key6);

	SetInputKey(key);

	//	スタックされた入力情報の経過時間を更新
	for (int i = 0; i < MaxInputKey; ++i)
	{
		inputKeys_[i].frame_++;
		//inputKeys_[i].frame_+=elapsedTime;
		if (inputKeys_[i].frame_ > SaveFrame)	//	入力保持時間を超えたら無効にする
		{
			inputKeys_[i].frame_ = 0;
			inputKeys_[i].key_ = 0;
		}
	}
#if 1
	//デバッグで入力キーをログ出力する
	if (key & Key1)OutputDebugStringA("1");
	if (key & Key2)OutputDebugStringA("2");
	if (key & Key3)OutputDebugStringA("3");
	if (key & Key4)OutputDebugStringA("4");
	if (key & Key5)OutputDebugStringA("5");
	if (key & Key6)OutputDebugStringA("6");
	if (key & Key7)OutputDebugStringA("7");
	if (key & Key8)OutputDebugStringA("8");
	if (key & Key9)OutputDebugStringA("9");
	if (key & KeyP)OutputDebugStringA("P");
	if (key & KeyK)OutputDebugStringA("K");
	OutputDebugStringA("\n");
#endif

}

//	入力情報リセット
void Input::ResetInputData()
{
	for (int i = 0; i < MaxInputKey; ++i)
	{
		inputKeys_[i].frame_ = 0;
		inputKeys_[i].key_ = 0;
	}
}

//	デバッグ描画
void Input::DrawDebug()
{
	if (ImGui::TreeNode("Input"))
	{
		//ImGui::DragFloat("InputKey_Frame", &inputKeys_[0].frame_);
		ImGui::DragInt("StartInputKey_Frame", &inputKeys_[0].frame_);
		ImGui::DragInt("EndInputKey_Frame", &inputKeys_[MaxInputKey - 1].frame_);
		ImGui::TreePop();
	}
}
