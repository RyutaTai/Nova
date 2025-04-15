#pragma once

#include "GamePad.h"

#include <Windows.h>
#include <mmsystem.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <Mmdeviceapi.h>
#include <Endpointvolume.h>
#include <thread>
#include <future>
#include <chrono>

#pragma comment(lib, "winmm.lib")

class Microphone
{
public: //  構造体
    struct Param {
        double _db;
    };

public:
    Microphone();
    ~Microphone();

public:
    //  インスタンス取得
    static Microphone& Instance() {
        static Microphone instance_;
        return instance_;
    }

    // 音声入力 (second)
    void StartRecording(float timer);

    // 録音終了　バッファの開放などをする
    void StopRecording();

    //  登録
    void Register(Param p, std::vector<Param> n) { _mic.emplace_back(p); }

    // ソート(デシベルの最大値を取得)
    float Sort();

    //  デシベル取得
    float GetDecibel() { return _mic.at(_timer-1)._db; }

    //  タイマー取得
    float GetTimer() { return _timer; }

    //  タイマー設定
    void SetTimer(float timer) { _timer = timer; }

    //  録音中か
    bool GetOnRecording() { return _isRecording; }

    //  録音フラグ設定
    void SetIsRecording(bool recording) { _isRecording = recording; }

    //  スレッドが終了しているか
    bool ThreadStoped() { return _stoped; }

    //  ?
    bool IsStart() { return _started; }

private:
    // ミュートかどうか確認
    bool CheckMuteStatus();
    // 録音
    void RecordThread(float timer);

public:
    std::vector<Param> _mic; // 全体

private:
    WAVEFORMATEX    _format;
    HWAVEIN         _hWaveIn;
    UINT            _deviceCount;
    float           _timer          = 0;
    bool            _muted          = true;     //  ミュート
    bool            _isRecording    = false;    //  録音中か
    bool            _started        = false;    //  録音化したらtrue もう一度実行されるのを防ぐ
    bool            _stoped         = false;    //  スレッドが終了しているか？

    const int _bufferSize = 4096;               //  
    const int _bufferCount = 3;                 //  

    std::vector<std::vector<char>>  _bufferData;        //  
    std::vector<WAVEHDR>            _bufferHeaders;     //  
    std::thread                     _recordingThread;   //  録音スレッド

};
