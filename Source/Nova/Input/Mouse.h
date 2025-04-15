#pragma once

#include <Windows.h>

using MouseButton = unsigned int;

// マウス
class Mouse
{
public:
	static const MouseButton BTN_LEFT = (1 << 0);
	static const MouseButton BTN_MIDDLE = (1 << 1);
	static const MouseButton BTN_RIGHT = (1 << 2);

public:
	Mouse(HWND hWnd_);
	~Mouse() {}

	// 更新
	void Update();

	// ボタン入力状態の取得
	MouseButton GetButton() const { return buttonState_[0]; }

	// ボタン押下状態の取得
	MouseButton GetButtonDown() const { return buttonDown_; }

	// ボタン押上状態の取得
	MouseButton GetButtonUp() const { return buttonUp_; }

	// ホイール値の設定
	void SetWheel(int wheel_) { this->wheel_[0] += wheel_; }

	// ホイール値の取得
	int GetWheel() const { return wheel_[1]; }

	// マウスカーソルX座標取得
	int GetPositionX() const { return positionX_[0]; }

	// マウスカーソルY座標取得
	int GetPositionY() const { return positionY_[0]; }

	// 前回のマウスカーソルX座標取得
	int GetOldPositionX() const { return positionX_[1]; }

	// 前回のマウスカーソルY座標取得
	int GetOldPositionY() const { return positionY_[1]; }

	// スクリーン幅設定
	void SetScreenWidth(int width) { screenWidth_ = width; }

	// スクリーン高さ設定
	void SetScreenHeight(int height) { screenHeight_ = height; }

	// スクリーン幅取得
	int GetScreenWidth() const { return screenWidth_; }

	// スクリーン高さ取得
	int GetScreenHeight() const { return screenHeight_; }

private:
	MouseButton		buttonState_[2] = { 0 };
	MouseButton		buttonDown_ = 0;
	MouseButton		buttonUp_ = 0;
	int				positionX_[2];
	int				positionY_[2];
	int				wheel_[2];
	int				screenWidth_ = 0;
	int				screenHeight_ = 0;
	HWND			hWnd_ = nullptr;
};
