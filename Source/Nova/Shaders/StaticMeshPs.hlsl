#include "StaticMesh.hlsli"

#include "SceneConstantBuffer.hlsli"

Texture2D colorMap : register(t0);
Texture2D normalMap : register(t1);
SamplerState pointSamplerState : register(s0);
SamplerState linearSamplerState : register(s1);
SamplerState anisotropicSamplerState : register(s2);

//  法線マッピング表現
float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = colorMap.Sample(anisotropicSamplerState, pin.texcoord);
    float alpha = color.a;
    float3 N = normalize(pin.worldNormal.xyz);
   
    float3 T = float3(1.0001, 0, 0);
    float3 B = normalize(cross(N, T));
    T = normalize(cross(B, N));
    
    float4 normal = normalMap.Sample(linearSamplerState, pin.texcoord);
    normal = (normal * 2.0) - 1.0;
    normal.w = 0;
    N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
    
    float3 L = normalize(-lightDirection.xyz);
    float3 diffuse = color.rgb * max(0, dot(N, L));
    
    float3 V = normalize(cameraPosition.xyz - pin.worldPosition.xyz);
    float specular = pow(max(0, dot(N, normalize(V + L))), 128);
    
    return float4(diffuse + specular, alpha) * pin.color;
}
