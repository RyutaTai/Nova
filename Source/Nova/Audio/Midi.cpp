#include "Midi.h"

#include <algorithm>

Midi::Midi(const std::string& midiFilename,const double& midiFileDurationSeconds)
{
    //  midi読み込み
    _ASSERT_EXPR(midiFile_.read(midiFilename), L"midiFile loading is failed.");
	if (midiFile_.read(midiFilename) == false)
    {
        _ASSERT_EXPR(false, "loading is failed.");
    }

    //  midiファイルの長さ設定(ファイル全体の時間が取得できないため、ファイルの時間[s]を打ち込む)
    midiFileDurationSeconds_ = midiFileDurationSeconds == 0.0f ? midiFile_.getFileDurationInSeconds() : midiFileDurationSeconds;

#if 0
    //  midiファイルの長さ設定(最後のノートの終了時間が取得できる)
    midiFileDurationSeconds_ = midiFile_.getFileDurationInSeconds();
#endif

}

//	初期化処理
void Midi::Initialize()
{
    midiFile_.doTimeAnalysis(); //   マスタートラックのテンポを元に、全MIDIイベントの時間(秒)を計算
    BuildNoteOnList();
}

//	終了処理
void Midi::Finalize()
{

}

//	更新処理
void Midi::Update(const double& elpasedTime)
{
    UpdateCurrentTimer(elpasedTime);    //  タイマー更新

    SortNoteOnList();   //  ノートオンリストをソート

}

//  タイマー更新
void Midi::UpdateCurrentTimer(const double& elapsedTime)
{
    //  再生時間を更新
    currentTimer_ += elapsedTime;

    //  再生時間が MIDI の総時間を超えた場合、ループ
    if (currentTimer_ >= midiFileDurationSeconds_) 
    {
        currentTimer_ = 0.0;  // ループさせる
        //currentTimer_ -= midiFileDurationSeconds_;  // ループさせる
        ResetJudgedNotes(); //  ループ時に判定フラグをリセット
    }
}

// ノートオンリストを事前構築(初期化時に呼ぶ)
void Midi::BuildNoteOnList()
{
    for (int track = 0; track < midiFile_.getTrackCount(); ++track) 
    {
        for (int event = 0; event < midiFile_[track].size(); ++event) 
        {
            auto& midiEvent = midiFile_[track][event];
            if (midiEvent.isNoteOn()) 
            {
				notes_.emplace_back(MidiNote{ EventType::NOTE_ON, midiEvent.getKeyNumber(), midiEvent.seconds,false });
            }
        }
    }
    SortNoteOnList();
}

//  ノートオンリストをソート(更新処理で呼ぶ)
void Midi::SortNoteOnList()
{
    std::sort(notes_.begin(), notes_.end(), [](const MidiNote& a, const MidiNote& b) {
        return a.time_ < b.time_;
        });
}

//  入力タイミングから最も近いノートを返す
Midi::MidiNote* Midi::FindClosestNote(const double& inputTime)
{
    //  ノートがない場合は nullptr を返す
    if (notes_.empty())return nullptr; 

    //  最も近いノートを探索
    MidiNote* closestNote = nullptr;
    double minDelta = DBL_MAX;

    for(auto& note : notes_) 
    {
        //  判定済みのノートは無視
        //if (note.judged_) continue;

        double delta = std::abs(note.time_ - inputTime);
        if (delta < minDelta)
        {
            minDelta = delta;
            closestNote = &note;
        }
    }

    return closestNote;
}

//  入力タイミングから最も近いノートを返す(midiのループに対応)
#if 0
Midi::MidiNote* Midi::FindClosestNoteInLoop(const double& inputTime)
{
    MidiNote* closestNote = nullptr;
    double minDelta = DBL_MAX;

    int count = 0;

    for (auto& note : notes_) 
    {
        count++;
        //  判定済みノートは無視
        //if (note.judged_) continue;

        //  ノート時間をループ補正
        float noteTime = note.time_;
		if (currentTimer_ < noteTime && noteTime < currentTimer_ + midiFileDurationSeconds_)
        {
            //noteTime -= midiFileDurationSeconds_; // 巻き戻し時の補正
        }

        double delta = std::abs(inputTime - noteTime);
        if (delta < minDelta)
        {
            minDelta = delta;
            closestNote = &note;
        }
        //  デバッグ用
        if (count > 3)
        {
            _ASSERT_EXPR(minDelta < 0.4f, "debugDelta >= 0.4f.");
        }
    }

    return closestNote;
}
#else
Midi::MidiNote* Midi::FindClosestNoteInLoop(const double& time)
{
    MidiNote* closestNote = nullptr;
    double minDelta = DBL_MAX;

    double inputTime = time;

    int count = 0;

    for (auto& note : notes_)
    {
        count++;
        //  判定済みノートは無視
        //if (note.judged_) continue;

        //  ループ対応
        float noteTime = note.time_;

        double delta = std::abs(inputTime - noteTime);
        if (delta < minDelta)
        {
            minDelta = delta;
            closestNote = &note;
        }
        //  デバッグ用
        if (count > 3)
        {
            //_ASSERT_EXPR(minDelta < 0.4f, "debugDelta >= 0.4f.");
        }
    }

    return closestNote;
}
#endif

