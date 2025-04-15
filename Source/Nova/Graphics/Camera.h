#pragma once

#include "../Others/Transform.h"

class Camera
{
private:
	Camera();
	~Camera();

public:
	static Camera& Instance()
	{
		static Camera instance_;
		return instance_;
	}

	void Initialize();
	void Update(const float& elapsedTime);	//	更新処理

	void NormalCamera(const float& elapsedTime);	//	通常カメラ

	//	この関数をコールすればカメラ移動がイージングで始まる
	//	第一引数：目標座標
	//	第二引数：目標角度
	//	第三引数：何秒で移動するか
	bool LaunchCameraMove(const DirectX::XMFLOAT3& targetEye, const DirectX::XMFLOAT3& targetAngle, const float& moveTime);

	//	デバッグ用
	void DebugCamera(const float& elapsedTime);	//	デバッグ用カメラ更新処理
	void DrawDebug();						//	デバッグ描画
	void Reset();							//	リセット

	void SetPerspectiveFov();		//	パースペクティブ設定
	void SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up);	//	指定方向を向く
	void SetEye(const DirectX::XMFLOAT3& eye)		{ this->eye_ = eye; }					//	視点設定
	void SetEyeX(const float& eyeX)					{ this->eye_.x = eyeX; }	
	void SetEyeY(const float& eyeY)					{ this->eye_.y = eyeY; }
	void SetEyeZ(const float& eyeZ)					{ this->eye_.z = eyeZ; }
	void SetTargetPos(const DirectX::XMFLOAT3& target) { this->focus_ = target; }				//	ターゲット位置設定
	void SetRange(const float& range)				{ this->range_ = range; }				//	カメラ距離設定
	void SetIsPose(const bool& isPose)				{ this->isPose_ = isPose; }

	Transform*					GetTransform()		{ return &transform_; }
	const DirectX::XMMATRIX		GetViewMatrix()const{ return viewMatrix_; }					//	ビュー行列取得
	const DirectX::XMMATRIX		GetProjectionMatrix()const{ return projectionMatrix_; }		//	プロジェクション行列取得
	const DirectX::XMMATRIX		CalcViewProjectionMatrix();									//	ビュープロジェクション行列取得	
	const DirectX::XMMATRIX		CalcInvViewProjectionMatrix();								//	ビュープロジェクション行列取得	
	const DirectX::XMFLOAT3		GetEye()	const	{ return eye_; }						//	視点取得
	const DirectX::XMFLOAT3		GetFocus()	const	{ return focus_; }						//	注視点取得
	const DirectX::XMFLOAT3		GetUp()		const	{ return up_; }							//	カメラ上方向取得
	const DirectX::XMFLOAT3		GetFront()	const	{ return front_; }						//	カメラ前方向取得
	const DirectX::XMFLOAT3		GetRight()	const	{ return right_; }						//	カメラ右方向取得
	const float					GetNearZ()  const	{ return nearZ_; }
	const float					GetFarZ()	const	{ return farZ_; }
	bool						GetIsPose()			{ return isPose_; }

	//	----- カメラ移動 -----
	bool						IsCameraMove()		{ return cameraMove_; }					//	カメラ演出中かどうか
	bool						CameraMove(const float& elapsedTime);

	bool						RayCastVsStage(DirectX::XMFLOAT3& intersectionPos, DirectX::XMFLOAT3& intersectionNormal, std::string& intersectionMesh, std::string& intersectionMaterial);

private:
	Transform transform_ = {};

	static constexpr float MaxAngleX_ = DirectX::XMConvertToRadians(45);
	static constexpr float MinAngleX_ = DirectX::XMConvertToRadians(-45);
	float minRange_ = 2.0f;
	float maxRange_ = 9.0f;
	//float maxRange_ = 5.0f;

	DirectX::XMMATRIX viewMatrix_;								//	ビュー行列
	DirectX::XMMATRIX projectionMatrix_;						//	プロジェクション行列
	DirectX::XMMATRIX viewProjectionMatrix_;					//	ビュープロジェクション行列
	DirectX::XMMATRIX invViewProjectionMatrix_;					//	ビュープロジェクション逆行列
	DirectX::XMFLOAT3 eye_ = {};								//	カメラの視点
	DirectX::XMFLOAT3 focus_		= { 0,-10,10 };				//	カメラの注視点
	DirectX::XMFLOAT3 angle_		= { 0,0,0 };				//	カメラの回転値
	DirectX::XMFLOAT3 eyeOffset_	= { 0,0,0 };				//	カメラの視点eye_を動かすときの移動値

	float fov_				= 60.0f;							//	視野角
	float range_			= 250.0f;							//	カメラとターゲットの距離を決めるのに使う
	float rollSpeed_		= DirectX::XMConvertToRadians(80);	//	カメラの回転速度
	float maxAngleX_		= DirectX::XMConvertToRadians(45);	//	カメラの回転値制限
	float minAngleX_		= DirectX::XMConvertToRadians(-45);	//	カメラの回転値制限
	float moveSpeed_		=  2.0f;							//	カメラの移動速度
	float maxEyeOffset_		=  2.0f;
	float minEyeOffset_		= -2.0f;
	float nearZ_			= 50.0f;
	float farZ_				= 400000.0f;

	DirectX::XMFLOAT3 up_ = {};										//	カメラの上方向
	DirectX::XMFLOAT3 front_ = {};									//	カメラの前方向
	DirectX::XMFLOAT3 right_ = {};									//	カメラの右方向

	bool isPose_ = false;

	bool cameraMove_ = false;
	float moveTime_ = 0.0f;
	float moveTimer_ = 0.0f;
	DirectX::XMFLOAT3 moveTargetEye_ = {};
	DirectX::XMFLOAT3 moveTargetAngle_ = {};
	DirectX::XMFLOAT3 cashPos_ = {};
	DirectX::XMFLOAT3 cashAngle_ = {};

	//	デバッグ用
	bool isDebugCamera_ = false;

};

