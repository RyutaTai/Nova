#include "StaticMesh.hlsli"

#include "SceneConstantBuffer.hlsli"

VS_OUT main(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD)
{
    VS_OUT vout;
    vout.position = mul(position, mul(world, viewProjection));
 
    vout.worldPosition = mul(position, world);
    normal.w = 0;
    vout.worldNormal = normalize(mul(normal, world));
    
    vout.color = materialColor;
    vout.texcoord = texcoord;
    
    return vout;

}
