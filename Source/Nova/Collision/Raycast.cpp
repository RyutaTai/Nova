#include "Raycast.h"

using namespace DirectX;

//	The function returns the index of the triangle if it intersects, or -1 if it does not.
int IntersectRayTriangles(
	const float* positions, //	vertices
	const uint32_t offset,	//	bytes
	const uint32_t stride,	//	bytes
	const uint32_t* indices,
	const size_t indexCount,
	const XMFLOAT3& rayPosition,
	const XMFLOAT3& rayDirection,
	XMFLOAT4& intersection,
	float& distance,
	const bool RHS)
{
	const bool CCW{ RHS }; // Counterclockwise 3D triangle ABC 反時計回りの3次元三角形ABC
	const int C0{ 0 };
	const int C1{ CCW ? 1 : 2 };	//if(CCW) C1=1; else C1=2; と同じ
	const int C2{ CCW ? 2 : 1 };

	// Ray-triangle intersection 
	// In this handout, we explore the steps needed to compute the intersection of a ray with a triangle, and then to
	// compute the barycentric coordinates of that intersection.First, we consider the geometry of such an intersection :
	//
	// where a ray with origin P and direction D intersects a triangle defined by its vertices, A, B, and C at intersection
	// point Q.The square region diagrammatically surrounding triangle ABC represents the supporting plane of the
	// triangle, i.e., the plane that the triangle lies on.
	//
	// To perform ray - triangle intersection, we must perform two steps :
	//
	//  1. Determine the point of intersection, Q.
	//  2. Determine if Q lies inside of triangle ABC.
	//
	//	光線と三角形の交点 
	//	このハンドアウトでは、光線と三角形の交点を計算し,その交点の重心座標を計算するために必要なステップを探ります。
	//	まず、このような交点の幾何学的形状を考えます：
	//
	//	原点 P と方向 D を持つ光線が、その頂点 A, B, C で定義される三角形と交点 Q で交わる。
	//	三角形ABCを図式的に囲む正方形の領域は、三角形の支持平面を表す。
	//	三角形ABCを図式的に囲む四角い領域は、三角形の支持平面、すなわち三角形が横たわる平面を表す。
	//
	//	光線と三角形の交点を求めるには、2つのステップを実行する必要がある：
	//
	//	1. 交点Qを決定する。
	//	Q が三角形 ABC の内側にあるかどうかを判断する。

	int intersectionCount{ 0 };				//	交差する数
	int intersectedTriangleIndex{ -1 };		//	交差する三角形のインデックス

	const float rayLengthLimit{ distance };	//	レイの長さの最大
	float closestDistance{ FLT_MAX };		//	最も近い点

	XMVECTOR X{}; // closest cross point
	const XMVECTOR P{ XMVectorSet(rayPosition.x, rayPosition.y, rayPosition.z, 1) };							//		
	const XMVECTOR D{ XMVector3Normalize(XMVectorSet(rayDirection.x, rayDirection.y, rayDirection.z, 0)) };	//	

	using byte = uint8_t;
	const byte* p{ reinterpret_cast<const byte*>(positions) + offset };
	const size_t triangleCount{ indexCount / 3 };
	for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
	{
		const XMVECTOR A{ XMVectorSet(
			(reinterpret_cast<const float*>(p + indices[triangleIndex * 3 + C0] * stride))[0],
			(reinterpret_cast<const float*>(p + indices[triangleIndex * 3 + C0] * stride))[1],
			(reinterpret_cast<const float*>(p + indices[triangleIndex * 3 + C0] * stride))[2],
			1.0f
		) };
		const XMVECTOR B{ XMVectorSet(
			(reinterpret_cast<const float*>(p + indices[triangleIndex * 3 + C1] * stride))[0],
			(reinterpret_cast<const float*>(p + indices[triangleIndex * 3 + C1] * stride))[1],
			(reinterpret_cast<const float*>(p + indices[triangleIndex * 3 + C1] * stride))[2],
			1.0f
		) };
		const XMVECTOR C{ XMVectorSet(
			(reinterpret_cast<const float*>(p + indices[triangleIndex * 3 + C2] * stride))[0],
			(reinterpret_cast<const float*>(p + indices[triangleIndex * 3 + C2] * stride))[1],
			(reinterpret_cast<const float*>(p + indices[triangleIndex * 3 + C2] * stride))[2],
			1.0f
		) };

		//	Determining Q
		//	Determining the point of intersection Q in turn requires us to take two more steps :
		//  1. Determine the equation of the supporting plane for triangle ABC.
		//  2. Intersect the ray with the supporting plane.
		//	Qの決定
		//	交点Qを求めるには、さらに2つのステップを踏む必要がある：
		//	1. 三角形 ABC の支持平面の方程式を求める。
		//	2. 支持平面と光線を交差させる。
		XMVECTOR Q;

		//	1. Determine the equation of the supporting plane for triangle ABC.
		//	1. 三角形ABCの支持平面の方程式を求める。
		const XMVECTOR N{ XMVector3Normalize(XMVector3Cross(XMVectorSubtract(B, A), XMVectorSubtract(C, A))) };
		const float d{ XMVectorGetByIndex(XMVector3Dot(N, A), 0) };

		//	2. Intersect the ray with the supporting plane.
		//	2. 支持平面と光線を交差させる。
		const float denominator{ XMVectorGetByIndex(XMVector3Dot(N, D), 0) };
		if (denominator < 0) //		Note that if N.D = 0 , then D is parallel to the plane and the ray does not intersect the plane.
							 //		N.D = 0 の場合、D は平面に平行であり、光線は平面と交差しない。
		{
			const float numerator{ d - XMVectorGetByIndex(XMVector3Dot(N, P), 0) };
			const float t{ numerator / denominator };

			if (t > 0 && t < rayLengthLimit) // Forward and Length limit of Ray
			{
				Q = XMVectorAdd(P, XMVectorScale(D, t));

				//	Triangle inside - outside testing
				//	三角形の内側と外側のテスト

				//	CHRISTER ERICSON. "REAL-TIME COLLISION DETECTION" pp.204
				//	Test if point Q lies inside the counterclockwise 3D triangle ABC
				//	点Qが反時計回りの3次元三角形ABCの内側にあるかどうかをテストする。

				//	Translate point and triangle so that point lies at origin
				//	点と三角形を移動して、その点が原点にくるようにする		

				const XMVECTOR QA{ XMVectorSubtract(A, Q) };
				const XMVECTOR QB{ XMVectorSubtract(B, Q) };
				const XMVECTOR QC{ XMVectorSubtract(C, Q) };

				XMVECTOR U{ XMVector3Cross(QB, QC) };
				XMVECTOR V{ XMVector3Cross(QC, QA) };
				if (XMVectorGetByIndex(XMVector3Dot(U, V), 0) < 0)
				{
					continue;
				}

				XMVECTOR W{ XMVector3Cross(QA, QB) };
				if (XMVectorGetByIndex(XMVector3Dot(U, W), 0) < 0)
				{
					continue;
				}
				if (XMVectorGetByIndex(XMVector3Dot(V, W), 0) < 0)
				{
					continue;
				}

				//	Otherwise Q must be in (or on) the triangle
				if (t < closestDistance)
				{
					closestDistance = t;
					intersectedTriangleIndex = static_cast<int>(triangleIndex);
					X = Q;
				}
				intersectionCount++;
			}
		}
	}
	if (intersectionCount > 0)
	{
		XMStoreFloat4(&intersection, X);
		distance = closestDistance;
	}
	return intersectedTriangleIndex;
}
