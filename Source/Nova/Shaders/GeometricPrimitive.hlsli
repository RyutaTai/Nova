struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

cbuffer ObjectConstantBuffer : register(b0)
{
    row_major float4x4  world;
    float4              materialColor;
}
