#include "../../Nova/Shaders/GltfModel.hlsli"
#include "../../Nova/Shaders/BidirectionalReflectanceDistributionFunction.hlsli"

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

float4 main(VS_OUT pin) : SV_TARGET
{
    const float GAMMA = 2.2f;
    
    const MaterialConstants materialConstant = materials[material];
    
    float4 baseColorFactor = materialConstant.pbrMetallicRoughness.baseColorFactor;
    
    return float4(baseColorFactor);
}