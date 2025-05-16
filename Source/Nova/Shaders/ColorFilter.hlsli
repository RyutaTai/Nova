//  カラーフィルター用定数バッファ
cbuffer ColorFilter : register(b7)
{
    float hueShift;     //  色相調整
    float saturation;   //  彩度調整
    float brightness;   //  明度調整
    float dummy;
};

#include "FilterFunctions.hlsli"