#include "CollisionData.h"

#include "../../imgui/imgui.h"

//	球判定用ImGui
void CollisionSphereData::DrawDebug()
{
	ImGui::DragFloat3("JointPos", &jointPosition_.x, 0.01f);
	ImGui::DragFloat3("OffsetPos", &offsetPosition_.x, 0.01f);
	ImGui::DragFloat("Radius", &radius_, 0.01f);

	ImGui::ColorEdit4("CurrentColor", &currentColor_.x);
	ImGui::ColorEdit4("DefaultColor", &defaultColor_.x);
	ImGui::ColorEdit4("HitColor", &hitColor_.x);

}

//	円柱判定用ImGui
void CollisionCylinderData::DrawDebug()
{
	ImGui::DragFloat3("JointPos", &jointPosition_.x, 0.01f);
	ImGui::DragFloat3("OffsetPos", &offsetPosition_.x, 0.01f);
	ImGui::DragFloat("Radius", &radius_, 0.01f);
	ImGui::ColorEdit4("CurrentColor", &currentColor_.x);
	ImGui::ColorEdit4("DefaultColor", &defaultColor_.x);
	ImGui::ColorEdit4("HitColor", &hitColor_.x);

}

//	攻撃判定用
void AttackDetectionData::DrawDebug()
{
	if (ImGui::TreeNode(GetName().c_str()))
	{
		collisionSphereData_.DrawDebug();
		ImGui::TreePop();
	}
}

//	くらい判定更新処理
void DamageDetectionData::Update(const float& elapsedTime)
{
	//	ヒットしたら色を変える
	if (isHit_)
	{
		hitTimer_ -= elapsedTime;

		if (hitTimer_ < 0.0f)
		{
			isHit_ = false;
		}
		SetColor(GetHitColor());
	}
	else
	{
		SetColor(GetDefaultColor());
	}
}

//	くらい判定用
void DamageDetectionData::DrawDebug()
{
	if (ImGui::TreeNode(GetName().c_str()))
	{
		collisionSphereData_.DrawDebug();
		ImGui::Checkbox("IsHit", &isHit_);
		ImGui::DragFloat("HitTimer", &hitTimer_);
		ImGui::DragFloat("Damage", &damage_);
		ImGui::TreePop();
	}
}

//	押し出し判定用
void CollisionDetectionData::DrawDebug()
{
	if (ImGui::TreeNode(GetName().c_str()))
	{
		collisionSphereData_.DrawDebug();
		ImGui::TreePop();
	}
}
