#pragma once

#include <DirectXMath.h>
#include <random>

//	浮動小数算術
class Mathf
{
public:
	//	線形補完
	static float Lerp(const float& start, const float& end, const float& timer)
	{
		return start * (1.0f - timer) + (end * timer);
	}
	//	指定範囲のランダム値を計算する
	static float RandomRange(const float& min, const float& max)
	{
		float randomNum = 0;
		std::random_device rnd;     //	非決定的な乱数生成器を生成
		std::mt19937 mt(rnd());     //	メルセンヌ・ツイスタの32ビット版、引数は初期シード値
		std::uniform_real_distribution<float> calcRandNum(min, max);
		randomNum = calcRandNum(mt);
		return randomNum;
	}

	static int RandomRange(const int& min, const int& max)
	{
		int randomNum = 0;
		std::random_device rnd;     //	非決定的な乱数生成器を生成
		std::mt19937 mt(rnd());     //	メルセンヌ・ツイスタの32ビット版、引数は初期シード値
		std::uniform_int_distribution<int> calcRandNum(min, max);
		randomNum = calcRandNum(mt);

		return randomNum;
	}

	//	min以上max以下のランダムな整数を返す
	static int GetRandomInt(const int& min, const int& max) 
	{
		return min + rand() % (max - min + 1);
	}

};

inline const float FloatLerp(const float& start, const float& end, const float& timer)
{
	return start + timer * (end - start);	
}

inline const float XMFLOAT3Dot(const DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3& f2)
{
	float dot = 0.0f;
	DirectX::XMStoreFloat(&dot,
		DirectX::XMVector3Dot(
			DirectX::XMLoadFloat3(&f1), DirectX::XMLoadFloat3(&f2)));
	return dot;
}

inline float LengthV3(const DirectX::XMVECTOR& v)
{
	float ret = 0.0f;
	DirectX::XMStoreFloat(&ret, DirectX::XMVector3Length(v));
	return ret;
}

inline float Length(const DirectX::XMFLOAT2& f)
{
	float ret = 0.0f;
	DirectX::XMStoreFloat(&ret, DirectX::XMVector2Length(DirectX::XMLoadFloat2(&f)));
	return ret;
}

inline float Length(const DirectX::XMFLOAT3& f)
{
	float ret = 0.0f;
	DirectX::XMStoreFloat(&ret, DirectX::XMVector3Length(DirectX::XMLoadFloat3(&f)));
	return ret;
}

inline float Length(const DirectX::XMFLOAT2& f1, const DirectX::XMFLOAT2& f2)
{
	float ret = 0.0f;
	DirectX::XMStoreFloat(
		&ret,
		DirectX::XMVector2Length(
			DirectX::XMVectorSubtract(DirectX::XMLoadFloat2(&f1),
				DirectX::XMLoadFloat2(&f2))));
	return ret;
}

inline float Length(const DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3& f2)
{
	float ret = 0.0f;
	DirectX::XMStoreFloat(
		&ret,
		DirectX::XMVector3Length(
			DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&f1),
				DirectX::XMLoadFloat3(&f2))));
	return ret;
}

inline float GetX(const DirectX::XMFLOAT3& f)
{
	float ret = 0.0f;
	ret = DirectX::XMVectorGetX(DirectX::XMLoadFloat3(&f));
}

inline DirectX::XMFLOAT2  operator+(const DirectX::XMFLOAT2& f1, const DirectX::XMFLOAT2& f2)
{
	DirectX::XMFLOAT2 ret = {};
	DirectX::XMVECTOR F1{ DirectX::XMLoadFloat2(&f1) };
	DirectX::XMVECTOR F2{ DirectX::XMLoadFloat2(&f2) };
	DirectX::XMStoreFloat2(&ret, DirectX::XMVectorAdd(F1, F2));
	return ret;
}

inline DirectX::XMFLOAT2  operator-(const DirectX::XMFLOAT2& f1, const DirectX::XMFLOAT2& f2)
{
	DirectX::XMFLOAT2 ret = {};
	DirectX::XMVECTOR F1{DirectX::XMLoadFloat2(&f1)};
	DirectX::XMVECTOR F2{DirectX::XMLoadFloat2(&f2)};
	DirectX::XMStoreFloat2(&ret,DirectX::XMVectorSubtract(F1, F2));
	return ret;
}

inline DirectX::XMFLOAT2  operator*(const DirectX::XMFLOAT2& f1, const DirectX::XMFLOAT2& f2)
{
	DirectX::XMFLOAT2 ret = {};
	DirectX::XMVECTOR F1{DirectX::XMLoadFloat2(&f1)};
	DirectX::XMVECTOR F2{DirectX::XMLoadFloat2(&f2)};
	DirectX::XMStoreFloat2(&ret, DirectX::XMVectorMultiply(F1, F2));
	return ret;
}

