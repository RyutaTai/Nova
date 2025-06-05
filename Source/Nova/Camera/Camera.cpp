#include "Camera.h"

#include <algorithm>

#include "../Core/Framework.h"
#include "../Input/Gamepad.h"
#include "../Others/MathHelper.h"
#include "../Others/Easing.h"
#include "../Input/Input.h"
#include "../../Game/Stage/Stage.h"

//	コンストラクタ
Camera::Camera()
{
	//	リスナー情報セット
	listener_.innerRadius_ = 0.7f;
	listener_.outerRadius_ = 1.67f;
	listener_.filterParam_ = 0.8f;

}

//	初期化
void Camera::Initialize()
{
	eye_	= { 10.5f, 8.6f, -23.3f };		//	視点
	focus_	= { 14.0f, 7.2f,  -20.0f };		//	注視点
	up_		= { 0.2f,  0.9f,   0.2f };		//	上方向
	angle_	= { 0.29f,  0.797f,   0.0f };	//	回転値
	eyeOffset_ = { 1.0f,2.0f,0.0f };
	fov_ = 60.0f;							//	視野角
	currentRange_ = 5.0f;					//	ターゲットとカメラとの距離
	nearZ_	= 0.01f;
	farZ_	= 1000.0f;
}

//	パースペクティブ設定
void Camera::SetPerspectiveFov()
{
    //	画面アスペクト比
    float aspectRatio = SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    //	プロジェクション行列
    projectionMatrix_ = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov_), aspectRatio, nearZ_, farZ_);

	DirectX::XMVECTOR eye, focus, up;
	eye = DirectX::XMVectorSet(eye_.x, eye_.y, eye_.z, 1.0f);
	focus = DirectX::XMVectorSet(focus_.x,focus_.y,focus_.z, 1.0f);
	up = DirectX::XMVectorSet(up_.x, up_.y, up_.z, 0.0f);
	//	カメラの視点(ビュー行列)
	viewMatrix_ = DirectX::XMMatrixLookAtLH(eye, focus, up);

}

//	プロジェクション逆行列算出
const DirectX::XMMATRIX Camera::CalcInvProjectionMatrix()
{
	invProjectionMatrix_ = DirectX::XMMatrixInverse(NULL, GetProjectionMatrix());
	return invProjectionMatrix_;
}

//	ビュープロジェクション行列算出
const DirectX::XMMATRIX Camera::CalcViewProjectionMatrix()
{
	viewProjectionMatrix_ = viewMatrix_* projectionMatrix_;
	return viewProjectionMatrix_;
}

//	ビュープロジェクション逆行列算出
const DirectX::XMMATRIX Camera::CalcInvViewProjectionMatrix()
{
	invViewProjectionMatrix_ = DirectX::XMMatrixInverse(NULL, CalcViewProjectionMatrix());
	return invViewProjectionMatrix_;
}

//	指定方向を向く
void Camera::SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
	//	視点、注視点、上方向からビュー行列を作成
	DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&eye);
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&up);
	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	viewMatrix_ = View;

	//	ビューを逆行列化し、ワールド行列に戻す
	//	XMMatrixInverse関数で逆行列を求める
	DirectX::XMMATRIX World = DirectX::XMMatrixInverse(nullptr, View);
	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, World);

	//	カメラの方向を取り出す
	right_.x = world.m[0][0];
	right_.y = world.m[0][1];
	right_.z = world.m[0][2];

	up_.x = world.m[1][0];
	up_.y = world.m[1][1];
	up_.z = world.m[1][2];

	front_.x = world.m[2][0];
	front_.y = world.m[2][1];
	front_.z = world.m[2][2];
	GetTransform()->SetWorld(World);

	//	視点、注視点を保存
	eye_ = eye;
	focus_ = focus;

}

