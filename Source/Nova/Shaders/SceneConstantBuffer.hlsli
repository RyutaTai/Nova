//  シーン定数バッファ
cbuffer SceneConstantBuffer : register(b1)
{
    row_major float4x4  viewProjection;
    float4              lightDirection;
    float4              cameraPosition;
    row_major float4x4  lightViewProjection;
    row_major float4x4  invViewProjection;
    row_major float4x4  invProjection;
};
