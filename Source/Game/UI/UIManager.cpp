#include "UIManager.h"

#include <algorithm>

#include "UI.h"
#include "../../Nova/Graphics/Graphics.h" 
#include "../../../External/imgui/imgui.h"

void UIManager::Initialize()
{
	for (UI* ui : generates_)
	{
		ui->Initialize();
	}
}

void UIManager::Update(const float& elapsedTime)
{
	//	破棄処理
	for (UI* ui : removes_)
	{
		std::vector<UI*>::iterator it =
			std::find(userInterfaces_.begin(), userInterfaces_.end(), ui);

		if (it != userInterfaces_.end())
		{
			userInterfaces_.erase(it);
		}
	}

	//	破棄リストをクリア
	removes_.clear();

	for (UI* ui : generates_)
	{
		userInterfaces_.emplace_back(ui);
	}
	generates_.clear();

	for (UI*& ui : userInterfaces_)
	{
		ui->Update(elapsedTime);
	}
}

void UIManager::Register(UI* ui)
{
	generates_.insert(ui);
}

//	UI削除
void UIManager::Remove(UI* ui)
{
	//	破棄リストに追加
	removes_.insert(ui);
}

//	UI削除
void UIManager::RemoveFromType(const UIType& type)
{
	for (auto& ui : userInterfaces_)
	{
		if (ui->GetUIType() == type)
		{
			removes_.insert(ui);
		}
	}

	//	破棄リストに追加
	//removes_.insert(userInterfaces_.at(static_cast<int>(type)));

}

void UIManager::Finalize()
{
	for (UI*& ui : userInterfaces_)
	{
		delete ui;
	}
	userInterfaces_.clear();
}

void UIManager::SetIsVisible(const bool& isVisible)
{
	//for (UI*& ui : userInterfaces_)
	//{
	//	ui->SetRenderFlag(drawFlag);
	//}
}

//	番号からUIを取得
UI* UIManager::GetUIFromNum(const int& num)
{
	//	numがuserInterfaces_より大きかったらアサートで落とす
	_ASSERT_EXPR(num < userInterfaces_.size(), L"UI num is too large.");

	return userInterfaces_.at(num);
}

//	種類からUIを取得
UI* UIManager::GetUIFromType(const UIType& type)
{
	_ASSERT_EXPR(static_cast<int>(type) < userInterfaces_.size(), L"UI num is too large.");

	return userInterfaces_.at(static_cast<int>(type));
}

//	指定したUIが存在するか
//bool UIManager::ExistUI(const UIType& type)
//{
//	bool found = false;
//
//
//	return found;
//}

//	名前からUIを取得
//UI* UIManager::GetUIFromName(const std::string& name)
//{
//	/*for (UI*& ui : userInterfaces_)
//	{
//		if (ui->GetName().compare(name)) return ui;
//	}
//	_ASSERT_EXPR(false, L"UI is not found.");*/
//}

void UIManager::Render()
{
	//	UI全体表示フラグがfalseなら処理しない
	if (allIsVisible_ == false)return;

	//	ステート設定
	Graphics::Instance().GetShader()->SetDepthStencilState(Shader::DEPTH_STENCIL_STATE::ZT_ON_ZW_ON);
	Graphics::Instance().GetShader()->SetBlendState(Shader::BLEND_STATE::ALPHA);
	Graphics::Instance().GetShader()->SetRasterizerState(Shader::RASTERIZER_STATE::CULL_NONE);

	//	描画
	for (UI*& ui : userInterfaces_)
	{
		if (ui->GetIsVisible()) ui->Render();
	}
}

void UIManager::DrawDebug()
{
	int size = static_cast<int>(userInterfaces_.size());

	//	UIManagerデバッグ
	if (ImGui::TreeNode("UIManager"))
	{
		ImGui::DragInt("UI Count", &size);
		if(ImGui::Checkbox("AllIsVisible", &allIsVisible_))
		{ 
			SetIsVisible(allIsVisible_);
		}

		//	UIデバッグ
		for (UI*& ui : userInterfaces_)
		{
			ui->DrawDebug();
		}

		ImGui::TreePop();
	}

}