#include "Camera.h"

#include <algorithm>

#include "../Core/Framework.h"
#include "../Input/Gamepad.h"
#include "../Others/MathHelper.h"
#include "../Others/Easing.h"
#include "../Input/Input.h"
#include "../../Game/Stage.h"

//	コンストラクタ
Camera::Camera()
{
}

//	デストラクタ
Camera::~Camera()
{
}

//	初期化
void Camera::Initialize()
{
	eye_ = { 10.5f, 8.6f, -23.3f };			//	視点
	focus_ = { 14.0f, 7.2f,  -20.0f };		//	注視点
	up_ = { 0.2f,  0.9f,   0.2f };			//	上方向
	angle_ = { 0.29f,  0.797f,   0.0f };	//	回転値
	eyeOffset_ = { 1.0f,2.0f,0.0f };
	fov_ = 60.0f;							//	視野角
	range_ = 5.0f;							//	ターゲットとカメラとの距離
	nearZ_ = 0.01f;
	farZ_ = 1000.0f;
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

	this->right_.x = world.m[0][0];
	this->right_.y = world.m[0][1];
	this->right_.z = world.m[0][2];

	this->up_.x = world.m[1][0];
	this->up_.y = world.m[1][1];
	this->up_.z = world.m[1][2];

	this->front_.x = world.m[2][0];
	this->front_.y = world.m[2][1];
	this->front_.z = world.m[2][2];
	this->GetTransform()->SetWorld(World);

	//	視点、注視点を保存
	this->eye_ = eye;
	this->focus_ = focus;

}

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
}

//	通常カメラ
void Camera::NormalCamera(const float& elapsedTime)
{
	//#ifdef _DEBUG
//	// カメラを演出で移動中は動かない
//	if (!CameraMove(elapsedTime))
//	{
//		GamePad& gamePad = Input::Instance().GetGamePad();
//		float ax = gamePad.GetAxisRX();
//		float ay = gamePad.GetAxisRY();
//		//カメラの回転速度
//		float speed = rollSpeed_ * elapsedTime;
//
//		//スティックの入力値に合わせてX軸とY軸を回転
//		angle_.x -= ay * speed;
//		angle_.y += ax * speed;
//	}
//#else	//	RELEASE
//	CameraMove(elapsedTime);
//#endif // DEBUG

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
		range_ = minRange_ + (maxRange_ - minRange_) * t;					//	rangeを補完

	}

	//	カメラ回転値を回転行列に変換
	DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(angle_.x, angle_.y, angle_.z);

	//	回転行列から前方向ベクトルを取り出す
	//	Transform.r[2]で行列の３行目のデータを取り出している
	DirectX::XMVECTOR Front = Transform.r[2];
	DirectX::XMFLOAT3 front;
	DirectX::XMStoreFloat3(&front, Front);

	// TODO:SetLoolAt()を呼び出してもfocusの値が変わらない。focusを変えられるようにする
	//		 eyeも変えないとおかしくなるかも
	// TODO:ここの式がおかしい気がする(　front　を　this->front_　に変えても違う気がする)　2024/03/10
	this->eye_.x = this->focus_.x - (front.x * this->range_);
	this->eye_.y = this->focus_.y - (front.y * this->range_);
	this->eye_.z = this->focus_.z - (front.z * this->range_);

	//	カメラの視点と注視点を設定
	Camera::Instance().SetLookAt(eye_, focus_, DirectX::XMFLOAT3(0, 1, 0));

}

