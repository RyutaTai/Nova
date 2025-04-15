#include "Microphone.h"

void CALLBACK WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

//  コンストラクタ
Microphone::Microphone()
{
    // WAVEFORMATEX 構造体の初期化
    _format.wFormatTag = WAVE_FORMAT_PCM;
    _format.nChannels = 1;  // モノラル
    _format.nSamplesPerSec = /*11025;*/ 44100;  // サンプリングレート

    _format.wBitsPerSample = 16;  // サンプルあたりのビット数
    _format.nBlockAlign = _format.nChannels * (_format.wBitsPerSample / 8);
    _format.nAvgBytesPerSec = _format.nSamplesPerSec * _format.nBlockAlign;
    _deviceCount = waveInGetNumDevs();//
    
}

//  デストラクタ
Microphone::~Microphone()
{
    StopRecording();
}

//  録音
void Microphone::RecordThread(float timer)
{
    // waveInStart で録音を開始
    MMRESULT result = waveInStart(_hWaveIn);

    // timer 秒止める
    std::this_thread::sleep_for(std::chrono::seconds(static_cast<int>(timer)));

    // waveInStop で録音を停止
    result = waveInStop(_hWaveIn);
    _stoped = true;
}

//  録音開始
void Microphone::StartRecording(float timer)
{
    _started = true;
    _stoped = false;
    // バッファの初期化
    _bufferData.resize(_bufferCount, std::vector<char>(_bufferSize));
    _bufferHeaders.resize(_bufferCount);
    if (!_mic.empty())_mic.clear();
        _mic.resize(0);

    // waveInOpen でマイクをオープン
    waveInOpen(&_hWaveIn, WAVE_MAPPER, &_format, (DWORD_PTR)WaveInProc, 0, CALLBACK_FUNCTION);

    // マイクがミュートか確認
    CheckMuteStatus();

    // バッファの登録
    for (int i = 0; i < _bufferCount; ++i) {
        _bufferHeaders[i].lpData = _bufferData[i].data();
        _bufferHeaders[i].dwBufferLength = _bufferSize;
        _bufferHeaders[i].dwFlags = 0;
        _bufferHeaders[i].dwLoops = 0;

        waveInPrepareHeader(_hWaveIn, &_bufferHeaders[i], sizeof(WAVEHDR));

        waveInAddBuffer(_hWaveIn, &_bufferHeaders[i], sizeof(WAVEHDR));
    }
    
    _recordingThread = std::thread(&Microphone::RecordThread, this, timer);
}

//  録音終了
void Microphone::StopRecording() {
    // スレッド待ち
    if (_recordingThread.joinable()) {
        _recordingThread.join();
    }

    // バッファの解放
    for (int i = 0; i < _bufferCount; ++i) {
        waveInUnprepareHeader(_hWaveIn, &_bufferHeaders[i], sizeof(WAVEHDR));
    }

    // waveInClose でマイクをクローズ
    MMRESULT result = waveInClose(_hWaveIn);
    _started = false;
    _isRecording = false;
    _timer = 0;
}

void CALLBACK WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    // dwParam1 には録音データが含まれている
    if (uMsg == WIM_DATA) {
        WAVEHDR* pWaveHdr = reinterpret_cast<WAVEHDR*>(dwParam1);
        Microphone::Instance().SetIsRecording(false);
        // pWaveHdr->lpData が録音データへのポインタ
        // pWaveHdr->dwBufferLength がバッファのサイズ

        // 最初のサンプルの振幅を取得してデシベルに変換して表示
        int16_t* data = reinterpret_cast<int16_t*>(pWaveHdr->lpData);
        size_t numSamples = pWaveHdr->dwBufferLength / sizeof(int16_t);

        double rms = 0.0;
        for (size_t i = 0; i < numSamples; ++i) {
            rms += static_cast<double>(data[i] * data[i]);
        }
        rms = sqrt(rms / numSamples);

        float decibels = 20.0 * log10(rms);
        if (decibels < 0)decibels = 0.0;

        Microphone::Param db;
        db._db = decibels;


        Microphone::Instance().Register(db, Microphone::Instance()._mic);
        Microphone::Instance().SetIsRecording(true);
        Microphone::Instance().SetTimer(Microphone::Instance().GetTimer() + 1);

        // 新しいバッファを指定して再度録音を開始する
        MMRESULT result = waveInAddBuffer(hwi, pWaveHdr, sizeof(WAVEHDR));
        
    }
}

// ミュートかどうか確認
bool Microphone::CheckMuteStatus() {
    CoInitialize(nullptr);

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioEndpointVolume* pEndpointVolume = nullptr;

    // デバイス列挙器を作成
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pEnumerator);
    if (SUCCEEDED(hr)) {
        // デフォルトの録音デバイス（マイク）を取得
        hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
        if (SUCCEEDED(hr)) {
            // オーディオエンドポイントボリューム インターフェースを取得
            hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (LPVOID*)&pEndpointVolume);
            if (SUCCEEDED(hr)) {
                // マイクのミュート状態を格納する変数
                BOOL muteState;

                // GetMuteメソッドにBOOLポインタを渡してマイクのミュート状態を取得
                hr = pEndpointVolume->GetMute(&muteState);
                if (SUCCEEDED(hr)) {
                    (muteState ? pEndpointVolume->SetMute(FALSE, nullptr) : _muted = false);
                }
                //FAILED(hr, "failed");

                // インターフェースの解放
                pEndpointVolume->Release();
            }
            //FAILED(hr, "failed");

            // デバイスの解放
            pDevice->Release();
        }
        //FAILED(hr, "failed");

        // デバイス列挙器の解放
        pEnumerator->Release();
    }
    //FAILED(hr, "failed");

    CoUninitialize();

    return hr == S_OK; // 正常に終了したかどうかを返す
}

// ソート (昇順)
auto sort_func = [](Microphone::Param& lhs, Microphone::Param& rhs)-> bool {
    return lhs._db < rhs._db;
};

// ソート(デシベルの最大値を取得)
float Microphone::Sort()
{
    std::sort(_mic.begin(), _mic.end(), sort_func);
    return _mic.back()._db;
}
