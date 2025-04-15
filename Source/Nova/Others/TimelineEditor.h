#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <DirectXMath.h>

//  ジョイント情報
struct Joint 
{
    std::string         name_ = {};     //  ジョイント名
    DirectX::XMFLOAT3   position_ = {}; //  ワールド座標でのジョイントの位置
};

//  ヒットボックスデータ
struct Hitbox 
{
    std::string         jointName_ = {};                //  ジョイント名 (空の場合ジョイントなし)
    DirectX::XMFLOAT3   offset_ = {};                   //  ジョイントに対するオフセット
    DirectX::XMFLOAT3   size_ = { 1.0f, 1.0f, 1.0f };   //  判定の大きさ
};

//  アニメーションイベントデータ
struct TimelineEvent 
{
    int startFrame_ = -1;                             // 開始フレーム
    int endFrame_ = -1;                               // 終了フレーム
    std::string name_ = {};                           // イベント名
    std::vector<Hitbox> hitboxes_;                    // ヒットボックスリスト
    DirectX::XMFLOAT4 color_ = { 0.5f, 0.5f, 1.0f, 1.0f }; // イベントの色
};

// キャラクタータイムラインデータ
struct CharacterTimeline {
    std::string name_;                                // キャラクター名
    std::unordered_map<std::string, Joint> joints_;  // ジョイント名とその情報
    std::vector<TimelineEvent> events_;              // タイムラインイベント
};

//  タイムラインエディタ
class TimelineEditor
{
public:
    TimelineEditor(const int& frameMin, const int& frameMax);

    TimelineEditor() = default;

    //  キャラクター管理
    void AddCharacter(const std::string& characterName, const std::unordered_map<std::string, Joint>& joints);
    void RemoveCharacter(const std::string& characterName);

    //  描画
    void DrawTimeline();

private:
    //  内部関数
    void DrawCharacterList();
    void DrawGrid();
    void DrawCharacterTimeline(CharacterTimeline& timeline);
    void DrawEventBar(TimelineEvent& event, CharacterTimeline& timeline, int index);
    void DrawEventProperties();
    unsigned int ImGuiColor(const DirectX::XMFLOAT4& color) const;

private:
    int frameMin_;  //  最小フレーム
    int frameMax_;  //  最大フレーム
    int selectedEvent_ = -1;                         // 選択中のイベント
    std::unordered_map<std::string, CharacterTimeline> characters_; // キャラクターごとのタイムライン

};