//	デバッグ用カメラ(まだ呼び出してない)
//	元Update()にあった処理をDebugCamera()に移した
void Camera::DebugCamera(const float& elapsedTime)
{
#ifdef _DEBUG
	// カメラを演出で移動中は動かない
	if (!CameraMove(elapsedTime))
	{
		GamePad& gamePad = Input::Instance().GetGamePad();
		float RX = gamePad.GetAxisRX();
		float RY = gamePad.GetAxisRY();
		float LY = gamePad.GetAxisLY();

		//	カメラの回転速度
		float aSpeed = rollSpeed_ * elapsedTime;
		float moveSpeed = moveSpeed_ * elapsedTime;

		//	カメラ移動(X軸、Y軸)
		//	左のShiftキーと右スティック(IJKLキー)いずれかを押しているとき、
		//	カメラの視点をスティックの入力値に合わせてX,Y軸方向に移動
		//if ((gamePad.GetButtonDown()&GamePad::BTN_LEFT_TRIGGER) && (RX != 0 || RY != 0))
		if ((GetAsyncKeyState(VK_LSHIFT)) && (RX != 0 || RY != 0))
		{
			eyeOffset_.x -= RX * moveSpeed;
			eyeOffset_.y += RY * moveSpeed;
		}
		else	//	カメラ回転　右スティックの入力値に合わせてX軸とY軸を回転(注視点を回転させる)
		{
			angle_.x += RY * aSpeed;
			angle_.y += RX * aSpeed;
			eyeOffset_.x = 0;	//	X,Y軸のカメラ移動値をリセット
			eyeOffset_.y = 0;
		}

		//	カメラ移動(Z軸)　左スティック(W,Aキー)の入力値に合わせてZ軸方向に移動
		//	入力がなかったらリセット
		if (gamePad.GetAxisLY() != 0)//	左スティック
		{
			eyeOffset_.z -= LY * moveSpeed;
		}
		else	eyeOffset_.z = 0;

#if 1 //	これを外すと向いてる方向に行かなくなる
		//////////////////////////////////////////////////////////////////////////////////////////////////
		DirectX::XMFLOAT3 cameraRight = this->GetRight();
		DirectX::XMFLOAT3 cameraFront = this->GetFront();
		DirectX::XMFLOAT3 cameraUp = this->GetUp();

		//	カメラ右方向ベクトルを単位ベクトルに変換
		float Rlength;
		DirectX::XMStoreFloat(&Rlength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraRight)));
		float cameraRightLength = DirectX::XMVectorGetX(DirectX::XMVectorSqrt(DirectX::XMLoadFloat3(&cameraRight)));
		if (cameraRightLength > 0.0f)
		{
			//	単位ベクトル化
			DirectX::XMVECTOR cameraRightVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraRight));
			DirectX::XMStoreFloat3(&cameraRight, cameraRightVec);
		}

		//	カメラ前方向ベクトルを単位ベクトルに変換
		float Zlength;
		DirectX::XMStoreFloat(&Zlength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraFront)));
		float cameraFrontLength = DirectX::XMVectorGetX(DirectX::XMVectorSqrt(DirectX::XMLoadFloat3(&cameraFront)));
		if (cameraFrontLength > 0.0f)
		{
			//	単位ベクトル化
			DirectX::XMVECTOR cameraFrontVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraFront));
			DirectX::XMStoreFloat3(&cameraFront, cameraFrontVec);
		}

		//	カメラ上方向ベクトルを単位ベクトルに変換
		float Ulength;
		DirectX::XMStoreFloat(&Ulength, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&cameraUp)));
		float cameraUpLength = DirectX::XMVectorGetX(DirectX::XMVectorSqrt(DirectX::XMLoadFloat3(&cameraUp)));
		if (cameraUpLength > 0.0f)
		{
			//	単位ベクトル化
			DirectX::XMVECTOR cameraUpVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&cameraUp));
			DirectX::XMStoreFloat3(&cameraUp, cameraUpVec);
		}

		//	スティックの水平入力値をカメラ右方向に反映し、
		//	スティック垂直入力値をカメラ前方向に反映し、
		//	進行ベクトルを計算する
		eyeOffset_.x = cameraFront.x * LY * moveSpeed_ + cameraRight.x * RX * moveSpeed_ + cameraUp.x * RY * moveSpeed_;
		eyeOffset_.y = cameraFront.y * LY * moveSpeed_ + cameraRight.y * RX * moveSpeed_ + cameraUp.y * RY * moveSpeed_;
		eyeOffset_.z = cameraFront.z * LY * moveSpeed_ + cameraRight.z * RX * moveSpeed_ + cameraUp.z * RY * moveSpeed_;
		////////////////////////////////////////////////////////////////////////////////////////////
