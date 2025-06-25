#include "JudgeRhythm.h"

#include <algorithm>

#include "../Stage/Stage.h"
#include "../UI/UIManager.h"
#include "../UI/UITempo.h"
#include "../UI/UIRank.h"
#include "../UI/UIRhythmJudgment.h"
#include "../../Nova/Core/Framework.h"
#include "../../imgui/imgui.h"

void JudgeRhythm::Initialize()
{
	//	midiの生成と初期化
	//midi_ = std::make_unique<Midi>("./Resources/Audio/MIDI/fourOnTheFloor_140bpm.mid", 1.714);
	midi_ = std::make_unique<Midi>("./Resources/Audio/MIDI/fourOnTheFloor_140bpm_Full.mid", 79.760);	//	楽曲の長さ分のmidi
	midi_->Initialize();

	//	コンボ数初期化
	comboCount_ = 0;

	//	オーディオ初期化
	AudioSource* rhythmMissSE = nullptr;
	rhythmMissSE = AudioManager::Instance().LoadAudioSource("./Resources/Audio/SE/Rhythm/Clap.wav", Audio::AudioType::SENormal, "GameScene");
	rhythmMissSE->SetVolume(0.2f, false);
	rhythmMissSE->SetAudioName("RhythmMissSE");
	AudioManager::Instance().Register(rhythmMissSE);

	AudioSource* rhythmGoodSE = nullptr;
	rhythmGoodSE = AudioManager::Instance().LoadAudioSource("./Resources/Audio/SE/Rhythm/RhythmMissSE2.wav", Audio::AudioType::SENormal, "GameScene");
	rhythmGoodSE->SetVolume(0.08f, false);
	rhythmGoodSE->SetAudioName("RhythmGoodSE");
	AudioManager::Instance().Register(rhythmGoodSE);

	AudioSource* rhythmPerfectSE = nullptr;
	rhythmPerfectSE = AudioManager::Instance().LoadAudioSource("./Resources/Audio/SE/Rhythm/RhythmSE.wav", Audio::AudioType::SENormal, "GameScene");
	rhythmPerfectSE->SetVolume(0.2f, false);
	rhythmPerfectSE->SetAudioName("RhythmPerfectSE");
	AudioManager::Instance().Register(rhythmPerfectSE);

}

void JudgeRhythm::Update()
{
	//	midi更新処理
	midi_->Update(Framework::GetDoubleDeltaTime());

}

//	読んだタイミングがリズムに合っているかをテンポUIを利用して判定する
bool JudgeRhythm::Judge()
{
	//	----- 中心円に一番近い半円の番号を取得 -----
	int nearSemicircleIndex = UIManager::Instance().GetUITempo()->FindNearSemicircleIndex();

	//	----- 入力タイミングの評価(PerfectやGood)ごとの処理 -----
	debugJudgeRange_ = static_cast<float>(UIManager::Instance().GetUITempo()->GetSemicircle(nearSemicircleIndex)->GetCurrentRange());
	
	//	Perfectのとき
	if (UIManager::Instance().GetUITempo()->GetSemicircle(nearSemicircleIndex)->GetCurrentRange() < perfectRange_)		//	Perfect
	{
		//  ----- 判定文字UIを生成 -----
		UIManager::Instance().RemoveFromType(UIManager::UIType::Rhythm);
		std::unique_ptr<UIRhythmJudgment> uiRhythmPtr = std::make_unique<UIRhythmJudgment>(JudgeRhythm::JudgmentType::Perfect);
		UIRhythmJudgment* uiRhythm = uiRhythmPtr.get();			// 生ポインタを取得
		UIManager::Instance().Register(std::move(uiRhythmPtr)); // 所有権を UIManager に渡す
		uiRhythm->Initialize();
		uiRhythm->SetIsVisible(true);

		//	----- 判定フラグをtrueにしてコンボ加算 -----
		UIManager::Instance().GetUITempo()->GetSemicircle(nearSemicircleIndex)->SetIsJudged(true);
		JudgeRhythm::Instance().AddComboCount(1);

		//	----- プレイヤーの足元のオーディオスペクトラムを変化させる -----
		//	色を変化させる
		Stage::Instance().SetSpectrumColor(Stage::AudioSpectrumType::Circle, { 1.0f,1.0f,0.0f,1.0f });
		//	スケールを変化させる
		Stage::Instance().SetCircleSpectrumFovy(Stage::AudioSpectrumType::Circle, 14.0f, 0.5f);

		//	----- SEを鳴らす -----
		AudioManager::Instance().PlayAudioByName("RhythmPerfectSE", false);
		AudioManager::Instance().PlayAudioByName("RhythmMissSE", false);

		//	----- コントローラー振動 -----
		GamePad& gamePad = Input::Instance().GetGamePad();
		gamePad.SetVibration(0.4f, 0.3f, 0.25f);

		return true;

	}
	//	Goodのとき
	else if (UIManager::Instance().GetUITempo()->GetSemicircle(nearSemicircleIndex)->GetCurrentRange() < goodRange_)	//	Good
	{
		//  ----- 判定文字UIを生成 -----
		UIManager::Instance().RemoveFromType(UIManager::UIType::Rhythm);
		std::unique_ptr<UIRhythmJudgment> uiRhythmPtr = std::make_unique<UIRhythmJudgment>(JudgeRhythm::JudgmentType::Good);
		UIRhythmJudgment* uiRhythm = uiRhythmPtr.get();
		UIManager::Instance().Register(std::move(uiRhythmPtr));
		uiRhythm->Initialize();
		uiRhythm->SetIsVisible(true);

		//	----- 判定フラグをtrueにしてコンボ加算 -----
		UIManager::Instance().GetUITempo()->GetSemicircle(nearSemicircleIndex)->SetIsJudged(true);
		JudgeRhythm::Instance().AddComboCount(1);

		//	----- SEを鳴らす -----
		AudioManager::Instance().PlayAudioByName("RhythmGoodSE", false);
		AudioManager::Instance().PlayAudioByName("RhythmMissSE", false);

		//	----- コントローラー振動 -----
		GamePad& gamePad = Input::Instance().GetGamePad();
		gamePad.SetVibration(0.3f, 0.1f, 0.1f);

		return true;
	}
	//	Perfect、Good以外
	else
	{
		//  ----- 判定文字UIを生成 -----
		UIManager::Instance().RemoveFromType(UIManager::UIType::Rhythm);
		std::unique_ptr<UIRhythmJudgment> uiRhythmPtr = std::make_unique<UIRhythmJudgment>(JudgeRhythm::JudgmentType::Miss);
		UIRhythmJudgment* uiRhythm = uiRhythmPtr.get();
		UIManager::Instance().Register(std::move(uiRhythmPtr));
		uiRhythm->Initialize();
		uiRhythm->SetIsVisible(true);

		//	----- コンボ数リセット -----
		JudgeRhythm::Instance().SetComboCount(0);

		//	----- SEを鳴らす -----
		AudioManager::Instance().PlayAudioByName("RhythmMissSE", false);

		return false;
	}

	return false;
}

