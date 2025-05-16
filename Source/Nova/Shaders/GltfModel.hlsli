struct VS_IN
{
    float4  position    : POSITION;
    float4  normal      : NORMAL;
    float4  tangent     : TANGENT;
    float2  texcoord    : TEXCOORD;
    uint4   joints [2]  : JOINTS;
    float4  weights[2]  : WEIGHTS;
};

struct VS_OUT
{
    float4  position     : SV_POSITION;
    float4  wPosition    : POSITION;
    float4  wNormal      : NORMAL;
    float4  wTangent     : TANGENT;
    float2  texcoord     : TEXCOORD;
};

cbuffer PrimitiveConstantBuffer : register(b0)
{
    row_major float4x4  world;
    int                 material;
    bool                hasTangent;
    int                 skin;
    int                 pad;
};

static const uint PRIMITIVE_MAX_JOINTS = 512;

cbuffer PrimitiveJointConstants : register(b2)
{
    row_major float4x4 jointMatrices[PRIMITIVE_MAX_JOINTS];
};

cbuffer EmissiveConstants : register(b3)
{
    float emissiveIntensity;
}

//  シャドウマップ
struct VS_OUT_CSM
{
    float4  position    : SV_POSITION;
    uint    instanceId  : INSTANCEID;   //  何回目のインスタンス描画か
};
struct GS_OUTPUT_CSM
{
    float4  position                : SV_POSITION;
    uint    renderTargetArrayIndex  : SV_RENDERTARGETARRAYINDEX;    //  何枚目のレンダーターゲット
};