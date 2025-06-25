#include "WaveRead.h"

#include <filesystem>

#include "../Others/Misc.h"

//  WAVEファイルチャンクIDのマクロ定義(リトルエンディアン)
//  Windows環境ではリトルエンディアンでFourCCを読み込むため、バイト順が逆になっている。
#define fourccRIFF 'FFIR'   //  RIFFチャンクID
#define fourccDATA 'atad'   //  dataチャンクID
#define fourccFMT  ' tmf'   //  fmt チャンクID(スペースに注意する)
#define fourccWAVE 'EVAW'   //  WAVEファイルタイプID
#define fourccXWMA 'AMWX'   //  XWMAファイルタイプID
#define fourccDPDS 'sdpd'   //  dpdsチャンクID(XWMA用)

WaveReader::WaveReader(const char* filename) : strFilename_(filename)
{
    //  ファイルを作成（開く）
    HRESULT hr = FileCreate();
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
   
    //  RIFFチャンクを探す
    hr = FindChunk(hFile_, fourccRIFF, dwChunkSize_, dwChunkPosition_);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //  RIFFチャンクのデータ(ファイルタイプ)を読み込む
    //  'WAVE'または'XWMA'以外サポートしていない
    hr = ReadChunkData(hFile_, &filetype_, sizeof(DWORD), dwChunkPosition_);
    if (filetype_ != fourccWAVE && filetype_ != fourccXWMA)
    {
        _ASSERT_EXPR(false, L"Unsupported file type. Should be WAVE or XWMA.");
    }

    //  fmt チャンクを探す(音声フォーマット情報)
    hr = FindChunk(hFile_, fourccFMT, dwChunkSize_, dwChunkPosition_);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //  fmt チャンクのデータを読み込み、WAVEFORMATEX構造体に格納
    hr = ReadChunkData(hFile_, &wfx_, dwChunkSize_, dwChunkPosition_);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //  data チャンクを探す(実際の音声データ)
    hr = FindChunk(hFile_, fourccDATA, dwChunkSize_, dwChunkPosition_);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //  dataチャンクのサイズに合わせてデータバッファを確保
    pDataBuffer_ = new BYTE[dwChunkSize_];

    //  dataチャンクのデータをバッファに読み込む
    hr = ReadChunkData(hFile_, pDataBuffer_, dwChunkSize_, dwChunkPosition_);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //  WAVEフォーマットタグに基づいてWAVEFORMATEX構造体のcbSizeを設定
    //  このcbSizeは、拡張情報が存在する場合にそのサイズを示す
    switch (wfx_.wFormatTag)
    {
    case WAVE_FORMAT_PCM:           //  リニアPCM形式
    case WAVE_FORMAT_IEEE_FLOAT:    //  IEEE float形式
        wfx_.cbSize = 0;            //  拡張情報はない
        break;

    case WAVE_FORMAT_EXTENSIBLE:    //  拡張可能なフォーマット
        //  WAVEFORMATEXTENSIBLE構造体全体のサイズからWAVEFORMATEX部分を引いたサイズ
        wfx_.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
        break;

    default:                                //  その他のフォーマット
        wfx_.cbSize = sizeof(WAVEFORMATEX); //  基本的なWAVEFORMATEX構造体のサイズを設定
        break;
    }

    //  音声データの再生時間を計算(秒単位)
    //  lengthFloat_        : 浮動小数点数でより正確な秒数
    //  dwChunkSize_        : データチャンクのバイト数
    //  wfx_.nSamplesPerSec : サンプリングレート(1秒あたりのサンプル数)
    //  4.0f                : 1サンプルあたりのバイト数 (例: 16bitステレオの場合、2バイト/サンプル * 2チャンネル = 4バイト)
    lengthFloat_ = (float)dwChunkSize_ / (wfx_.nSamplesPerSec * 4.0f);
    //  length_: 整数で概算の秒数
    length_ = dwChunkSize_ / (wfx_.nSamplesPerSec * 4);

    //  ファイル名から名前を設定
    SetName(filename);

}

