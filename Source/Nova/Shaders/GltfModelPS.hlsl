#include "GltfModel.hlsli"

#include "BidirectionalReflectanceDistributionFunction.hlsli"

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4

Texture2D<float4> materialTextures[5] : register(t1);

struct TextureInfo
{
    int index;
    int texcoord;
};

struct NormalTextureInfo
{
    int     index;
    int     texcoord;
    float   scale;
};

struct OcclusionTextureInfo
{
    int     index;
    int     texcoord;
    float   strength;
};

struct PbrMetallicRoughness
{
    float4      baseColorFactor;
    TextureInfo baseColorTexture;
    float       metallicFactor;
    float       roughnessFactor;
    TextureInfo metallicRoughnessTexture;
};

struct MaterialConstants
{
    float3  emissiveFactor;
    int     alphaMode;  //  "OPAQUE" : 0,"MASK" : 1,"BLEND" : 2
    float   alphaCutOff;
    bool    doubleSided;
    
    PbrMetallicRoughness pbrMetallicRoughness;
    
    NormalTextureInfo normalTexture;
    OcclusionTextureInfo occlusionTexture;
    TextureInfo emissiveTexture;
};
StructuredBuffer<MaterialConstants> materials : register(t0);

float4 main(VS_OUT pin) : SV_TARGET
{
#if 1
    const float GAMMA = 2.2f;
    
    const MaterialConstants materialConstant = materials[material];
    
    float4 baseColorFactor = materialConstant.pbrMetallicRoughness.baseColorFactor;
    
    const int baseColorTexture = materialConstant.pbrMetallicRoughness.baseColorTexture.index;
    if (baseColorTexture > -1)
    {
        float4 sampled = materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        baseColorFactor *= sampled;
    }
    
    
    //return baseColorFactor;
 
    
    
    float3 emissiveFactor = materialConstant.emissiveFactor;
    const int emissiveTexture = materialConstant.emissiveTexture.index;
    if (emissiveTexture > -1)
    {
        float4 sampled = materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emissiveFactor *= sampled.rgb;
    }
    
    float roughnessFactor = materialConstant.pbrMetallicRoughness.roughnessFactor;
    float metallicFactor = materialConstant.pbrMetallicRoughness.metallicFactor;
    const int metallicRoughnessTexture = materialConstant.pbrMetallicRoughness.metallicRoughnessTexture.index;
    if (metallicRoughnessTexture > -1)
    {
        float4 sampled = materialTextures[METALLIC_ROUGHNESS_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        roughnessFactor *= sampled.g;
        metallicFactor *= sampled.b;
    }
    
    float occlusionFactor = 1.0f;
    const int occlusionTexture = materialConstant.occlusionTexture.index;
    if (occlusionTexture > -1)
    {
        float4 sampled = materialTextures[OCCLUSION_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        occlusionFactor *= sampled.r;
    }
    const float occlusionStrength = materialConstant.occlusionTexture.strength;
    
    const float3 f0 = lerp(0.04f, baseColorFactor.rgb, metallicFactor);
    const float3 f90 = 1.0f;
    const float alphaRoughness = roughnessFactor * roughnessFactor;
    const float3 cDiff = lerp(baseColorFactor.rgb, 0.0f, metallicFactor);
    
    const float3 P = pin.wPosition.xyz;
    const float3 V = normalize(cameraPosition.xyz - pin.wPosition.xyz);
    
    float3 N = normalize(pin.wNormal.xyz);
    float3 T = hasTangent ? normalize(pin.wTangent.xyz) : float3(1, 0, 0);
    float sigma = hasTangent ? pin.wTangent.w : 1.0f;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    
    const int normalTexture = materialConstant.normalTexture.index;
    if (normalTexture > -1)
    {
        float4 sampled = materialTextures[NORMAL_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        float3 normalFactor = sampled.xyz;
        normalFactor = (normalFactor * 2.0f) - 1.0f;
        normalFactor = normalize(normalFactor * float3(materialConstant.normalTexture.scale, materialConstant.normalTexture.scale, 1.0f));
        N = normalize((normalFactor.x * T) + (normalFactor.y * B) + (normalFactor.z * N));
    }
    
    float3 diffuse  = 0;
    float3 specular = 0;
    
    //  Loop for shading process for each light
    float3 L = normalize(-lightDirection.xyz);
    float3 Li = float3(1.0f, 1.0f, 1.0f);   //  Radiance of the light
    const float NoL = max(0.0f, dot(N, L));
    const float NoV = max(0.0f, dot(N, V));
    if (NoL > 0.0f || NoV > 0.0f)
    {
        const float3 R = reflect(-L, N);
        const float3 H = normalize(V + L);
        
        const float NoH = max(0.0f, dot(N, H));
        const float HoV = max(0.0f, dot(H, V));
        
        diffuse += Li * NoL * BrdfLambertian(f0, f90, cDiff, HoV);
        specular += Li * NoL * BrdfSpecularGgx(f0, f90, alphaRoughness, HoV, NoL, NoV, NoH);
    }
    
    diffuse += IblRadianceLambertian(N, V, roughnessFactor, cDiff, f0);
    specular += IblRadianceGgx(N, V, roughnessFactor, f0);
    
    float3 emissive = emissiveFactor;
    diffuse = lerp(diffuse, diffuse * occlusionFactor, occlusionStrength);
    specular = lerp(specular, specular * occlusionFactor, occlusionStrength);
 
    float3 Lo = diffuse + specular + emissive;
    return float4(Lo, baseColorFactor.a);

#endif
    
#if 0   //  Unit38ˆÈ‘O
    const MaterialConstants materialConstant = materials[material];
    
    float4 baseColor = materialConstant.pbrMetallicRoughness.baseColorTexture.index > -1 ?
    materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord) :
    materialConstant.pbrMetallicRoughness.baseColorFactor;
    
    float3 emissive = materialConstant.emissiveTexture.index > -1 ?
    materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord).rgb :
    materialConstant.emissiveFactor;

    float3 N = normalize(pin.wNormal.xyz);
    float3 L = normalize(-lightDirection.xyz);
    
    float3 color = max(0, dot(N, L)) * baseColor.rgb + emissive;
    
    return float4(color, baseColor.a);
#endif
    
}