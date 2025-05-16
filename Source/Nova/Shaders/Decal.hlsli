cbuffer ObjectConstantBuffer : register(b0)
{
	row_major float4x4 world;
	row_major float4x4 decalInverseProjection;
}
