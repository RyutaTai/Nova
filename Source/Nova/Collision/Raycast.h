#pragma once

#include <directxmath.h>
#include <utility>

int IntersectRayTriangles
(
	const float* positions, // vertices
	const uint32_t offset, // bytes
	const uint32_t stride, // bytes
	const uint32_t* indices,
	const size_t indexCount,
	const DirectX::XMFLOAT3& rayPosition,
	const DirectX::XMFLOAT3& rayDirection,
	DirectX::XMFLOAT4& intersection,
	float& distance,
	const bool RHS = true
);

// The coordinate systems of all the argument variables of these functions must be the same.
inline bool IntersectRayAABB(const float rayPos[3], const float rayDirection[3], const float min[3], const float max[3])
{
	float tMin{ 0 };
	float tMax{ +FLT_MAX };

	for (size_t a = 0; a < 3; ++a)
	{
		float inv_d{ 1.0f / rayDirection[a] };
		float t0{ (min[a] - rayPos[a]) * inv_d };
		float t1{ (max[a] - rayPos[a]) * inv_d };
		if (inv_d < 0.0f)
		{
			std::swap<float>(t0, t1);
		}
		tMin = t0 > tMin ? t0 : tMin;
		tMax = t1 < tMax ? t1 : tMax;
		if (tMax <= tMin)
		{
			return false;
		}
	}
	return true;
}
