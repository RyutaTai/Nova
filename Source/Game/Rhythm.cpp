#include "Rhythm.h"

#include "UI/UIManager.h"
#include "UI/UITempo.h"
#include "UI/UIRank.h"
#include "../Nova/Core/Framework.h"
#include "../../imgui/imgui.h"

#include <algorithm>

void Rhythm::Initialize()
{
	//	midiの生成と初期化
	//midi_ = std::make_unique<Midi>("./Resources/Audio/MIDI/fourOnTheFloor_140bpm.mid", 1.714);
	midi_ = std::make_unique<Midi>("./Resources/Audio/MIDI/fourOnTheFloor_140bpm_Full.mid", 79.760);	//	楽曲の長さ分のmidi
	midi_->Initialize();

	//	コンボ数初期化
	comboCount_ = 0;

}

void Rhythm::Update()
{
	//	midi更新処理
	midi_->Update(Framework::GetDoubleDeltaTime());

}

//	midiを見てノートオンならtrueを返す(テンポに合わせた動きをさせるために使用する)
bool Rhythm::GetRhythm()
{
	//  現在ノートオンならtrueを返す
	if (midi_->IsInputNoteOn(GetCurrentMidiTime()))
		return true;

	return false;
}

//	コンボ加算
void Rhythm::AddComboCount(const int& comboCount)
{
	comboCount_ += comboCount;										//	コンボ加算
	UIManager::Instance().GetUIRank()->AddRankPoint(comboCount);	//	ランクポイント加算
}

void Rhythm::DrawDebug()
{
	if (ImGui::TreeNode("Rhythm"))
	{
		float currentMidiTimer = static_cast<float>(midi_->GetCurrentTimer());
		float maxMidiTimer = debugMaxMidiTimer_;
		debugMaxMidiTimer_ = max(debugMaxMidiTimer_, midi_->GetCurrentTimer());

		ImGui::DragFloat("CurrentMidiTimer", &currentMidiTimer);
		ImGui::DragFloat("MaxMidiTimer", &maxMidiTimer);
		//ImGui::DragFloat("ClosestNoteTime", &midi_->FindClosestNoteInLoop(currentMidiTimer)->time_);

		float midiDuration = midi_->GetMidiFileDurationSeconds();
		ImGui::DragFloat("MidiDuration", &midiDuration);   //   midiファイルの長さ[s]

		ImGui::DragFloat("Delta", &debugDelta_, 0.01f);                         //  入力時間と一番近いノートの差
		ImGui::DragFloat("ClosestNoteTime", &debugClosestNoteTime_, 0.01f);     //  一番近いノートの開始時間
		ImGui::DragFloat("InputTime", &debugInputTime_, 0.01f);                 //  入力時間

		ImGui::Text(u8"----- コンボ -----");
		ImGui::DragInt("ComboCount", &comboCount_);

		ImGui::TreePop();
	}
}