#endif

	//	カメラ移動(X軸、Y軸)
	//	左のShiftキーと右スティック(IJKLキー)いずれかを押しているとき、
	//	カメラの視点をスティックの入力値に合わせてX,Y軸方向に移動
		//if ((gamePad.GetButtonDown()&GamePad::BTN_LEFT_TRIGGER) && (RX != 0 || RY != 0))
		if ((GetAsyncKeyState(VK_LSHIFT)) && (RX != 0 || RY != 0))
		{
			eyeOffset_.x -= RX * moveSpeed;
			eyeOffset_.y += RY * moveSpeed;
		}
		else	//	カメラ回転　右スティックの入力値に合わせてX軸とY軸を回転(注視点を回転させる)
		{
			angle_.x += RY * aSpeed;
			angle_.y += RX * aSpeed;
			eyeOffset_.x = 0;	//	X,Y軸のカメラ移動値をリセット
			eyeOffset_.y = 0;
		}

		//	カメラ移動(Z軸)　左スティック(W,Aキー)の入力値に合わせてZ軸方向に移動
		//	入力がなかったらリセット
		if (gamePad.GetAxisLY() != 0)//	左スティック
		{
			eyeOffset_.z -= LY * moveSpeed;
		}
		else	eyeOffset_.z = 0;

	}
#else	//	RELEASE
	CameraMove(elapsedTime);

#endif // DEBUG

	//	カメラ回転値を回転行列に変換
	DirectX::XMMATRIX Transform = DirectX::XMMatrixRotationRollPitchYaw(angle_.x, angle_.y, angle_.z);

	//	回転行列から前方向ベクトルを取り出す
	//	Transform.r[2]で行列の３行目のデータを取り出している
	DirectX::XMVECTOR Front = Transform.r[2];
	DirectX::XMFLOAT3 front;
	DirectX::XMStoreFloat3(&front, Front);

	// TODO:SetLoolAt()を呼び出してもfocusの値が変わらない。focusを変えられるようにする
	//		 eyeも変えないとおかしくなるかも
	focus_.x = eye_.x - (front.x * range_);
	focus_.y = eye_.y - (front.y * range_);
	focus_.z = eye_.z - (front.z * range_);

	eye_ = eye_ + eyeOffset_;	//	カメラ視点の更新

	//	カメラの視点と注視点を設定
	Camera::Instance().SetLookAt(eye_, focus_, DirectX::XMFLOAT3(0, 1, 0));
}

//	カメラからステージへレイキャスト
bool Camera::RayCastVsStage(DirectX::XMFLOAT3& intersectionPos, DirectX::XMFLOAT3& intersectionNormal, std::string& intersectionMesh, std::string& intersectionMaterial)
{
	//	レイキャストに必要なパラメータ
	DirectX::XMFLOAT3 pos = GetEye();
	DirectX::XMFLOAT3 dir = GetFront();
	DirectX::XMFLOAT4X4 transform = {};								//	ステージのワールド変換行列
	DirectX::XMStoreFloat4x4(&transform, Stage::Instance().GetTransform()->CalcWorld());

	return Stage::Instance().Collision(pos, dir, transform, intersectionPos, intersectionNormal, intersectionMesh, intersectionMaterial);
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
	DirectX::XMFLOAT3 right = this->GetTransform()->CalcRight();
	DirectX::XMFLOAT3 forward = this->GetTransform()->CalcForward();
	
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
		ImGui::DragFloat3	("Right",		&right.x,		0.01f,	-FLT_MAX,	FLT_MAX);	//	右方向
		ImGui::DragFloat3	("Up",			&up_.x,			0.01f,	-FLT_MAX,	FLT_MAX);	//	上方向
		ImGui::DragFloat3	("Forward",		&forward.x,		0.01f,	-FLT_MAX,	FLT_MAX);	//	前方向

		ImGui::Text("----- Angle -----");
		float maxAngleX = MaxAngleX_;
		float minAngleX = MinAngleX_;
		ImGui::DragFloat3	("Angle",		&angle_.x,		0.01f,	-FLT_MAX,	FLT_MAX);	//	回転値
		ImGui::DragFloat	("MinAngleX_", &minAngleX);	//	X軸の最小角度
		ImGui::DragFloat	("MaxAngleX_", &maxAngleX);	//	X軸の最大角度
		
		ImGui::Text("----- Range -----");
		ImGui::DragFloat	("MinRange",	&minRange_, 0.01f);
		ImGui::DragFloat	("MaxRange",	&maxRange_, 0.01f);
		ImGui::DragFloat	("Range",		&range_,		0.1f,	FLT_MIN,	FLT_MAX);	//	間隔
		if (ImGui::Button	("Reset"))
		{
			Reset();
		}
		ImGui::TreePop();
	}
}