//	カメラ演出に使うパラメータの設定
bool Camera::LaunchCameraMove(const DirectX::XMFLOAT3& targetEye, const DirectX::XMFLOAT3& targetAngle, const float& moveTime)
{
	if (cameraMove_)return false;

	moveTime_ = moveTime;
	moveTimer_ = 0.0f;

	cashPos_ = eye_;
	cashAngle_ = angle_;
	moveTargetEye_ = targetEye;
	moveTargetAngle_ = targetAngle;

	cameraMove_ = true;

	return true;
}

//	LaunchCameraMoveで設定したパラメータをもとにカメラを移動させる
bool Camera::CameraMove(const float& elapsedTime)
{
	if (cameraMove_ == false)return false;

	if (moveTimer_ >= moveTime_)
	{
		eye_ = moveTargetEye_;
		angle_ = moveTargetAngle_;
		cameraMove_ = false;
		return false;
	}

	eye_.x = Easing::InSine(moveTimer_, moveTime_, moveTargetEye_.x, cashPos_.x);
	eye_.y = Easing::InSine(moveTimer_, moveTime_, moveTargetEye_.y, cashPos_.y);
	eye_.z = Easing::InSine(moveTimer_, moveTime_, moveTargetEye_.z, cashPos_.z);
	angle_.x = Easing::InSine(moveTimer_, moveTime_, moveTargetAngle_.x, cashAngle_.x);
	angle_.y = Easing::InSine(moveTimer_, moveTime_, moveTargetAngle_.y, cashAngle_.y);
	angle_.z = Easing::InSine(moveTimer_, moveTime_, moveTargetAngle_.z, cashAngle_.z);

	moveTimer_ += elapsedTime;

    return true;
}

//	更新処理
void Camera::Update(const float& elapsedTime)
{
	if (isDebugCamera_)DebugCamera(elapsedTime);
	else NormalCamera(elapsedTime);

	//	----- オーディオリスナー更新 -----
	UpdateListener();

	//	----- 当たり判定 -----
	//RayVsHorizontal(elapsedTime);

}

//	リスナー情報更新
void Camera::UpdateListener()
{
	listener_.position_ = { eye_.x, eye_.y , eye_.z };
	listener_.frontVec_ = GetFront();
	listener_.velocity_ = {0,0,0};
	listener_.rightVec_ = GetRight();

}
//	通常カメラ
void Camera::NormalCamera(const float& elapsedTime)
{
	//	右スティックでカメラ回転
	if (isPose_ == false)
	{
		GamePad& gamePad = Input::Instance().GetGamePad();
		float ax = gamePad.GetAxisRX();
		float ay = gamePad.GetAxisRY();
		//	カメラの回転速度
		float speed = rollSpeed_ * elapsedTime;

		//	スティックの入力値に合わせてX軸とY軸を回転
		angle_.x -= ay * speed;
		angle_.y += ax * speed;

		//	X軸のカメラ回転を制限
		if (angle_.x < MinAngleX_)
		{
			angle_.x = MinAngleX_;
		}
		if (angle_.x > MaxAngleX_)
		{
			angle_.x = MaxAngleX_;
		}

		//	Y軸の回転値を-3.14～3.14に収まるようにする
		if (angle_.y < -DirectX::XM_PI)
		{
			angle_.y += DirectX::XM_2PI;
		}
		if (angle_.y > DirectX::XM_PI)
		{
			angle_.y -= DirectX::XM_2PI;
		}

		//	range_を線形補間
		float t = (angle_.x - MinAngleX_) / (MaxAngleX_ - MinAngleX_);		//	補間係数tを計算
		currentRange_ = minRange_ + (maxRange_ - minRange_) * t;			//	rangeを補完

		//	ベロシティ追加
		velocity_.x = ay * speed;
		velocity_.y = ax * speed;

	}

	//	カメラ回転値を回転行列に変換
	DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(angle_.x, angle_.y, angle_.z);

	//	回転行列から前方向ベクトルを取り出す
	//	Transform.r[2]で行列の３行目のデータを取り出している
	DirectX::XMVECTOR Front = Transform.r[2];
	DirectX::XMFLOAT3 front;
	DirectX::XMStoreFloat3(&front, Front);

	eye_.x = focus_.x - (front.x * currentRange_);
	eye_.y = focus_.y - (front.y * currentRange_);
	eye_.z = focus_.z - (front.z * currentRange_);

	//	カメラの視点と注視点を設定
	Camera::Instance().SetLookAt(eye_, focus_, DirectX::XMFLOAT3(0, 1, 0));

}

