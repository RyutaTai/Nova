#pragma once

using GamePadButton = unsigned int;

// ゲームパッド
class GamePad
{
public:
	static const GamePadButton BTN_UP				= (1 <<  0);
	static const GamePadButton BTN_RIGHT			= (1 <<  1);
	static const GamePadButton BTN_DOWN				= (1 <<  2);
	static const GamePadButton BTN_LEFT				= (1 <<  3);
	static const GamePadButton BTN_A				= (1 <<  4);	//	Zキー
	static const GamePadButton BTN_B				= (1 <<  5);	//	Xキー
	static const GamePadButton BTN_X				= (1 <<  6);	//	Cキー
	static const GamePadButton BTN_Y				= (1 <<  7);	//	Vキー
	static const GamePadButton BTN_START			= (1 <<  8);	//	Enterキー(Xboxコントローラーのメニューボタン)
	static const GamePadButton BTN_BACK				= (1 <<  9);	//	Xboxコントローラーのビューボタン
	static const GamePadButton BTN_LEFT_THUMB		= (1 << 10);
	static const GamePadButton BTN_RIGHT_THUMB		= (1 << 11);
	static const GamePadButton BTN_LEFT_SHOULDER	= (1 << 12);
	static const GamePadButton BTN_RIGHT_SHOULDER	= (1 << 13);
	static const GamePadButton BTN_LEFT_TRIGGER		= (1 << 14);
	static const GamePadButton BTN_RIGHT_TRIGGER	= (1 << 15);
	static const GamePadButton BTN_SPACE			= (1 << 16);
	static const int BTN_MAX = 17;

public:
	GamePad() {}
	~GamePad() {}

	// 更新
	void Update(const float& elapsedTime);

	// スロット設定
	void SetSlot(const int& slot) { this->slot_ = slot; }

	// ボタン入力状態の取得
	GamePadButton GetButton() const { return buttonState_[0]; }

	// ボタン押下状態の取得
	GamePadButton GetButtonDown() const { return buttonDown_; }

	// ボタン押上状態の取得
	GamePadButton GetButtonUp() const { return buttonUp_; }

	// 左スティックX軸入力状態の取得
	float GetAxisLX() const { return axisLx_; }

	// 左スティックY軸入力状態の取得
	float GetAxisLY() const { return axisLy_; }

	// 右スティックX軸入力状態の取得
	float GetAxisRX() const { return axisRx_; }

	// 右スティックY軸入力状態の取得
	float GetAxisRY() const { return axisRy_; }

	// 左トリガー入力状態の取得
	float GetTriggerL() const { return triggerL_; }

	// 右トリガー入力状態の取得
	float GetTriggerR() const { return triggerR_; }

	//	コントローラー振動
	void SetVibration(const float& leftMotor, const float& rightMotor, const float& time);
	void StopVibration();

private:
	GamePadButton		buttonState_[2] = { 0 };
	GamePadButton		buttonDown_ = 0;
	GamePadButton		buttonUp_ = 0;
	float				axisLx_ = 0.0f;
	float				axisLy_ = 0.0f;
	float				axisRx_ = 0.0f;
	float				axisRy_ = 0.0f;
	float				triggerL_ = 0.0f;
	float				triggerR_ = 0.0f;
	int					slot_ = 0;

	bool				isVibration_ = false;	//	コントローラー振動フラグ
	float				vibrationTime_ = 0.0f;	//	コントローラー振動時間
	
};
