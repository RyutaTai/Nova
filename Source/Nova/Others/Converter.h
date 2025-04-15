#pragma once

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <fbxsdk.h>
#include <string>
#include <locale>
#include <codecvt>

inline DirectX::XMFLOAT4X4 ToXMFLOAT4X4(const FbxAMatrix& fbxAMatrix)
{
	DirectX::XMFLOAT4X4 xmfloat4x4;
	for (int row = 0; row < 4; ++row)
	{
		for (int colmun = 0; colmun < 4; ++colmun)
		{
			xmfloat4x4.m[row][colmun] = static_cast<float>(fbxAMatrix[row][colmun]);
		}
	}
	return xmfloat4x4;
}

inline DirectX::XMFLOAT3 ToXMFLOAT3(const FbxDouble3& fbxDouble3)
{
	DirectX::XMFLOAT3 xmfloat3;
	xmfloat3.x = static_cast<float>(fbxDouble3[0]);
	xmfloat3.y = static_cast<float>(fbxDouble3[1]);
	xmfloat3.z = static_cast<float>(fbxDouble3[2]);
	return xmfloat3;
}

inline DirectX::XMFLOAT4 ToXMFLOAT4(const FbxDouble4& fbxDouble4)
{
	DirectX::XMFLOAT4 xmfloat4;
	xmfloat4.x = static_cast<float>(fbxDouble4[0]);
	xmfloat4.y = static_cast<float>(fbxDouble4[1]);
	xmfloat4.z = static_cast<float>(fbxDouble4[2]);
	xmfloat4.w = static_cast<float>(fbxDouble4[3]);
	return xmfloat4;
}

//	角度[θ]からラジアン(弧度法)[rad]へ変換
inline float ConvertToRadian(const float& angle)
{
	float rad = 0.0f;
	static const float PI = 3.141592653589793f;	//	円周率
	rad = angle * (PI / 180.0f);
	
	return rad;
}

//	wstringからstring
inline std::string ConvertWstringToUTF8(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string str(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], sizeNeeded, NULL, NULL);
	
	return str;
}

//	stringからwstring
inline std::wstring ConvertStringToWstring(const std::string& str)
{
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
	std::wstring wstr(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], sizeNeeded);

	return wstr;
}

inline std::string ConvertIntegerToString(int num, bool back = false)
{
    std::string str;
    if (num == 0) return "0";

    if (num < 0) {
        str += "-";
        num = -num;
    }

    while (num > 0)
    {
        int digitValue = num % 10;
        char number = digitValue + '0';
        if (back)
            str += number;
        else
            str = number + str;
        num /= 10;
    }
    return str;
}

inline bool BOOLTobool(BOOL value)
{
    return (value != FALSE);
}

inline BOOL boolToBOOL(bool value)
{
    return value ? TRUE : FALSE;
}

//	const BYTE*をstd::vector<uint8_t>に変換する
inline std::vector<uint8_t> ConvertToVector(const BYTE* data, const size_t& size) 
{
	return std::vector<uint8_t>(data, data + size);
}