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

    void	Initialize();								    //	初期化処理
    void	Update(const double& elapsedTime);				//	更新処理
    //void	Update(const float& elapsedTime);				//	更新処理
    void	Finalize();									    //	終了処理
    void	DrawDebug();								    //	デバッグ描画

    //  ノートを追加する関数
    void AddNote(const EventType& eventType, const int& noteNumber, const float& time);

    //  ノートを取得する関数
    std::vector<MidiNote> GetNotes() const { return notes_; }
    Midi::MidiNote*       FindClosestNote(const double& inputTime);
    Midi::MidiNote*       FindClosestNoteInLoop(const double& inputTime);
    const Midi::MidiNote* GetNextNote(const float& currentTime);

    //  タイマー更新処理
    void    UpdateCurrentTimer(const double& elapsedTime);  
    //void UpdateCurrentTimer(const float& elapsedTime);

    //  ノートオンかどうか判定する関数
    bool IsNoteOnAtTime(const double& time, const float& threshold = 0.17f);
    bool IsInputNoteOn(const double& inputTime);            //  入力時間がノートオンかどうか
    bool IsCurrentTimeNoteOn();                             //  現在の時間がノートオンかどうか

    //  ノートオンリスト
    void    BuildNoteOnList();                              //   ノートオンリストを事前に構築
    void    SortNoteOnList();                               //   ノートオンリストをソート
    
    const double	GetNearMidiTime(const double& inputTime);       //   入力されたタイミングから近いノートを判定	

    smf::MidiFile&      GetMidiFile()                   { return midiFile_; }	                //	midiファイル取得
    const double        GetMidiFileDurationSeconds()    { return midiFileDurationSeconds_; }    //  midiファイル全体の長さ取得
    const double        GetCurrentTimer()const { return currentTimer_; }
    //float           GetCurrentTimer() { return currentTimer_; }

    void ResetJudgedNotes();    //  ノートの判定済みフラグをリセット

private:
    double                  midiFileDurationSeconds_ = 0.0;    //  midiファイル全体の長さ(時間[s])
    double                  currentTimer_ = 0.0;              //  現在の時間[s]
    //float                   currentTimer_ = 0.0f;             //  現在の時間
    smf::MidiFile	        midiFile_ = {};                     //  midiファイル
    std::vector<MidiNote>   notes_;                             //  midiデータ内のノート

};
