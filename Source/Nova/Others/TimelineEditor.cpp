#include "TimelineEditor.h"

#include <algorithm>

#include "../../imgui/imgui.h"

//  コンストラクタ
TimelineEditor::TimelineEditor(const int& frameMin, const int& frameMax)
    : frameMin_(frameMin), frameMax_(frameMax) 
{}

// キャラクター追加
void TimelineEditor::AddCharacter(const std::string& characterName, const std::unordered_map<std::string, Joint>& joints)
{
    characters_[characterName] = { characterName, joints, {} };
}

// キャラクター削除
void TimelineEditor::RemoveCharacter(const std::string& characterName) {
    characters_.erase(characterName);
}

// タイムライン描画
void TimelineEditor::DrawTimeline() {
    ImGui::Begin("Timeline Editor");
    {
        ImGui::Columns(3, "TimelineColumns", true);

        // 左側: キャラクターリスト
        DrawCharacterList();

        // 中央: グリッドとタイムライン
        ImGui::NextColumn();
        DrawGrid();

        // 右側: プロパティ編集
        ImGui::NextColumn();
        DrawEventProperties();

    }
    ImGui::End();
}

// キャラクターリストの描画
void TimelineEditor::DrawCharacterList() {
    ImGui::Text("Characters:");
    ImGui::Separator();

    for (auto& [characterName, timeline] : characters_) {
        if (ImGui::TreeNode(characterName.c_str())) {
            for (size_t i = 0; i < timeline.events_.size(); ++i) {
                auto& event = timeline.events_[i];
                if (ImGui::Selectable(event.name_.c_str(), selectedEvent_ == static_cast<int>(i))) {
                    selectedEvent_ = static_cast<int>(i);
                }
            }
            ImGui::TreePop();
        }
    }

    if (ImGui::Button("Add Character")) {
        AddCharacter("New Character", {});
    }
}

// グリッドの描画
void TimelineEditor::DrawGrid() {
    ImGui::Text("Timeline:");
    ImGui::Separator();

    float timelineWidth = ImGui::GetContentRegionAvail().x;
    float timelineHeight = 200.0f;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();

    const int gridSpacing = 10; // フレームごとのグリッド間隔
    float frameToPixels = timelineWidth / (frameMax_ - frameMin_);

    // グリッド線とラベル
    for (int frame = frameMin_; frame <= frameMax_; frame += gridSpacing) {
        float x = start.x + (frame - frameMin_) * frameToPixels;
        drawList->AddLine(ImVec2(x, start.y), ImVec2(x, start.y + timelineHeight), IM_COL32(200, 200, 200, 255));
        ImGui::SetCursorScreenPos(ImVec2(x, start.y + timelineHeight + 2));
        ImGui::Text("%d", frame);
    }

    drawList->AddRect(start, ImVec2(start.x + timelineWidth, start.y + timelineHeight), IM_COL32(255, 255, 255, 255));
    ImGui::Dummy(ImVec2(0, timelineHeight + 20));
}

// キャラクタータイムライン描画
void TimelineEditor::DrawCharacterTimeline(CharacterTimeline& timeline) {
    ImGui::Separator();
    for (size_t i = 0; i < timeline.events_.size(); ++i) {
        DrawEventBar(timeline.events_[i], timeline, static_cast<int>(i));
    }

    if (ImGui::Button(("Add Event##" + timeline.name_).c_str())) {
        timeline.events_.push_back({ frameMin_, frameMin_ + 10, "New Event", {} });
    }
}

// イベントバーの描画
void TimelineEditor::DrawEventBar(TimelineEvent& event, CharacterTimeline& timeline, int index) {
    ImVec2 start = ImGui::GetCursorScreenPos();
    float frameToPixels = ImGui::GetContentRegionAvail().x / (frameMax_ - frameMin_);
    float barStart = start.x + (event.startFrame_ - frameMin_) * frameToPixels;
    float barEnd = start.x + (event.endFrame_ - frameMin_) * frameToPixels;
    float barHeight = 20.0f;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(ImVec2(barStart, start.y), ImVec2(barEnd, start.y + barHeight), ImGuiColor(event.color_), 4.0f);
    drawList->AddRect(ImVec2(barStart, start.y), ImVec2(barEnd, start.y + barHeight), IM_COL32(255, 255, 255, 255));
}

// 選択したイベントのプロパティを描画
void TimelineEditor::DrawEventProperties() {
    ImGui::Text("Properties:");
    ImGui::Separator();

    if (selectedEvent_ >= 0) {
        auto& event = characters_.begin()->second.events_[selectedEvent_];
        ImGui::InputText("Name", &event.name_[0], 64);
        ImGui::SliderInt("Start Frame", &event.startFrame_, frameMin_, frameMax_);
        ImGui::SliderInt("End Frame", &event.endFrame_, frameMin_, frameMax_);
        ImGui::ColorEdit4("Color", &event.color_.x);

        for (size_t i = 0; i < event.hitboxes_.size(); ++i) {
            auto& hitbox = event.hitboxes_[i];
            ImGui::Text("Hitbox %zu", i);
            ImGui::Text("Joint", &hitbox.jointName_);
            ImGui::DragFloat3("Offset", &hitbox.offset_.x, 0.1f);
            ImGui::DragFloat3("Size", &hitbox.size_.x, 0.1f);
        }

        if (ImGui::Button("Add Hitbox")) {
            event.hitboxes_.push_back({});
        }
    }
}

// ImGuiカラー変換
unsigned int TimelineEditor::ImGuiColor(const DirectX::XMFLOAT4& color) const {
    return IM_COL32(
        static_cast<int>(color.x * 255.0f),
        static_cast<int>(color.y * 255.0f),
        static_cast<int>(color.z * 255.0f),
        static_cast<int>(color.w * 255.0f)
    );
}