const Midi::MidiNote* Midi::GetNextNote(const float& currentTime)
{
    for (const auto& note : notes_)
    {
		if (note.time_ > currentTime && note.judged_ == false)
        {
            return &note; // 現在時刻より未来にある最初の未判定ノートを返す
        }
    }
    return nullptr; // 見つからなければ nullptr を返す
}

// 入力時間に最も近いノートの開始時間を取得
const double Midi::GetNearMidiTime(const double& inputTime)
{
    if (notes_.empty()) return -1.0f;

    auto it = std::lower_bound(notes_.begin(), notes_.end(), inputTime, [](const MidiNote& note, float time) {
        return note.time_ < time;
        });

    if (it == notes_.end()) return notes_.back().time_;
    if (it == notes_.begin()) return it->time_;

    auto prev = std::prev(it);
    return (std::abs(prev->time_ - inputTime) < std::abs(it->time_ - inputTime)) ? prev->time_ : it->time_;
}

#if 0
//	入力されたタイミングから、最も近いノートの開始時間を返す	
float Midi::GetNearMidiTime(const float& inputTime)
{
    float noteTime = 0.0f;
    float lastTimeSubtract = FLT_MAX;   //  過去に一番短かった時間の差
    float currentTimeSubtract = 0.0f;   //  Inputとnoteの時間の差(絶対値)

    // 全トラックのMIDIイベントからノートオンだけ取り出す
    for (int track = 0; track < GetMidiFile().getTrackCount(); ++track)
    {
        for (int event = 0; event < GetMidiFile()[track].size(); ++event)
        {
            if (GetMidiFile()[track][event].isNoteOn())     //  ノートオンだったら 
            {
                //  時間の差を求める
                currentTimeSubtract = std::fabs(GetMidiFile()[track][event].seconds - inputTime);

                //  一番短かった時間の差と比較
                if (currentTimeSubtract < lastTimeSubtract)
                {
                    lastTimeSubtract = currentTimeSubtract;
                    noteTime = GetMidiFile()[track][event].seconds;
                }
            }
        }
    }

    return noteTime;
}
#endif

//  引数で受け取った時間がノートオンかどうか
#if 1
bool Midi::IsNoteOnAtTime(const double& time, const float& threshold)
{
    for (const auto& note : notes_) 
    {
        if (std::abs(note.time_ - time) <= threshold)
        {
            return true;
        }
    }
    return false;
}

bool Midi::IsInputNoteOn(const double& inputTime) 
{
    return IsNoteOnAtTime(inputTime);
}

//  現在の時間がノートオンかどうか
bool Midi::IsCurrentTimeNoteOn()
{
    return IsNoteOnAtTime(currentTimer_);
}
#else
bool Midi::IsInputNoteOn(const float& time)
{
    bool isNoteOn = false;
    float lastTimeSubtract = FLT_MAX;   //  過去に一番短かった時間の差
    float currentTimeSubtract = 0.0f;   //  Inputとnoteの時間の差(絶対値)

    // 全トラックのMIDIイベントからノートオンだけ取り出す
    for (int track = 0; track < GetMidiFile().getTrackCount(); ++track)
    {
        for (int event = 0; event < GetMidiFile()[track].size(); ++event)
        {
            if (GetMidiFile()[track][event].isNoteOn())     //  ノートオンだったら 
            {
                //  時間の差を求める
                currentTimeSubtract = std::fabs(GetMidiFile()[track][event].seconds - time);

                //  一番短かった時間の差がなければ
                if (currentTimeSubtract < 0.17f)
                {
                    isNoteOn = true;
                }
                else
                {
                    isNoteOn = false;
                }

            }
        }
    }

    return isNoteOn;
}
#endif

//  ノートを追加するメソッド
void Midi::AddNote(const EventType& eventType, const int& noteNumber, const float& time)
{
    MidiNote note;
    note.eventType_     = eventType;
    note.noteNumber_    = noteNumber;
    note.time_          = time;
    notes_.emplace_back(note);

}

//  ノートの判定済みフラグをリセット
void Midi::ResetJudgedNotes()
{
    for (auto& note : notes_) 
    {
        note.judged_ = false;
    }
}

//	デバッグ描画
void Midi::DrawDebug()
{

}
