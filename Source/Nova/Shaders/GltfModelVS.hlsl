#include "GltfModel.hlsli"

VS_OUT main(VS_IN vin)
{
    float sigma = vin.tangent.w;
    
    if (skin > -1)
    {
        float totalWeight = totalWeight = dot(vin.weights[0], 1) + dot(vin.weights[1], 1); // This calculation is done to make the sum of the weights equal 1.
        float4x4 skinMatrix =
		    vin.weights[0].x / totalWeight * jointMatrices[vin.joints[0].x] +
		    vin.weights[0].y / totalWeight * jointMatrices[vin.joints[0].y] +
		    vin.weights[0].z / totalWeight * jointMatrices[vin.joints[0].z] +
		    vin.weights[0].w / totalWeight * jointMatrices[vin.joints[0].w] +
		    vin.weights[1].x / totalWeight * jointMatrices[vin.joints[1].x] +
		    vin.weights[1].y / totalWeight * jointMatrices[vin.joints[1].y] +
		    vin.weights[1].z / totalWeight * jointMatrices[vin.joints[1].z] +
		    vin.weights[1].w / totalWeight * jointMatrices[vin.joints[1].w];
        vin.position = mul(float4(vin.position.xyz, 1), skinMatrix);
        vin.normal = normalize(mul(float4(vin.normal.xyz, 0), skinMatrix));
        vin.tangent = normalize(mul(float4(vin.tangent.xyz, 0), skinMatrix));
        
    }
    
    VS_OUT vout;
    
    vin.position.w = 1;
    vout.position = mul(vin.position, mul(world, viewProjection));
    vout.wPosition = mul(vin.position, world);
    
    vin.normal.w = 0;
    vout.wNormal = normalize(mul(vin.normal, world));
    
    vin.tangent.w = 0;
    vout.wTangent = normalize(mul(vin.tangent, world));
    vout.wTangent.w = sigma;
    
    vout.texcoord = vin.texcoord;
    
    return vout;
}