inline DirectX::XMFLOAT2  operator*(const DirectX::XMFLOAT2& f1, const float& f2)
{
	DirectX::XMFLOAT2 ret = {};
	DirectX::XMVECTOR F1{ DirectX::XMLoadFloat2(&f1) };
	DirectX::XMStoreFloat2(&ret, DirectX::XMVectorScale(F1, f2));
	return ret;
}

inline DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3& f2)
{
	DirectX::XMFLOAT3 ret = {};
	DirectX::XMVECTOR F1{DirectX::XMLoadFloat3(&f1)};
	DirectX::XMVECTOR F2{DirectX::XMLoadFloat3(&f2)};
	DirectX::XMStoreFloat3(&ret, DirectX::XMVectorAdd(F1, F2));
	return ret;
}

inline DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& f1, const float& f2)
{
	DirectX::XMFLOAT3 ret = {};
	ret = f1;
	ret.x += f2;
	ret.y += f2;
	ret.z += f2;

	return ret;
}

inline DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3& f2)
{
	DirectX::XMFLOAT3 ret = {};
	DirectX::XMVECTOR F1{DirectX::XMLoadFloat3(&f1)};
	DirectX::XMVECTOR F2{DirectX::XMLoadFloat3(&f2)};
	DirectX::XMStoreFloat3(&ret, DirectX::XMVectorSubtract(F1, F2));
	return ret;
}

inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3& f2)
{
	DirectX::XMFLOAT3 ret = {};
	DirectX::XMVECTOR F1{DirectX::XMLoadFloat3(&f1)};
	DirectX::XMVECTOR F2{DirectX::XMLoadFloat3(&f2)};
	DirectX::XMStoreFloat3(&ret, DirectX::XMVectorMultiply(F1, F2));
	return ret;
}

inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& f1, float f)
{
	DirectX::XMFLOAT3 ret = {};
	DirectX::XMVECTOR F1{DirectX::XMLoadFloat3(&f1)};
	DirectX::XMStoreFloat3(&ret, DirectX::XMVectorScale(F1,f));
	return ret;
}

inline DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& f)
{
	DirectX::XMFLOAT3 ret = {};
	DirectX::XMStoreFloat3(&ret, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&f)));
	return ret;
}

inline DirectX::XMFLOAT4 operator+(const DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4& f2)
{
	DirectX::XMFLOAT4 ret = {};
	DirectX::XMVECTOR F1{DirectX::XMLoadFloat4(&f1)};
	DirectX::XMVECTOR F2{DirectX::XMLoadFloat4(&f2)};
	DirectX::XMStoreFloat4(&ret, DirectX::XMVectorAdd(F1, F2));
	return ret;
}

inline DirectX::XMFLOAT4 operator-(const DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4& f2)
{
	DirectX::XMFLOAT4 ret = {};
	DirectX::XMVECTOR F1{DirectX::XMLoadFloat4(&f1)};
	DirectX::XMVECTOR F2{DirectX::XMLoadFloat4(&f2)};
	DirectX::XMStoreFloat4(&ret, DirectX::XMVectorSubtract(F1, F2));
	return ret;
}

inline DirectX::XMVECTOR operator-(const DirectX::XMVECTOR& v1,const DirectX::XMVECTOR& v2)
{
	return DirectX::XMVectorSubtract(v1, v2);
}

inline DirectX::XMFLOAT4 Max(const DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4& f2)
{
	DirectX::XMFLOAT4 ret = {};
	ret.x = f1.x > f2.x ? f1.x : f2.x;
	ret.y = f1.y > f2.y ? f1.y : f2.y;
	ret.z = f1.z > f2.z ? f1.z : f2.z;
	ret.w = f1.w > f2.w ? f1.w : f2.w;
	return ret;
}

inline DirectX::XMFLOAT4 Min(const DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4& f2)
{
	DirectX::XMFLOAT4 ret = {};
	ret.x = f1.x < f2.x ? f1.x : f2.x;
	ret.y = f1.y < f2.y ? f1.y : f2.y;
	ret.z = f1.z < f2.z ? f1.z : f2.z;
	ret.w = f1.w < f2.w ? f1.w : f2.w;
	return ret;
}

inline DirectX::XMFLOAT4 ToRadians(const DirectX::XMFLOAT4& f)
{
	return DirectX::XMFLOAT4(
		DirectX::XMConvertToRadians(f.x),
		DirectX::XMConvertToRadians(f.y),
		DirectX::XMConvertToRadians(f.z),
		DirectX::XMConvertToRadians(f.w)
	);
}

//	絶対値を求める
inline DirectX::XMFLOAT4 Absolute(const DirectX::XMFLOAT4& f)
{
	DirectX::XMFLOAT4 ret = {};
	ret.x = fabsf(f.x);
	ret.y = fabsf(f.y);
	ret.z = fabsf(f.z);
	ret.w = fabsf(f.w);
	return ret;
}
