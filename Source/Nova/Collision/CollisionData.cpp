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
	ImGui::DragFloat("Height", &height_, 0.01f);
	ImGui::ColorEdit4("CurrentColor", &currentColor_.x);
	ImGui::ColorEdit4("DefaultColor", &defaultColor_.x);
	ImGui::ColorEdit4("HitColor", &hitColor_.x);

}

//	攻撃判定用ImGui
void AttackDetectionData::DrawDebug()
{
	if (ImGui::TreeNode(GetName().c_str()))
	{
		//	AttackDetectionData自身のプロパティ
		char bufName[256];
		strcpy_s(bufName, sizeof(bufName), GetName().c_str());
		if (ImGui::InputText("Name", bufName, sizeof(bufName)))
		{
			SetName(bufName);
		}
		char bufUpdateName[256];
		strcpy_s(bufUpdateName, sizeof(bufUpdateName), updateName_.c_str());
		if (ImGui::InputText("Update Name", bufUpdateName, sizeof(bufUpdateName))) 
		{
			updateName_ = bufUpdateName;
		}
		ImGui::Checkbox("Is Active", &isActive_);

		//	内部のCollisionSphereDataのプロパティ
		if (ImGui::TreeNode("Collision Sphere Data"))
		{
			collisionSphereData_.DrawDebug();
			ImGui::TreePop();
		}
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
//	くらい判定用ImGui
void DamageDetectionData::DrawDebug()
{
	if (ImGui::TreeNode(GetName().c_str()))
	{
		// DamageDetectionData自身のプロパティ
		char bufName[256];
		strcpy_s(bufName, sizeof(bufName), GetName().c_str());
		if (ImGui::InputText("Name", bufName, sizeof(bufName))) 
		{
			SetName(bufName);
		}
		char bufUpdateName[256];
		strcpy_s(bufUpdateName, sizeof(bufUpdateName), updateName_.c_str());
		if (ImGui::InputText("Update Name", bufUpdateName, sizeof(bufUpdateName)))
		{
			updateName_ = bufUpdateName;
		}
		ImGui::DragFloat("Damage Multiplier", &damage_, 0.01f);
		ImGui::Checkbox("Is Hit", &isHit_); 
		ImGui::DragFloat("Hit Timer", &hitTimer_, 0.01f);

		// 内部のCollisionSphereDataのプロパティ
		if (ImGui::TreeNode("Collision Sphere Data"))
		{
			collisionSphereData_.DrawDebug();
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}

//	押し出し判定用ImGui
void CollisionDetectionData::DrawDebug()
{
	if (ImGui::TreeNode(GetName().c_str()))
	{
		// CollisionDetectionData自身のプロパティ
		char bufName[256];
		strcpy_s(bufName, sizeof(bufName), GetName().c_str());
		if (ImGui::InputText("Name", bufName, sizeof(bufName)))
		{
			SetName(bufName);
		}
		char bufUpdateName[256];
		strcpy_s(bufUpdateName, sizeof(bufUpdateName), updateName_.c_str());
		if (ImGui::InputText("Update Name", bufUpdateName, sizeof(bufUpdateName))) 
		{
			updateName_ = bufUpdateName;
		}
		ImGui::Checkbox("Is Active", &isActive_);
		ImGui::Checkbox("Fixed Y", &fixedY_);

		// 内部のCollisionSphereDataのプロパティ
		if (ImGui::TreeNode("Collision Sphere Data"))
		{
			collisionSphereData_.DrawDebug();
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}
