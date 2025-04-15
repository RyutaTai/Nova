#include "Transform.h"

//  デバッグ用
void Transform::DrawDebug()
{
    if (ImGui::TreeNode("Transform"))
    {
        ImGui::DragFloat3("Position", &position_.x, 1.0f, -FLT_MAX, FLT_MAX);
        ImGui::DragFloat3("Scale", &scale_.x, 0.001f, -FLT_MAX, FLT_MAX);

        DirectX::XMFLOAT3 rot{};
        rot.x = DirectX::XMConvertToDegrees(rotation_.x);
        rot.y = DirectX::XMConvertToDegrees(rotation_.y);
        rot.z = DirectX::XMConvertToDegrees(rotation_.z);
        ImGui::DragFloat3("Rotation", &rot.x, 0.5f, -FLT_MAX, FLT_MAX);
        rotation_.x = DirectX::XMConvertToRadians(rot.x);
        rotation_.y = DirectX::XMConvertToRadians(rot.y);
        rotation_.z = DirectX::XMConvertToRadians(rot.z);

        ImGui::DragFloat("ScaleFactor", &scaleFactor_, 0.001f, 0.001f, 100.0f);

        ImGui::SliderInt("Coordinate System", &coordinateSystem_, 0, static_cast<int>(CoordinateSystem::cLeftZup));
        ImGui::Text(coordinateSystemName_[coordinateSystem_].c_str());

        if (ImGui::Button("Reset"))
        {
            Reset();
        }
    }
}

//  リセット
void Transform::Reset()
{
    position_ = DirectX::XMFLOAT3(0, 0, 0);
    scale_ = DirectX::XMFLOAT3(1, 1, 1);
    rotation_ = DirectX::XMFLOAT4(0, 0, 0, 0);
}

//  ワールド座標取得
DirectX::XMMATRIX Transform::CalcWorld()
{
    const DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale_.x,scale_.y,scale_.z) * DirectX::XMMatrixScaling(scaleFactor_,scaleFactor_,scaleFactor_) };
    const DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(rotation_.x,rotation_.y,rotation_.z) };
    const DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position_.x,position_.y,position_.z) };

#if 0
    return S * R * T;
#else

    const DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&CoordinateSystemTransforms_[coordinateSystem_]) };

    return C * S * R * T;
#endif
}

//  座標系をかけたワールド行列
DirectX::XMMATRIX Transform::CalcWorldMatrix(float scaleFactor)
{
    const DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&CoordinateSystemTransforms_[coordinateSystem_])
    * DirectX::XMMatrixScaling(scaleFactor,scaleFactor,scaleFactor) };
    return C * CalcWorld();
}

//  前方向ベクトル取得
DirectX::XMFLOAT3 Transform::CalcForward() const
{
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
    DirectX::XMFLOAT3 forward{};
    DirectX::XMStoreFloat3(&forward, DirectX::XMVector3Normalize(rotationMatrix.r[2]));
    return forward;
}

//  上方向ベクトル取得
DirectX::XMFLOAT3 Transform::CalcUp() const
{
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
    DirectX::XMFLOAT3 up{};
    DirectX::XMStoreFloat3(&up, DirectX::XMVector3Normalize(rotationMatrix.r[1]));
    return up;
}

//  右方向ベクトル取得
DirectX::XMFLOAT3 Transform::CalcRight() const
{
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation_.x, rotation_.y, rotation_.z);
    DirectX::XMFLOAT3 right{};
    DirectX::XMStoreFloat3(&right, DirectX::XMVector3Normalize(rotationMatrix.r[0]));
    return right;
}

void Transform::SetWorld(DirectX::XMMATRIX world)
{
    DirectX::XMFLOAT4X4 w;
    DirectX::XMStoreFloat4x4(&w, world);

    position_ = { w._11, w._12, w._13 };
    scale_    = { w._21, w._22, w._23 };
    rotation_ = { w._31, w._32, w._33, w._34 };
}