//	midiを見てノートオンならtrueを返す(テンポに合わせた動きをさせるために使用する)
bool JudgeRhythm::GetRhythm()
{
	//  現在ノートオンならtrueを返す
	if (midi_->IsInputNoteOn(GetCurrentMidiTime()))
		return true;

	return false;
}

//	コンボ加算
void JudgeRhythm::AddComboCount(const int& comboCount)
{
	comboCount_ += comboCount;										//	コンボ加算
	UIManager::Instance().GetUIRank()->AddRankPoint(comboCount);	//	ランクポイント加算
}

void JudgeRhythm::DrawDebug()
{
	if (ImGui::TreeNode("JudgeRhythm"))
	{
		//	----- midi -----
		ImGui::Text("----- midi -----");
		float currentMidiTimer = static_cast<float>(midi_->GetCurrentTimer());
		float maxMidiTimer = static_cast<float>(debugMaxMidiTimer_);
		debugMaxMidiTimer_ = std::max(debugMaxMidiTimer_, midi_->GetCurrentTimer());

		ImGui::DragFloat("CurrentMidiTimer", &currentMidiTimer);
		ImGui::DragFloat("MaxMidiTimer", &maxMidiTimer);
		//ImGui::DragFloat("ClosestNoteTime", &midi_->FindClosestNoteInLoop(currentMidiTimer)->time_);

		float midiDuration = static_cast<float>(midi_->GetMidiFileDurationSeconds());
		ImGui::DragFloat("MidiDuration", &midiDuration);   //   midiファイルの長さ[s]

		ImGui::DragFloat("Delta", &debugDelta_, 0.01f);                         //  入力時間と一番近いノートの差
		ImGui::DragFloat("ClosestNoteTime", &debugClosestNoteTime_, 0.01f);     //  一番近いノートの開始時間
		ImGui::DragFloat("InputTime", &debugInputTime_, 0.01f);                 //  入力時間

		//	----- 判定範囲 -----
		ImGui::Text(u8"----- 判定範囲 -----");
		ImGui::DragFloat("PerfectRange", &perfectRange_, 0.1f);
		ImGui::DragFloat("GoodRange", &goodRange_, 0.1f);

		//	----- コンボ -----
		ImGui::Text(u8"----- コンボ -----");
		ImGui::DragInt("ComboCount", &comboCount_);

		//	----- 判定を取った時の中心円からの距離 -----
		ImGui::Text(u8"----- 判定を取った時の中心円からの距離 -----");
		ImGui::DragFloat("JudgeSmicircleRange", &debugJudgeRange_);

		ImGui::TreePop();
	}
}