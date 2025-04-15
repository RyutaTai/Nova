#include "WaveRead.h"

#include <filesystem>

#include "../Others/Misc.h"

#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT  'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

WaveReader::WaveReader(const char* filename) : strFilename_(filename)
{
    HRESULT hr = FileCreate();
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
   
    //check the file type, should be fourccWAVE or 'XWMA'
    hr = FindChunk(hFile_, fourccRIFF, dwChunkSize_, dwChunkPosition_);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    hr = ReadChunkData(hFile_, &filetype_, sizeof(DWORD), dwChunkPosition_);

    hr = FindChunk(hFile_, fourccFMT, dwChunkSize_, dwChunkPosition_);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    hr = ReadChunkData(hFile_, &wfx_, dwChunkSize_, dwChunkPosition_);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    hr = FindChunk(hFile_, fourccDATA, dwChunkSize_, dwChunkPosition_);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    pDataBuffer_ = new BYTE[dwChunkSize_];
    hr = ReadChunkData(hFile_, pDataBuffer_, dwChunkSize_, dwChunkPosition_);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    switch (wfx_.wFormatTag)
    {
    case WAVE_FORMAT_PCM:
    case WAVE_FORMAT_IEEE_FLOAT:
        wfx_.cbSize = 0;
        break;

    case WAVE_FORMAT_EXTENSIBLE:
        wfx_.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
        break;

    default:
        wfx_.cbSize = sizeof(WAVEFORMATEX);
        break;
    }

    lengthFloat_ = (float)dwChunkSize_ / (wfx_.nSamplesPerSec * 4.0f);
    length_ = dwChunkSize_ / (wfx_.nSamplesPerSec * 4);

    SetName(filename);

}

HRESULT WaveReader::FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
    HRESULT hr = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while (hr == S_OK)
    {
        DWORD dwRead;
        if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        switch (dwChunkType)
        {
        case fourccRIFF:
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());
            break;

        default:
            if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                return HRESULT_FROM_WIN32(GetLastError());
        }

        dwOffset += sizeof(DWORD) * 2;

        if (dwChunkType == fourcc)
        {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;

        if (bytesRead >= dwRIFFDataSize) return S_FALSE;

    }

    return S_OK;

}

HRESULT WaveReader::ReadChunkData(HANDLE hFile, void* buffer, DWORD bufferSize, DWORD bufferOffset)
{
    HRESULT hr = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferOffset, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());
    DWORD dwRead;
    if (0 == ReadFile(hFile, buffer, bufferSize, &dwRead, NULL))
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}


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

    if (INVALID_HANDLE_VALUE == hFile_)
        return HRESULT_FROM_WIN32(GetLastError());

    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile_, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

}

//	âπåπñºê›íË
void WaveReader::SetName(const char* filename)
{
    std::filesystem::path filepath = filename;
    name_ = filepath.filename().string();
}