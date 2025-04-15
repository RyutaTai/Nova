#pragma once

#include <windows.h>
#include <xaudio2.h>
#include <tchar.h>
#include <string>

class WaveReader
{
public:
    WaveReader(const char* filename);
    ~WaveReader() {}

    void SetName(const char* filename); //  âπåπñºê›íË
    
    DWORD               GetAudioBytes()                 { return dwChunkSize_; }
    BYTE*               GetAudioData()                  { return pDataBuffer_; }
    WAVEFORMATEX        GetWaveForMatex()               { return wfx_; }
    UINT32              GetPlayLength()         const   { return length_; }
    float               GetPlayLengthFLOAT()    const   { return lengthFloat_; }
    const char*         GetStrFilename()        const   { return strFilename_; }
    const std::string   GetName()               const   { return name_; }

private:
    HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
    HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD bufferSize, DWORD bufferOffset);
    HRESULT FileCreate();

private:
    WAVEFORMATEX    wfx_    = {};
    XAUDIO2_BUFFER  buffer_ = {};

    const char* strFilename_;
    std::string name_ = "";

    //  Open the file
    HANDLE hFile_ = {};

    DWORD dwChunkSize_ = 0; //size of the audio buffer in bytes
    DWORD dwChunkPosition_ = 0;
   
    DWORD filetype_ = 0;
   
    BYTE*   pDataBuffer_; //buffer containing audio data
    UINT32  length_ = 0;
    float   lengthFloat_ = 0.0f;
};