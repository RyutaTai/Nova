#include "../../Nova/Shaders/GltfModel.hlsli"
#include "../../Nova/Shaders/BidirectionalReflectanceDistributionFunction.hlsli"
#include "../../Nova/Shaders/SceneConstantBuffer.hlsli"

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
    int     alphaMode; //  "OPAQUE" : 0,"MASK" : 1,"BLEND" : 2
    float   alphaCutOff;
    bool    doubleSided;
    
    PbrMetallicRoughness pbrMetallicRoughness;
    
    NormalTextureInfo       normalTexture;
    OcclusionTextureInfo    occlusionTexture;
    TextureInfo             emissiveTexture;
};
StructuredBuffer<MaterialConstants> materials : register(t0);

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4
Texture2D<float4> materialTextures[5] : register(t1);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2

#if 0
// If you enable earlydepthstencil, 'clip', 'discard' and 'Alpha to coverage' won't work!
[earlydepthstencil]
#endif

float4 main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace) : SV_TARGET
{
    const float GAMMA = 2.2;

    const MaterialConstants m = materials[material];

    float4 baseColorFactor = m.pbrMetallicRoughness.baseColorFactor;
    const int baseColorTexture = m.pbrMetallicRoughness.baseColorTexture.index;
    if (baseColorTexture > -1)
    {
        float4 sampled = materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        baseColorFactor *= sampled;
    }
#if 1
    clip(baseColorFactor.a - 0.25);
#endif

    float3 emmisiveFactor = m.emissiveFactor;
    const int emissiveTexture = m.emissiveTexture.index;
    if (emissiveTexture > -1)
    {
        float4 sampled = materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emmisiveFactor *= sampled.rgb;
    }

    float roughnessFactor = m.pbrMetallicRoughness.roughnessFactor;
    float metallicFactor = m.pbrMetallicRoughness.metallicFactor;
    const int metallicRoughnessTexture = m.pbrMetallicRoughness.metallicRoughnessTexture.index;
    if (metallicRoughnessTexture > -1)
    {
        float4 sampled = materialTextures[METALLIC_ROUGHNESS_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        roughnessFactor *= sampled.g;
        metallicFactor *= sampled.b;
    }

    float occlusionFactor = 1.0;
    const int occlusionTexture = m.occlusionTexture.index;
    if (occlusionTexture > -1)
    {
        float4 sampled = materialTextures[OCCLUSION_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        occlusionFactor *= sampled.r;
    }
    const float occlusionStrength = m.occlusionTexture.strength;

    const float3 f0 = lerp(0.04, baseColorFactor.rgb, metallicFactor);
    const float3 f90 = 1.0;
    const float alphaRoughness = roughnessFactor * roughnessFactor;
    const float3 cDiff = lerp(baseColorFactor.rgb, 0.0, metallicFactor);

    const float3 P = pin.wPosition.xyz;
    const float3 V = normalize(cameraPosition.xyz - pin.wPosition.xyz);

    float3 N = normalize(pin.wNormal.xyz);
    float3 T = hasTangent ? normalize(pin.wTangent.xyz) : float3(1, 0, 0);
    float sigma = hasTangent ? pin.wTangent.w : 1.0;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
#if 1
	// For a back-facing surface, the tangential basis vectors are negated.
    if (isFrontFace == false)
    {
        T = -T;
        B = -B;
        N = -N;
    }
#endif

    const int normalTexture = m.normalTexture.index;
    if (normalTexture > -1)
    {
        float4 sampled = materialTextures[NORMAL_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        float3 normalFactor = sampled.xyz;
        normalFactor = (normalFactor * 2.0) - 1.0;
        normalFactor = normalize(normalFactor * float3(m.normalTexture.scale, m.normalTexture.scale, 1.0));
        N = normalize((normalFactor.x * T) + (normalFactor.y * B) + (normalFactor.z * N));
    }

    float3 diffuse = 0;
    float3 specular = 0;

	// Loop for shading process for each light
    float3 L = normalize(-lightDirection.xyz);
    float3 Li = float3(1.0, 1.0, 1.0); // Radiance of the light
    const float NoL = max(0.0, dot(N, L));
    const float NoV = max(0.0, dot(N, V));
    if (NoL > 0.0 || NoV > 0.0)
    {
        const float3 R = reflect(-L, N);
        const float3 H = normalize(V + L);
		//return sample_specular_pmrem(R, 0.0);

        const float NoH = max(0.0, dot(N, H));
        const float HoV = max(0.0, dot(H, V));

        diffuse += Li * NoL * BrdfLambertian(f0, f90, cDiff, HoV);
        specular += Li * NoL * BrdfSpecularGgx(f0, f90, alphaRoughness, HoV, NoL, NoV, NoH);
    }
    
    diffuse += IblRadianceLambertian(N, V, roughnessFactor, cDiff, f0);
    specular += IblRadianceGgx(N, V, roughnessFactor, f0);

    float3 emmisive = emmisiveFactor;
    diffuse = lerp(diffuse, diffuse * occlusionFactor, occlusionStrength);
    specular = lerp(specular, specular * occlusionFactor, occlusionStrength);

    float3 Lo = diffuse + specular + emmisive;
    return float4(Lo, baseColorFactor.a);
}