//	カメラからステージへレイキャスト
bool Camera::RayVsHorizontal(const float& elapsedTime)
{
	DirectX::XMFLOAT3 rayStartPos;							//	レイの始点
	DirectX::XMFLOAT3 rayDirection;							//	レイの方向
	float liftup = 0.01f;									//	レイの始点を持ち上げる
	DirectX::XMVECTOR RayPos = DirectX::XMLoadFloat3(&eye_);							//	レイの始点
	DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSet(velocity_.x, 0.0f, velocity_.z, 0.0f));	//	レイの方向
	DirectX::XMVECTOR Liftup = DirectX::XMVector3Normalize(DirectX::XMVectorSet(0.0f, liftup, 0.0f, 1.0f));		//	LIFTUP
#if 1
	DirectX::XMStoreFloat3(&rayStartPos, DirectX::XMVectorAdd(RayPos, Liftup));
#else
	float stepBack = 1.0f;
	DirectX::XMStoreFloat3(&rayStartPos, DirectX::XMVectorSubtract(RayPos, DirectX::XMVectorScale(Direction, stepBack)));
#endif
	DirectX::XMStoreFloat3(&rayDirection, Direction);

	DirectX::XMFLOAT3 cameraPos = GetTransform()->GetPosition();	//	プレイヤーの位置(足元が基準点)

	DirectX::XMFLOAT4X4 transform = {};								//	ステージのワールド変換行列
	DirectX::XMStoreFloat4x4(&transform, Stage::Instance().GetTransform()->CalcWorld());

	//	当たり判定結果格納用
	DirectX::XMFLOAT3	intersectionPosition	= {};			//	当たった位置
	DirectX::XMFLOAT3	intersectionNormal		= {};			//	法線の方向
	std::string			intersectionMesh		= {};			//	メッシュ名
	std::string			intersectionMaterial	= {};			//	マテリアル名

	//	当たり判定処理
	bool isHit = false;
	//	レイと地面が当たっていたら
	if (Stage::Instance().Collision(rayStartPos, rayDirection, transform, intersectionPosition, intersectionNormal, intersectionMesh, intersectionMaterial))
	{
		float d0 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraPos) - DirectX::XMLoadFloat3(&rayStartPos)));
		float d1 = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&intersectionPosition) - DirectX::XMLoadFloat3(&rayStartPos)));

		float rayOffset = 0.5f;	//	レイの長さを少し増やす

		//	プレイヤーと地面が当たっていたら
		if (d0 + radius_ + rayOffset > d1)
		{
			//	プレイヤーの位置を補正
			float d = d0 - d1;
			cameraPos.x -= d * rayDirection.x;
			cameraPos.y -= d * rayDirection.y;
			cameraPos.z -= d * rayDirection.z;

			eye_ = cameraPos;

			// Reflection
			//DirectX::XMStoreFloat3(&velocity_, DirectX::XMVector3Reflect(DirectX::XMLoadFloat3(&velocity_), DirectX::XMLoadFloat3(&intersectionNormal)));

			//	当たり判定フラグを立てる
			isHit = true;

		}

	}

	return isHit;
}

//	リセット
void Camera::Reset()
{
	eye_		= { 0.0f,	10.0f,	100.0f };	//	カメラ視点
	eyeOffset_	= { 1.0f,	2.0f,	0.0f };		//	カメラ視点補正値
	focus_		= { 0.0f,	25.0f,	50.0f };	//	カメラの注視点
	up_			= { 0.0f,	0.0f,	0.0f };		//	カメラの上方向
	angle_		= { 0.0f,	0.0f,	0.0f };		//	カメラの回転値
	fov_		= 60.0f;						//	視野角
}

