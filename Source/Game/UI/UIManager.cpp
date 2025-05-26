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
	generates_.clear();
}

void UIManager::Update(const float& elapsedTime)
{
	//	削除対象のUIを userInterfaces_ から削除 (遅延削除)
	userInterfaces_.erase(std::remove_if(userInterfaces_.begin(), userInterfaces_.end(),
		[this](const std::unique_ptr<UI>& p) {
			return std::find(removes_.begin(), removes_.end(), p.get()) != removes_.end();
		}),
		userInterfaces_.end());
	removes_.clear(); // 削除リストをクリア

	//	generates_ は Initialize で処理済みなので、Update での特別な処理は不要
	//	(Initialize と Update のどちらで generates_ を処理するかは設計次第)
	//	ここで generates_.clear(); は Initialize() 側で処理するため不要

	for (std::unique_ptr<UI>& ui : userInterfaces_)
	{
		ui->Update(elapsedTime);
	}
}

//	UI登録
void UIManager::Register(std::unique_ptr<UI> ui)
{
	if (ui == false)
	{
		_ASSERT_EXPR(false, L"Attempted to register a nullptr UI.");
		return;
	}
	UI* rawPtr = ui.get();
	userInterfaces_.emplace_back(std::move(ui));	//	所有権をムーブして登録
	generates_.emplace_back(rawPtr);				//	新規追加リストにも登録
}

//	UITempo登録
void UIManager::RegisterUITempo(UITempo* uiTempo)
{
	uiTempo_ = uiTempo;
}

//	UITempoを取得
UITempo* UIManager::GetUITempo()
{
	_ASSERT_EXPR(uiTempo_ != nullptr, L"uiTempo_ is nullptr.");
	return uiTempo_;
}

//	ランクUI登録
void UIManager::RegisterUIRank(UIRank* uiRank)
{
	uiRank_ = uiRank;
}

//	ランクUI取得
UIRank* UIManager::GetUIRank()
{
	_ASSERT_EXPR(uiRank_ != nullptr, L"uiRank_ is nullptr.");
	return uiRank_;
}

//	UI削除
void UIManager::Remove(UI* ui)
{
	if (std::find(removes_.begin(), removes_.end(), ui) == removes_.end()) 
	{
		removes_.emplace_back(ui);
	}
}

//	UI削除
void UIManager::RemoveFromType(const UIType& type)
{
	for (auto& uiPtr : userInterfaces_)
	{
		if (uiPtr->GetUIType() == type)
		{
			Remove(uiPtr.get());
		}
	}
}

void UIManager::Finalize()
{
	userInterfaces_.clear();
	generates_.clear();
	removes_.clear();

	uiTempo_ = nullptr;
	uiRank_ = nullptr;
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
	//	範囲外ならアサートで落とす
	if (num < 0 || static_cast<size_t>(num) >= userInterfaces_.size())
    {
        _ASSERT_EXPR(false, L"UI index is out of bounds.");
        return nullptr;
    }
	return userInterfaces_.at(num).get();
}

//	種類からUIを取得
UI* UIManager::GetUIFromType(const UIType& type)
{
	for (const auto& ui_ptr : userInterfaces_)
	{
		if (ui_ptr->GetUIType() == type)
		{
			return ui_ptr.get();
		}
	}
	_ASSERT_EXPR(false, L"UI of specified type not found.");
	return nullptr;
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
	for (std::unique_ptr<UI>& ui : userInterfaces_)
	{
		if (ui->GetIsVisible()) ui->Render();
	}
}

void UIManager::DrawDebug()
{
	int size = static_cast<int>(userInterfaces_.size());

	//	UIManager
	if (ImGui::TreeNode("UIManager"))
	{
		ImGui::DragInt("UI Count", &size);
		if(ImGui::Checkbox("AllIsVisible", &allIsVisible_))
		{ 
			SetIsVisible(allIsVisible_);
		}

		//	各UI
		for (std::unique_ptr<UI>& ui : userInterfaces_)
		{
			ui->DrawDebug();
		}

		ImGui::TreePop();
	}

}