//  指定されたFourCCを持つチャンクをファイル内で検索し、そのサイズと位置を取得する
//  hFile               : 検索対象のファイルハンドル
//  fourcc              : 検索するチャンクのFourCC
//  dwChunkSize         : 見つかったチャンクのデータサイズが格納される参照変数
//  dwChunkDataPosition : 見つかったチャンクのデータ開始位置が格納される参照変数
HRESULT WaveReader::FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
    HRESULT hr = S_OK;
    //  ファイルポインタをファイルの先頭に設定
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkType;          //  読み込んだチャンクのFourCC
    DWORD dwChunkDataSize;      //  読み込んだチャンクのデータサイズ
    DWORD dwRIFFDataSize = 0;   //  RIFFチャンクの全体のデータサイズ
    DWORD dwFileType;           //  RIFFチャンク内のファイルタイプ (例: 'WAVE')
    DWORD bytesRead = 0;        //  これまでに読み込んだバイト数 (ループの終了条件に使用されるが、現在は未使用？)
    DWORD dwOffset = 0;         //  現在のファイルポインタのオフセット

    //  ファイル終端に到達するか、エラーが発生するまでチャンクを読み進める
    while (hr == S_OK)
    {
        DWORD dwRead;   //  ReadFileで実際に読み込まれたバイト数

        //  チャンクタイプ(FourCC)を読み込む
        if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        //  チャンクデータサイズを読み込む
        if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        //  読み込んだチャンクタイプに応じた処理
        switch (dwChunkType)
        {
        case fourccRIFF:    //  RIFFチャンクの場合
            dwRIFFDataSize = dwChunkDataSize;       //  RIFFチャンクのデータサイズを保存
            dwChunkDataSize = 4;                    //  RIFFチャンク内のファイルタイプ(WAVEなど)のサイズは4バイト
            //  ファイルタイプを読み込む
            if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());
            break;

        default:    //  その他のチャンクの場合
            //  チャンクデータサイズ分だけファイルポインタを移動し、次のチャンクへスキップ
            if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                return HRESULT_FROM_WIN32(GetLastError());
        }

        //  現在のオフセットを更新(FourCCとデータサイズの合計8バイト分)
        dwOffset += sizeof(DWORD) * 2;

        //  検索対象のFourCCと一致したら
        if (dwChunkType == fourcc)
        {
            dwChunkSize         = dwChunkDataSize;  //  見つかったチャンクのサイズを格納
            dwChunkDataPosition = dwOffset;         //  見つかったチャンクのデータ開始位置を格納
            return S_OK;                            //  成功を返して終了する
        }

        //  次のチャンクの位置へオフセットを更新
        dwOffset += dwChunkDataSize;

        //  チャンクが見つからずにRIFFデータの終わりに到達
        if (bytesRead >= dwRIFFDataSize) return S_FALSE;

    }

    return S_OK;

}

//  ファイルから指定されたデータを読み込む
//  hFile       : 読み込み対象のファイルハンドル
//  buffer      : 読み込んだデータを格納するバッファ
//  bufferSize  : 読み込むバイト数
//  bufferOffset: ファイルのどこから読み込むかのオフセット
HRESULT WaveReader::ReadChunkData(HANDLE hFile, void* buffer, DWORD bufferSize, DWORD bufferOffset)
{
    HRESULT hr = S_OK;
    //  ファイルポインタを指定されたオフセットに設定
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferOffset, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());
    
    DWORD dwRead;   //  ReadFileで実際に読み込まれたバイト数
    //  データを読み込む
    if (0 == ReadFile(hFile, buffer, bufferSize, &dwRead, NULL))
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

//  ファイルを作成し(開き)、ファイルハンドルを取得する
//  strFilename_: コンストラクタで渡されたファイル名
HRESULT WaveReader::FileCreate()
{
    hFile_ = CreateFileA(
        strFilename_,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    //  ファイルが開けなかった
    if (INVALID_HANDLE_VALUE == hFile_)
        return HRESULT_FROM_WIN32(GetLastError());

    //  ファイルポインタをファイルの先頭に設定
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile_, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}

//	音源名を設定する
void WaveReader::SetName(const char* filename)
{
    std::filesystem::path filepath = filename;
    name_ = filepath.filename().string();       //  パスからファイル名部分のみを抽出し、name_に格納
}