//	デバッグ描画
void Camera::DrawDebug()
{	
	if (ImGui::TreeNode(u8"Cameraカメラ"))
	{
		ImGui::Checkbox("DebugCamera", &isDebugCamera_);	//	デバッグカメラ切り替え
		if (ImGui::TreeNode("Transform"))
		{
			GetTransform()->DrawDebug();
			ImGui::TreePop();
		}
		
		ImGui::DragFloat	("NearZ",		&nearZ_,		1.0f,	FLT_MIN,	FLT_MAX);	//	Near
		ImGui::DragFloat	("FarZ",		&farZ_,			1.0f,	FLT_MIN,	FLT_MAX);	//	Far
		ImGui::DragFloat	("MoveSpeed",	&moveSpeed_,	0.01f,	-FLT_MAX,	FLT_MAX);	//	移動速度
		ImGui::DragFloat	("RollSpeed",	&rollSpeed_,	0.01f,	-FLT_MAX,	FLT_MAX);	//	回転速度
		ImGui::DragFloat3	("Eye",			&eye_.x,		0.01f,	-FLT_MAX,	FLT_MAX);	//	カメラ視点
		ImGui::DragFloat3	("EyeOffset",	&eyeOffset_.x,	0.001f,	-FLT_MAX,	FLT_MAX);	//	カメラ視点補正値
		ImGui::DragFloat3	("Focus",		&focus_.x,		0.01f,	-FLT_MAX,	FLT_MAX);	//	注視点
		ImGui::DragFloat3	("Right",		&right_.x,		0.01f,	-FLT_MAX,	FLT_MAX);	//	右方向
		ImGui::DragFloat3	("Up",			&up_.x,			0.01f,	-FLT_MAX,	FLT_MAX);	//	上方向
		ImGui::DragFloat3	("Front",		&front_.x,		0.01f,	-FLT_MAX,	FLT_MAX);	//	前方向


		ImGui::Text("----- Angle -----");
		float maxAngleX = MaxAngleX_;
		float minAngleX = MinAngleX_;
		ImGui::DragFloat3	("Angle",		&angle_.x,		0.01f,	-FLT_MAX,	FLT_MAX);	//	回転値
		ImGui::DragFloat	("MinAngleX_", &minAngleX);	//	X軸の最小角度
		ImGui::DragFloat	("MaxAngleX_", &maxAngleX);	//	X軸の最大角度
		
		ImGui::Text("----- Range -----");
		ImGui::DragFloat	("MinRange",	&minRange_, 0.01f);
		ImGui::DragFloat	("MaxRange",	&maxRange_, 0.01f);
		ImGui::DragFloat	("Range",		&currentRange_,		0.1f,	FLT_MIN,	FLT_MAX);	//	間隔
		
		//	3Dオーディオのリスナー情報
		if (ImGui::TreeNode("3DAudio_Listener"))
		{
			ImGui::DragFloat3("Position", &listener_.position_.x);
			ImGui::DragFloat("InnerRadius", &listener_.innerRadius_);
			ImGui::DragFloat("OuterRadius", &listener_.outerRadius_);
			ImGui::DragFloat("FilterParam", &listener_.filterParam_);
			ImGui::DragFloat3("FrontVec", &listener_.frontVec_.x);
			ImGui::DragFloat3("RightVec", &listener_.rightVec_.x);
			ImGui::DragFloat3("Velocity", &listener_.velocity_.x);

			ImGui::TreePop();
		}

		//	当たり判定
		if (ImGui::TreeNode("Collision"))
		{
			ImGui::DragFloat("Radius", &radius_, 0.01f);
			ImGui::DragFloat3("Velocity", &velocity_.x, 0.01f);	//	ベロシティ
			ImGui::TreePop();
		}

		if (ImGui::Button	("Reset"))
		{
			Reset();
		}
		ImGui::TreePop();
	}
}
