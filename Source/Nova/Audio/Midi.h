#pragma once

#include <iostream>
#include <vector>

#include "../../../External/MidiFile/include/MidiFile.h"

//  MIDIトラッククラス
class Midi 
{
public:
    enum class EventType    //  MIDIイベントの種類
    {
        NOTE_ON,
        NOTE_OFF
    };

    struct MidiNote         //  MIDIノートを表す構造体
    {
        EventType   eventType_ = {};
        int         noteNumber_ = 0;
        double       time_ = 0.0f;
        //float       time_ = 0.0f;
        bool        judged_ = false; // 判定済みフラグを追加
    };

public:
	Midi(const std::string& midiFilename, const double& midiFileDurationSeconds = 0.0f/*midiファイルの長さ[s]*/);
    ~Midi() {}

    //  ノートを追加する関数
    void AddNote(const EventType& eventType, const int& noteNumber, const float& time);

    //  ノートを取得する関数
    std::vector<MidiNote> GetNotes() const { return notes_; }
    Midi::MidiNote*       FindClosestNote(const double& inputTime);
    Midi::MidiNote*       FindClosestNoteInLoop(const double& time);
    const Midi::MidiNote* GetNextNote(const float& currentTime);

    void	Initialize();								    //	初期化処理
    void	Update(const double& elapsedTime);				//	更新処理
    //void	Update(const float& elapsedTime);				//	更新処理
    void	Finalize();									    //	終了処理
    void	DrawDebug();								    //	デバッグ描画

    void    UpdateCurrentTimer(const double& elapsedTime);  //  タイマー更新処理
    //void UpdateCurrentTimer(const float& elapsedTime);    //  タイマー更新処理

    bool IsNoteOnAtTime(const double& time, const float& threshold = 0.17f);
    bool IsInputNoteOn(const double& inputTime);            //  入力時間がノートオンかどうか
    bool IsCurrentTimeNoteOn();                             //  現在の時間がノートオンかどうか

    void    BuildNoteOnList();                              //   ノートオンリストを事前に構築
    void    SortNoteOnList();                               //   ノートオンリストをソート
    const double	GetNearMidiTime(const double& inputTime);       //   入力されたタイミングから近いノートを判定	

    smf::MidiFile&  GetMidiFile()                   { return midiFile_; }	                //	midiファイル取得
    const double     GetMidiFileDurationSeconds()    { return midiFileDurationSeconds_; }    //  midiファイル全体の長さ取得
    const double     GetCurrentTimer()const { return currentTimer_; }
    //float           GetCurrentTimer() { return currentTimer_; }

    void ResetJudgedNotes();    //  ノートの判定済みフラグをリセット

private:
    double                   midiFileDurationSeconds_ = 0.0;    //  midiファイル全体の長さ(時間[s])
    double                   currentTimer_ = 0.0;              //  現在の時間[s]
    //float                   currentTimer_ = 0.0f;             //  現在の時間
    smf::MidiFile	        midiFile_ = {};                     //  midiファイル
    std::vector<MidiNote>   notes_;                             //  midiデータ内のノート

};

////  リズム判定関数
//void PerformRhythmAnalysis(const std::vector<Midi::MidiNote>& notes)
//{
//    //  リズム判定の実装（ここでは単純にノートの時間を出力するだけ）
//    for (const auto& note : notes)
//    {
//        std::cout << "Note Number: " << note.noteNumber_ << ", ";
//        std::cout << "Event Type: " << (note.eventType_ == Midi::EventType::NOTE_ON ? "Note On" : "Note Off") << ", ";
//        std::cout << "Time: "       << note.time_ << " seconds" << std::endl;
//    }
//}

////  メイン関数
//int main() {
//    //  MIDIトラックを作成
//    Midi track;
//
//    //  仮のノートを追加（実際のMIDI解析はここでは行わない）
//    track.AddNote(Midi::EventType::NOTE_ON, 60, 0.0);
//    track.AddNote(Midi::EventType::NOTE_OFF, 60, 1.0);
//
//    //  リズム判定を実行
//    PerformRhythmAnalysis(track.GetNotes());
//
//    return 0;
//}


