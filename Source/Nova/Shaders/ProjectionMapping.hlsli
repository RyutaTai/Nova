//  円形のオーディオスペクトラム
cbuffer CircleSpectrumConstant : register(b4)
{
    row_major float4x4 circleSpectrumTransform;
};

//  波形のオーディオスペクトラム
cbuffer WaveformSpectrumConstant : register(b5)
{
    row_major float4x4 waveformSpectrumTransform;
};