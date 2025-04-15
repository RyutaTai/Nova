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

cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b0)
{
    row_major float4x4  world;
    int                 material;
    bool                hasTangent;
    int                 skin;
    int                 pad;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4  viewProjection;
    float4              lightDirection;
    float4              cameraPosition;
    row_major float4x4  lightViewProjection;    //	Shadow
    row_major float4x4  invViewProjection;      //	Skymap
    // PROJECTION_MAPPING
    //row_major float4x4  projectionMappingTransform;
};

static const uint PRIMITIVE_MAX_JOINTS = 512;

cbuffer PRIMITIVE_JOINT_CONSTANTS : register(b2)
{
    row_major float4x4 jointMatrices[PRIMITIVE_MAX_JOINTS];
};

cbuffer EMISSIVE_CONSTANTS : register(b3)
{
    float emissiveIntensity;
}