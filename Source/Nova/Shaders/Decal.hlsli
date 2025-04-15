cbuffer OBJECT_CONSTANT_BUFFER : register(b0)
{
	row_major float4x4 world;
	row_major float4x4 decalInverseProjection;
}

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
	row_major float4x4	viewProjection;
	float4				lightDirection;
	float4				cameraPosition;
	row_major float4x4	inverseViewProjection;
}


