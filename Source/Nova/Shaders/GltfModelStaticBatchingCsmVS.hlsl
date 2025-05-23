#include "GltfModel.hlsli"

struct CsmConstants
{
    row_major float4x4 cascadedMatrices[4];
    float4 cascadedPlaneDistances;
};
cbuffer CsmConstants : register(b3)
{
    CsmConstants csmData;
}

VS_OUT_CSM main(float4 position : POSITION, float4 normal : NORMAL, float4 tangent : TANGENT, float2 texcoord : TEXCOORD, uint instanceId : SV_INSTANCEID)
{
    VS_OUT_CSM vout;

    vout.instanceId = instanceId;
    vout.position = mul(float4(position.xyz, 1), mul(world, csmData.cascadedMatrices[instanceId]));
    
    return vout;
}
