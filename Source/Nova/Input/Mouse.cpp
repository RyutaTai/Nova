#include "../Input/Mouse.h"

static const int KeyMap[] =
{
	VK_LBUTTON,		// 左ボタン
	VK_MBUTTON,		// 中ボタン
	VK_RBUTTON,		// 右ボタン
};

// コンストラクタ
Mouse::Mouse(HWND hWnd_)
	: hWnd_(hWnd_)
{
	RECT rc;
	GetClientRect(hWnd_, &rc);
	screenWidth_ = rc.right - rc.left;
	screenHeight_ = rc.bottom - rc.top;
}

// 更新
void Mouse::Update()
{
	// スイッチ情報
	MouseButton newButtonState = 0;

	for (int i = 0; i < ARRAYSIZE(KeyMap); ++i)
	{
		if (::GetAsyncKeyState(KeyMap[i]) & 0x8000)
		{
			newButtonState |= (1 << i);
		}
	}

	// ホイール
	wheel_[1] = wheel_[0];
	wheel_[0] = 0;

	// ボタン情報更新
	buttonState_[1] = buttonState_[0];	// スイッチ履歴
	buttonState_[0] = newButtonState;

	buttonDown_ = ~buttonState_[1] & newButtonState;	// 押した瞬間
	buttonUp_ = ~newButtonState & buttonState_[1];	// 離した瞬間

	// カーソル位置の取得
	POINT cursor;
	::GetCursorPos(&cursor);
	::ScreenToClient(hWnd_, &cursor);

	// 画面のサイズを取得する。
	RECT rc;
	GetClientRect(hWnd_, &rc);
	UINT screenW = rc.right - rc.left;
	UINT screenH = rc.bottom - rc.top;
	UINT viewportW = screenWidth_;
	UINT viewportH = screenHeight_;

	// 画面補正
	positionX_[1] = positionX_[0];
	positionY_[1] = positionY_[0];
	positionX_[0] = (LONG)(cursor.x / static_cast<float>(viewportW) * static_cast<float>(screenW));
	positionY_[0] = (LONG)(cursor.y / static_cast<float>(viewportH) * static_cast<float>(screenH));
}
