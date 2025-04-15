#include "../../Nova/Shaders/GltfModel.hlsli"
#include "../../Nova/Shaders/BidirectionalReflectanceDistributionFunction.hlsli"

#if 0
float4 main(VS_OUT pin) : SV_TARGET
{
	float3 N = normalize(pin.world_normal.xyz);
	float3 L = normalize(-light_direction.xyz);
	return float4(max(0, dot(N, L)) * pin.color.rgb, pin.color.a);
}
#endif

struct TextureInfo
{
    int index;
    int texcoord;
};

struct NormalTextureInfo
{
    int index;
    int texcoord;
    float scale;
};

struct OcclusionTextureInfo
{
    int index;
    int texcoord;
    float strength;
};

struct PbrMetallicRoughness
{
    float4 baseColorFactor;
    TextureInfo baseColorTexture;
    float metallicFactor;
    float roughnessFactor;
    TextureInfo metallicRoughnessTexture;
};

struct MaterialConstants
{
    float3 emissiveFactor;
    int alphaMode; //  "OPAQUE" : 0,"MASK" : 1,"BLEND" : 2
    float alphaCutOff;
    bool doubleSided;
    
    PbrMetallicRoughness pbrMetallicRoughness;
    
    NormalTextureInfo normalTexture;
    OcclusionTextureInfo occlusionTexture;
    TextureInfo emissiveTexture;
};
StructuredBuffer<MaterialConstants> materials : register(t0);

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4
Texture2D<float4> material_textures[5] : register(t1);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2

#if 0
// If you enable earlydepthstencil, 'clip', 'discard' and 'Alpha to coverage' won't work!
[earlydepthstencil]
#endif

float4 main(VS_OUT pin, bool is_front_face : SV_IsFrontFace) : SV_TARGET
{
    const float GAMMA = 2.2;

    const MaterialConstants m = materials[material];

    float4 basecolor_factor = m.pbrMetallicRoughness.baseColorFactor;
    const int basecolor_texture = m.pbrMetallicRoughness.baseColorTexture.index;
    if (basecolor_texture > -1)
    {
        float4 sampled = material_textures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        basecolor_factor *= sampled;
    }
#if 1
    clip(basecolor_factor.a - 0.25);
#endif

    float3 emmisive_factor = m.emissiveFactor;
    const int emissive_texture = m.emissiveTexture.index;
    if (emissive_texture > -1)
    {
        float4 sampled = material_textures[EMISSIVE_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emmisive_factor *= sampled.rgb;
    }

    float roughness_factor = m.pbrMetallicRoughness.roughnessFactor;
    float metallic_factor = m.pbrMetallicRoughness.metallicFactor;
    const int metallic_roughness_texture = m.pbrMetallicRoughness.metallicRoughnessTexture.index;
    if (metallic_roughness_texture > -1)
    {
        float4 sampled = material_textures[METALLIC_ROUGHNESS_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        roughness_factor *= sampled.g;
        metallic_factor *= sampled.b;
    }

    float occlusion_factor = 1.0;
    const int occlusion_texture = m.occlusionTexture.index;
    if (occlusion_texture > -1)
    {
        float4 sampled = material_textures[OCCLUSION_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        occlusion_factor *= sampled.r;
    }
    const float occlusion_strength = m.occlusionTexture.strength;

    const float3 f0 = lerp(0.04, basecolor_factor.rgb, metallic_factor);
    const float3 f90 = 1.0;
    const float alpha_roughness = roughness_factor * roughness_factor;
    const float3 c_diff = lerp(basecolor_factor.rgb, 0.0, metallic_factor);

    const float3 P = pin.wPosition.xyz;
    const float3 V = normalize(cameraPosition.xyz - pin.wPosition.xyz);

    float3 N = normalize(pin.wNormal.xyz);
    float3 T = hasTangent ? normalize(pin.wTangent.xyz) : float3(1, 0, 0);
    float sigma = hasTangent ? pin.wTangent.w : 1.0;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
#if 1
	// For a back-facing surface, the tangential basis vectors are negated.
    if (is_front_face == false)
    {
        T = -T;
        B = -B;
        N = -N;
    }
#endif

    const int normal_texture = m.normalTexture.index;
    if (normal_texture > -1)
    {
        float4 sampled = material_textures[NORMAL_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        float3 normal_factor = sampled.xyz;
        normal_factor = (normal_factor * 2.0) - 1.0;
        normal_factor = normalize(normal_factor * float3(m.normalTexture.scale, m.normalTexture.scale, 1.0));
        N = normalize((normal_factor.x * T) + (normal_factor.y * B) + (normal_factor.z * N));
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

        diffuse += Li * NoL * BrdfLambertian(f0, f90, c_diff, HoV);
        specular += Li * NoL * BrdfSpecularGgx(f0, f90, alpha_roughness, HoV, NoL, NoV, NoH);
    }
    
    diffuse += IblRadianceLambertian(N, V, roughness_factor, c_diff, f0);
    specular += IblRadianceGgx(N, V, roughness_factor, f0);

    float3 emmisive = emmisive_factor;
    diffuse = lerp(diffuse, diffuse * occlusion_factor, occlusion_strength);
    specular = lerp(specular, specular * occlusion_factor, occlusion_strength);

    float3 Lo = diffuse + specular + emmisive;
    return float4(Lo, basecolor_factor.a);
}

