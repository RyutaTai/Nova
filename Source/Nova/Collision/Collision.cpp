#include "Collision.h"

#include <vector>
#include <wrl.h>
#include <iostream>

#include "../Others/MathHelper.h"
#include "../Resources/GltfModel.h"

//	球と球の交差判定
bool Collision::IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float& radiusA,
	const DirectX::XMFLOAT3& positionB, const float& radiusB,
	DirectX::XMFLOAT3& outPositionB)
{
	//	A->Bの単位ベクトルを算出
	DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
	DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);
	DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);//ルートを取っていない状態（2乗）
	float lengthSq;
	DirectX::XMStoreFloat(&lengthSq, LengthSq);

	//	距離判定
	float range = radiusA + radiusB;
	if (lengthSq > range*range)//	球の距離が(半径+半径)より大きかったら当たっていない
	{
		return false;
	}

	//	AがBを押し出す
	DirectX::XMVECTOR OutPositionB;
	Vec = DirectX::XMVector3Normalize(Vec);
	Vec = DirectX::XMVectorScale(Vec, range);
	OutPositionB = DirectX::XMVectorAdd(PositionA,Vec);
	DirectX::XMStoreFloat3(&outPositionB, OutPositionB);

	return true;
}

//	円柱と円柱の交差判定
bool Collision::IntersectCylinderVsCyliner(const DirectX::XMFLOAT3& positionA, const float& radiusA, const float& heightA,
	const DirectX::XMFLOAT3& positionB, const float& radiusB, const float& heightB,
	DirectX::XMFLOAT3& outPosition, const bool& outIsA)
{
	//	Aの足元がBの頭より上なら当たっていない
	if (positionA.y > positionB.y + heightB)
	{
		return false;
	}
	//	Aの頭がBの足元より下なら当たっていない
	if (positionA.y + heightA < positionB.y)
	{
		return false;
	}
	//	XZ平面での範囲チェック
	DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
	DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);//PositionAとBが逆だった
	DirectX::XMVECTOR Length = DirectX::XMVector3LengthSq(Vec);
	float length;
	DirectX::XMStoreFloat(&length, Length);
	float range = radiusA + radiusB;
	if (length>range*range)
	{
		return false;
	}

	if (outIsA)
	{
		//	BがAを押し出す
		DirectX::XMVECTOR OutPositionA;
		OutPositionA = DirectX::XMVectorSubtract(PositionB, DirectX::XMVectorScale(DirectX::XMVector3Normalize(Vec), range));
		DirectX::XMStoreFloat3(&outPosition, OutPositionA);
	}
	else
	{
		//	AがBを押し出す
		DirectX::XMVECTOR OutPositionB;
		OutPositionB = DirectX::XMVectorAdd(PositionA, DirectX::XMVectorScale(DirectX::XMVector3Normalize(Vec), range));
		DirectX::XMStoreFloat3(&outPosition, OutPositionB);
	}

	return true;
}

//	球と円柱の交差判定
bool Collision::IntersectSphereVsCylinder(const DirectX::XMFLOAT3& spherePosition, const float& sphereRadius,
	const DirectX::XMFLOAT3& cylinderPosition, const float& cylinderRadius, const float& cylinderHeight,
	DirectX::XMFLOAT3& outCylinderPosition)
{
	//	球の位置が円柱の位置より上なら当たっていない
	if (spherePosition.y - sphereRadius > cylinderPosition.y + cylinderHeight)
	{
		return false;
	}
	//	球の位置が円柱の位置より下なら当たっていない
	if (spherePosition.y + sphereRadius < cylinderPosition.y)
	{
		return false;
	}

	//	A->Bの単位ベクトルを算出
	DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&spherePosition);
	DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&cylinderPosition);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);
	DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);//	ルートを取っていない状態（2乗）
	float lengthSq;
	DirectX::XMStoreFloat(&lengthSq, LengthSq);
	//	距離判定
	float range = sphereRadius + cylinderRadius;
	if (lengthSq > range * range)//	球の距離が(半径+半径)より大きかったら当たっていない
	{
		return false;
	}

	return true;
}
//
////	球Vs三角形
//bool Collision::IntersectSphereVsTriangle(const DirectX::XMVECTOR& spherePos, float radius, const DirectX::XMVECTOR trianglePos[3], IntersectionResult* result, bool triangleNormOnly)
//{
//	DirectX::XMVECTOR tmpPos = {};
//	GetClosestPointPointTriangle(spherePos, trianglePos, tmpPos);
//	DirectX::XMVECTOR vec = DirectX::XMVectorSubtract(spherePos, tmpPos);
//	bool ret = false;
//	if (result)
//	{
//		DirectX::XMVECTOR vecNorm = DirectX::XMVector3Normalize(vec);
//		result->pointA = DirectX::XMVectorAdd(spherePos, DirectX::XMVectorScale(vecNorm, radius));
//		result->pointB = tmpPos;
//		ret = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(vec)) <= (radius * radius);
//
//		if (ret && triangleNormOnly)
//		{
//			result->normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMVectorSubtract(trianglePos[1], trianglePos[0]), DirectX::XMVectorSubtract(trianglePos[2], trianglePos[0])));
//			float dist = DirectX::XMVectorGetX(DirectX::XMVector3Dot(result->normal, vec));
//			result->penetration = radius - dist;
//		}
//		else
//		{
//			result->normal = vecNorm;
//			result->penetration = radius - DirectX::XMVectorGetX(DirectX::XMVector3Length(vec));
//		}
//	}
//	return ret;
//}
//
////	スフィアキャストVsモデル（メッシュ）
//bool Collision::IntersectSphereCastVsModel(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, float radius, const Model* model, HitResultVector* result)
//{
//	DirectX::XMVECTOR WorldStart = DirectX::XMLoadFloat3(&start);
//	DirectX::XMVECTOR WorldEnd = DirectX::XMLoadFloat3(&end);
//	HitResultVector resultVec = {};
//
//	bool ret = Collision::IntersectSphereCastVsModel(WorldStart, WorldEnd, radius, model, &resultVec);
//
//	if (result)
//	{
//		result->distance = resultVec.distance;
//		result->position = resultVec.position;
//		result->normal = resultVec.normal;
//		result->materialIndex = resultVec.materialIndex;
//		for (int i = 0; i < 3; ++i)
//		{
//			result->triangleVerts[i] = resultVec.triangleVerts[i];
//		}
//	}
//
//	return ret;
//}
//
////	スフィアキャストVsモデル（メッシュ）
//bool Collision::IntersectSphereCastVsModel(const DirectX::XMVECTOR& start, const DirectX::XMVECTOR& end, float radius, const Model* model, HitResultVector* result)
//{
//	DirectX::XMVECTOR WorldStart = start;
//	DirectX::XMVECTOR WorldEnd = end;
//
//	// ワールド空間のレイの長さ
//	float minLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(WorldEnd, WorldStart)));
//
//	HitResult tmpResult;
//
//	bool hit = false;
//	const ModelResource* resource = model->GetResource();
//	for (const ModelResource::Mesh& mesh : resource->GetMeshes())
//	{
//		// メッシュノード取得
//		const Model::Node& node = model->GetNodes().at(mesh.nodeIndex);
//
//		DirectX::XMVECTOR Start = {};	// スフィアキャストの始点
//		DirectX::XMVECTOR End = {};		// スフィアキャストの終点
//
//		bool calcLocal = true;	// ローカル空間で処理するかどうかのフラグ
//		// 全軸等しいスケールでない場合、メッシュのローカル空間ではなく、ワールド空間で判定を行う
//		if (node.scale.x != node.scale.y || node.scale.y != node.scale.z)
//		{
//			calcLocal = false;
//		}
//
//		// レイの始点・終点をワールド空間からローカル空間へ変換
//		DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&node.worldTransform);
//		if (calcLocal)	// ローカルの場合レイを座標変換する
//		{
//			DirectX::XMMATRIX InvWorldMatrix = DirectX::XMMatrixInverse(nullptr, WorldTransform);
//			Start = DirectX::XMVector3Transform(start, InvWorldMatrix);
//			End = DirectX::XMVector3Transform(end, InvWorldMatrix);
//		}
//		else	// ワールドで処理する場合はそのまま
//		{
//			Start = start;
//			End = end;
//		}
//
//		// レイの長さと正規化
//		DirectX::XMVECTOR direction = DirectX::XMVectorSubtract(End, Start);
//		float neart = DirectX::XMVectorGetX(DirectX::XMVector3Length(direction));
//		direction = DirectX::XMVector3Normalize(direction);
//
//		// 三角形（面）との交差判定
//		const std::vector<ModelResource::Vertex>& vertices = mesh.vertices;
//		const std::vector<UINT> indices = mesh.indices;
//
//		int materialIndex = -1;
//		DirectX::XMVECTOR HitPosition;
//		DirectX::XMVECTOR HitNormal;
//		DirectX::XMVECTOR HitVerts[3] = {};
//
//		for (const ModelResource::Subset& subset : mesh.subsets)
//		{
//			for (UINT i = 0; i < subset.indexCount; i += 3)
//			{
//				UINT index = subset.startIndex + i;
//
//				// 三角形の頂点を抽出
//				const ModelResource::Vertex& a = vertices.at(indices.at(index));
//				const ModelResource::Vertex& b = vertices.at(indices.at(index + 1));
//				const ModelResource::Vertex& c = vertices.at(indices.at(index + 2));
//
//				DirectX::XMVECTOR TrianglePos[3] = {
//					DirectX::XMLoadFloat3(&a.position),
//					DirectX::XMLoadFloat3(&b.position),
//					DirectX::XMLoadFloat3(&c.position)
//				};
//
//				// スフィアキャストの実行前処理
//				float scaledRadius = radius;
//				if (calcLocal)	// ローカルの場合、半径をnode（三角形）のスケール倍する
//				{
//					scaledRadius *= node.scale.x;
//				}
//				else	// ワールドの場合、このタイミングで各頂点をワールド座標に変換しておく
//				{
//					TrianglePos[0] = DirectX::XMVector3Transform(TrianglePos[0], WorldTransform);
//					TrianglePos[1] = DirectX::XMVector3Transform(TrianglePos[1], WorldTransform);
//					TrianglePos[2] = DirectX::XMVector3Transform(TrianglePos[2], WorldTransform);
//				}
//
//				// スフィアキャストVs三角形
//				if (IntersectSphereCastVsTriangle(Start, direction, neart, scaledRadius, TrianglePos, &tmpResult))
//				{
//					// 最近距離を更新
//					if (neart > tmpResult.distance)
//					{
//						HitPosition = DirectX::XMLoadFloat3(&tmpResult.position);
//						HitNormal = DirectX::XMLoadFloat3(&tmpResult.normal);
//						HitVerts[0] = DirectX::XMLoadFloat3(&tmpResult.triangleVerts[0]);
//						HitVerts[1] = DirectX::XMLoadFloat3(&tmpResult.triangleVerts[1]);
//						HitVerts[2] = DirectX::XMLoadFloat3(&tmpResult.triangleVerts[2]);
//						neart = tmpResult.distance;
//						materialIndex = subset.materialIndex;
//					}
//				}
//			}
//		}
//		if (materialIndex >= 0)
//		{
//			// ヒット情報保存
//			if (minLength > neart)
//			{
//				if (result)
//				{
//					if (calcLocal)	// ヒット情報をローカル空間からワールド空間へ変換
//					{
//						result->position = DirectX::XMVector3TransformCoord(HitPosition, WorldTransform);
//						result->normal = DirectX::XMVector3TransformNormal(HitNormal, WorldTransform);
//						for (int i = 0; i < 3; ++i)
//						{
//							DirectX::XMVECTOR Vertex = DirectX::XMVector3Transform(HitVerts[i], WorldTransform);
//							result->triangleVerts[i] = Vertex;
//						}
//					}
//					else
//					{
//						result->position = HitPosition;
//						result->normal = HitNormal;
//						for (int i = 0; i < 3; ++i)
//						{
//							result->triangleVerts[i] = HitVerts[i];
//						}
//					}
//					result->normal = DirectX::XMVector3Normalize(result->normal);
//					result->distance = neart;
//					result->materialIndex = materialIndex;
//				}
//
//				minLength = neart;
//				hit = true;
//			}
//		}
//	}
//
//	return hit;
//}
//
////	スフィアキャストVs三角形
//bool Collision::IntersectSphereCastVsTriangle(const DirectX::XMVECTOR& sphereCastStart, const DirectX::XMVECTOR& sphereCastDirection, float sphereCastDist, float sphereCastRadius, const DirectX::XMVECTOR trianglePos[3], HitResult* result, bool firstSphereChk)
//{
//	// 三角形とスタート位置における球が交差している場合は、falseで終了する
//	if (firstSphereChk)
//	{
//		if (IntersectSphereVsTriangle(sphereCastStart, sphereCastRadius, trianglePos))
//		{
//			return false;
//		}
//	}
//
//	DirectX::XMVECTOR ab = DirectX::XMVectorSubtract(trianglePos[1], trianglePos[0]);
//	DirectX::XMVECTOR ac = DirectX::XMVectorSubtract(trianglePos[2], trianglePos[0]);
//	DirectX::XMVECTOR norm = DirectX::XMVector3Cross(ab, ac);
//	DirectX::XMVECTOR inverceDirection = DirectX::XMVectorScale(sphereCastDirection, -sphereCastDist);
//	float d = DirectX::XMVectorGetX(DirectX::XMVector3Dot(norm, inverceDirection));
//	bool hitFlg = false;
//
//	if (d >= 0.0f)	// 表側から交差しているときのみ判定を行う
//	{
//		// 三角形の各頂点を法線を元に球半径だけ移動させる
//		DirectX::XMVECTOR fixVec = DirectX::XMVectorScale(DirectX::XMVector3Normalize(norm), sphereCastRadius);
//
//		// 移動後の三角形とスフィアキャストの中心レイが交差するなら、元の三角形の内部(面領域)でスフィアキャストが交差することが確定
//		DirectX::XMVECTOR ap = DirectX::XMVectorSubtract(sphereCastStart, DirectX::XMVectorAdd(trianglePos[0], fixVec));
//		float t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(norm, ap));
//
//		if (t >= 0.0f && t < d)		// レイの向きと長さ確認
//		{
//			DirectX::XMVECTOR cross = DirectX::XMVector3Cross(inverceDirection, ap);
//			float v = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ac, cross));
//			if (v >= 0.0f && v <= d)
//			{
//				float w = -1.0f * DirectX::XMVectorGetX(DirectX::XMVector3Dot(ab, cross));
//				if (w > 0.0f && v + w <= d)
//				{
//					if (result)
//					{
//						DirectX::XMVECTOR crossPos = DirectX::XMVectorAdd(sphereCastStart, DirectX::XMVectorScale(inverceDirection, -t / d));
//						result->distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(crossPos, sphereCastStart)));
//						DirectX::XMStoreFloat3(&result->position, DirectX::XMVectorSubtract(crossPos, fixVec));
//						DirectX::XMStoreFloat3(&result->normal, DirectX::XMVector3Normalize(norm));
//						DirectX::XMStoreFloat3(&result->triangleVerts[0], trianglePos[0]);
//						DirectX::XMStoreFloat3(&result->triangleVerts[1], trianglePos[1]);
//						DirectX::XMStoreFloat3(&result->triangleVerts[2], trianglePos[2]);
//					}
//					return true;
//				}
//			}
//		}
//
//		// 面領域で交差がなければ、ボロノイの各頂点領域、辺領域で交差判定を行い、最短距離を算出する
//		enum class IntersectPattern
//		{
//			enNone = -1,
//			enVertex0 = 1,
//			enVertex1,
//			enVertex2,
//			enEdge01,
//			enEdge02,
//			enEdge12
//		};
//		HitResultVector tmpResult = {};
//		DirectX::XMVECTOR minPosition = {}, minNormal = {}, tmpOnCenterLinePos = {}, minOnCenterLinePos = {};
//		float minDistance = sphereCastDist;
//		IntersectPattern minDistCalcPattern = IntersectPattern::enNone;
//
//		// trianglePos[0] 頂点領域のチェック
//		if (IntersectRayVsSphere(sphereCastStart, sphereCastDirection, sphereCastDist, trianglePos[0], sphereCastRadius, &tmpResult))
//		{
//			if (minDistance > tmpResult.distance)
//			{
//				minDistance = tmpResult.distance;
//				minPosition = tmpResult.position;
//				minNormal = tmpResult.normal;
//				minDistCalcPattern = IntersectPattern::enVertex0;
//				hitFlg = true;
//			}
//		}
//
//		// trianglePos[1] 頂点領域のチェック
//		if (IntersectRayVsSphere(sphereCastStart, sphereCastDirection, sphereCastDist, trianglePos[1], sphereCastRadius, &tmpResult))
//		{
//			if (minDistance > tmpResult.distance)
//			{
//				minDistance = tmpResult.distance;
//				minPosition = tmpResult.position;
//				minNormal = tmpResult.normal;
//				minDistCalcPattern = IntersectPattern::enVertex1;
//				hitFlg = true;
//			}
//		}
//
//		// trianglePos[2] 頂点領域のチェック
//		if (IntersectRayVsSphere(sphereCastStart, sphereCastDirection, sphereCastDist, trianglePos[2], sphereCastRadius, &tmpResult))
//		{
//			if (minDistance > tmpResult.distance)
//			{
//				minDistance = tmpResult.distance;
//				minPosition = tmpResult.position;
//				minNormal = tmpResult.normal;
//				minDistCalcPattern = IntersectPattern::enVertex2;
//				hitFlg = true;
//			}
//		}
//
//		// trianglePos[0]-trianglePos[1] 辺領域のチェック
//		if (IntersectRayVsOrientedCylinder(sphereCastStart, sphereCastDirection, sphereCastDist, trianglePos[0], trianglePos[1], sphereCastRadius, &tmpResult, &tmpOnCenterLinePos))
//		{
//			if (minDistance > tmpResult.distance)
//			{
//				minDistance = tmpResult.distance;
//				minPosition = tmpResult.position;
//				minNormal = tmpResult.normal;
//				minOnCenterLinePos = tmpOnCenterLinePos;
//				minDistCalcPattern = IntersectPattern::enEdge01;
//				hitFlg = true;
//			}
//		}
//
//		// trianglePos[0]-trianglePos[2] 辺領域のチェック
//		if (IntersectRayVsOrientedCylinder(sphereCastStart, sphereCastDirection, sphereCastDist, trianglePos[0], trianglePos[2], sphereCastRadius, &tmpResult, &tmpOnCenterLinePos))
//		{
//			if (minDistance > tmpResult.distance)
//			{
//				minDistance = tmpResult.distance;
//				minPosition = tmpResult.position;
//				minNormal = tmpResult.normal;
//				minOnCenterLinePos = tmpOnCenterLinePos;
//				minDistCalcPattern = IntersectPattern::enEdge02;
//				hitFlg = true;
//			}
//		}
//
//		// trianglePos[1]-trianglePos[2] 辺領域のチェック
//		if (IntersectRayVsOrientedCylinder(sphereCastStart, sphereCastDirection, sphereCastDist, trianglePos[1], trianglePos[2], sphereCastRadius, &tmpResult, &tmpOnCenterLinePos))
//		{
//			if (minDistance > tmpResult.distance)
//			{
//				minDistance = tmpResult.distance;
//				minPosition = tmpResult.position;
//				minNormal = tmpResult.normal;
//				minOnCenterLinePos = tmpOnCenterLinePos;
//				minDistCalcPattern = IntersectPattern::enEdge12;
//				hitFlg = true;
//			}
//		}
//
//		// 交差が確定し、resultが有効ならHitResult情報を算出する
//		if (hitFlg && result)
//		{
//			result->distance = minDistance;
//			DirectX::XMStoreFloat3(&result->triangleVerts[0], trianglePos[0]);
//			DirectX::XMStoreFloat3(&result->triangleVerts[1], trianglePos[1]);
//			DirectX::XMStoreFloat3(&result->triangleVerts[2], trianglePos[2]);
//			DirectX::XMStoreFloat3(&result->normal, DirectX::XMVector3Normalize(minNormal));
//
//			// 交点(result->position)の算出は当たり方によって分岐
//			switch (minDistCalcPattern)
//			{
//			case IntersectPattern::enVertex0:
//				DirectX::XMStoreFloat3(&result->position, trianglePos[0]);
//				break;
//			case IntersectPattern::enVertex1:
//				DirectX::XMStoreFloat3(&result->position, trianglePos[1]);
//				break;
//			case IntersectPattern::enVertex2:
//				DirectX::XMStoreFloat3(&result->position, trianglePos[2]);
//				break;
//			case IntersectPattern::enEdge01:
//			case IntersectPattern::enEdge02:
//			case IntersectPattern::enEdge12:
//				DirectX::XMStoreFloat3(&result->position, minOnCenterLinePos);
//				break;
//
//			default:
//				break;
//			}
//		}
//	}
//
//	return hitFlg;
//}
//
////	レイVs球
//bool Collision::IntersectRayVsSphere(const DirectX::XMVECTOR& rayStart, const DirectX::XMVECTOR& rayDirection, float rayDist, const DirectX::XMVECTOR& spherePos, float radius, HitResultVector* result)
//{
//	DirectX::XMVECTOR ray2sphere = DirectX::XMVectorSubtract(spherePos, rayStart);
//	float projection = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ray2sphere, rayDirection));
//	float distSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(ray2sphere)) - projection * projection;
//
//	if (distSq < radius * radius)
//	{
//		float distance = projection - sqrtf(radius * radius - distSq);
//		if (distance > 0.0f)
//		{
//			if (distance < rayDist)
//			{
//				if (result)
//				{
//					result->position = DirectX::XMVectorAdd(rayStart, DirectX::XMVectorScale(rayDirection, distance));
//					result->distance = distance;
//					result->normal = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(result->position, spherePos));
//				}
//				return true;
//			}
//		}
//	}
//
//	return false;
//}
//
////	レイVs円柱
//bool Collision::IntersectRayVsOrientedCylinder(const DirectX::XMVECTOR& rayStart, const DirectX::XMVECTOR& rayDirection, float rayDist, const DirectX::XMVECTOR& startCylinder, const DirectX::XMVECTOR& endCylinder, float radius, HitResultVector* result, DirectX::XMVECTOR* onCenterLinPos)
//{
//	DirectX::XMVECTOR d = DirectX::XMVectorSubtract(endCylinder, startCylinder);
//	DirectX::XMVECTOR m = DirectX::XMVectorSubtract(rayStart, startCylinder);
//	DirectX::XMVECTOR n = DirectX::XMVectorScale(rayDirection, rayDist);
//
//	float md = DirectX::XMVectorGetX(DirectX::XMVector3Dot(m, d));
//	float nd = DirectX::XMVectorGetX(DirectX::XMVector3Dot(n, d));
//	float dd = DirectX::XMVectorGetX(DirectX::XMVector3Dot(d, d));
//
//	// 線分全体が円柱の底面・上面に垂直なスラブに対して外側にあるかどうかを判定
//
//	//レイの始点がスラブPQの外側にある
//	if (md < 0)
//	{
//		//レイの先端がスラブPQに届かないか
//		//float mdDot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(m, d));
//		//float ndDot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(n, d));
//		if (md + nd < 0)
//		{
//			//底面を含む平面と交差しない
//			return false;
//		}
//
//	}
//	//上面の外側にあるかどうか
//	else if (md > 0)
//	{
//		if (md > dd && md + nd > dd)
//		{
//			return false;
//		}
//	}
//
//	float nn = DirectX::XMVectorGetX(DirectX::XMVector3Dot(n, n));
//	float a = nn * dd - nd * nd;
//	float mm = DirectX::XMVectorGetX(DirectX::XMVector3Dot(m, m));
//	float k = mm - radius * radius;
//	float c = mm * dd - md * md - radius * radius * dd;
//
//	// 線分が円柱の軸に対して平行
//	if (fabsf(a) < 0.0001f)		// 誤差が出やすい計算なので閾値は大きめ（0.0001f）
//	{
//		if (c > 0.0f) return false;	// 線分は円柱の外側
//
//		if (result)
//		{
//			// 底面交差のresultを算出
//			if (md < 0)
//			{
//				float t = -md / nd;
//				//交差点算出
//				result->position = DirectX::XMVectorAdd(rayStart, DirectX::XMVectorScale(n, t));
//
//				//法線算出
//				DirectX::XMVECTOR nLength = DirectX::XMVector3Length(n);
//				result->normal = DirectX::XMVectorDivide(DirectX::XMVectorSubtract({}, n), nLength);
//
//				//距離算出
//				result->distance = t * DirectX::XMVectorGetX(nLength);
//			}
//			// 上面交差のresultを算出
//			else
//			{
//				float t = (dd - md) / nd;
//				//交差点算出
//				result->position = DirectX::XMVectorAdd(rayStart, DirectX::XMVectorScale(n, t));
//
//				//法線算出
//				DirectX::XMVECTOR nLength = DirectX::XMVector3Length(n);
//				result->normal = DirectX::XMVectorDivide(DirectX::XMVectorSubtract({}, n), nLength);
//
//				//距離算出
//				result->distance = t * DirectX::XMVectorGetX(nLength);
//			}
//
//		}
//		return true;
//	}
//
//	// 線分が円柱の軸に対して平行でない
//	// 円柱の表面を表す陰関数方程式と直線の方程式の解を求めて交差判定を行う。
//	float mn = DirectX::XMVectorGetX(DirectX::XMVector3Dot(m, n));
//	float b = mn * dd - nd * md;
//	float D = b * b - a * c;	// 判別式
//
//	if (D < 0) return false;	// 実数解がないので交差していない
//
//	// 解の公式により、交点までの距離を算出
//	float hitDistance = -b - sqrtf(D);
//	if (hitDistance < 0.0f)
//	{
//		hitDistance = (-b + sqrtf(D));
//		if (hitDistance < 0.0f)
//		{
//			return false;	// 交点が線分の外側にあり交差していない
//		}
//	}
//	else if (hitDistance > a)
//	{
//		hitDistance = (-b + sqrtf(D));
//		if (hitDistance > a)
//		{
//			return false;	// 交点が線分の外側にあり交差していない
//		}
//	}
//	hitDistance /= a;
//
//	// 解の公式の結果、円柱のstartCylinder側の底面の外で交差しているか確認
//	if (md + hitDistance * nd < 0.0f)
//	{
//		// 底面との交差を確認し、交差していればhitDistanceを更新
//		float t = -md / nd;
//		float v = mm + 2 * t * mn + t * t * nn;
//		if (v > radius * radius)
//		{
//			//交差しない
//			return false;
//		}
//		hitDistance = t;
//	}
//	// 解の公式の結果、円柱のendCylinder側の上面の外で交差しているか確認
//	else if (md + hitDistance * nd > dd)
//	{
//		// 上面との交差を確認し、交差していればhitDistanceを更新
//		float t = -(md - dd) / nd;
//		float v = mm + 2 * t * mn - 2 * md + t * t * nn - 2 * t * nd + dd;
//		if (v > radius * radius)
//		{
//			//交差しない
//			return false;
//		}
//		hitDistance = t;
//	}
//
//	// 線分が円柱の底面と上面の間で交差していることが確定（底面・上面での交差も含む）
//	if (result)
//	{
//		// resultの各パラメータを算出する
//		result->position = DirectX::XMVectorAdd(rayStart, DirectX::XMVectorScale(n, hitDistance));
//
//		//法線算出
//		DirectX::XMVECTOR nLength = DirectX::XMVector3Length(n);
//		result->normal = DirectX::XMVectorDivide(DirectX::XMVectorSubtract({}, n), nLength);
//
//		//距離算出
//		result->distance = hitDistance * DirectX::XMVectorGetX(nLength);
//
//		if (onCenterLinPos)
//		{
//			//円柱の軸上における最近点の算出
//			DirectX::XMVECTOR DNormal = DirectX::XMVector3Normalize(d);
//			DirectX::XMVECTOR V = DirectX::XMVectorSubtract(result->position, startCylinder);
//			DirectX::XMVECTOR Proj = DirectX::XMVectorScale(DNormal, DirectX::XMVectorGetX(DirectX::XMVector3Dot(DNormal, V)));
//			*onCenterLinPos = DirectX::XMVectorAdd(startCylinder, Proj);
//		}
//	}
//
//	return true;
//}
//
////	外部の点に対する三角形内部の最近点を取得する(要確認)
//bool Collision::GetClosestPointPointTriangle(const DirectX::XMVECTOR& point, const DirectX::XMVECTOR trianglePos[3], DirectX::XMVECTOR& nearPos)
//{
//	// 各頂点間のベクトル
//	DirectX::XMVECTOR Vec01 = trianglePos[1] - trianglePos[0];
//	DirectX::XMVECTOR Vec02 = trianglePos[2] - trianglePos[0];
//	DirectX::XMVECTOR Vec0P = point - trianglePos[0];
//
//	// 点が TrianglePos[0] の外側の頂点領域にあるかチェック
//	float d1 = DirectX::XMVector3Dot(Vec01, Vec0P).m128_f32[0];
//	float d2 = DirectX::XMVector3Dot(Vec02, Vec0P).m128_f32[0];
//	if (d1 <= 0.0f && d2 <= 0.0f) {
//		nearPos = trianglePos[0];
//		return false;  // TrianglePos[0] が最も近い
//	}
//
//	// 点が TrianglePos[1] の外側の頂点領域にあるかチェック
//	DirectX::XMVECTOR Vec1P = point - trianglePos[1];
//	float d3 = DirectX::XMVector3Dot(Vec01, Vec1P).m128_f32[0];
//	float d4 = DirectX::XMVector3Dot(Vec02, Vec1P).m128_f32[0];
//	if (d3 >= 0.0f && d4 <= d3) {
//		nearPos = trianglePos[1];
//		return false;  // TrianglePos[1] が最も近い
//	}
//
//	// 点が TrianglePos[2] の外側の頂点領域にあるかチェック
//	DirectX::XMVECTOR Vec2P = point - trianglePos[2];
//	float d5 = DirectX::XMVector3Dot(Vec02, Vec2P).m128_f32[0];
//	float d6 = DirectX::XMVector3Dot(Vec01, Vec2P).m128_f32[0];
//	if (d5 >= 0.0f && d6 <= d5) {
//		nearPos = trianglePos[2];
//		return false;  // TrianglePos[2] が最も近い
//	}
//
//	// 辺上の最近点を算出
//	float v = d1 * d4 - d3 * d2;
//	float u = d5 * d2 - d1 * d6;
//	float w = d3 * d6 - d5 * d4;
//
//	// 最近点が辺 TrianglePos[0]-TrianglePos[1] にあるかチェック
//	if (v <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
//		float t = d1 / (d1 - d3);
//		nearPos = DirectX::XMVectorAdd(trianglePos[0], DirectX::XMVectorScale(Vec01, t));
//		return false;
//	}
//
//	// 最近点が辺 TrianglePos[0]-TrianglePos[2] にあるかチェック
//	if (u <= 0.0f && d2 >= 0.0f && d5 <= 0.0f) {
//		float t = d2 / (d2 - d5);
//		nearPos = DirectX::XMVectorAdd(trianglePos[0], DirectX::XMVectorScale(Vec02, t));
//		return false;
//	}
//
//	// 最近点が辺 TrianglePos[1]-TrianglePos[2] にあるかチェック
//	if (w <= 0.0f && (d4 - d3) >= 0.0f && (d6 - d5) >= 0.0f) {
//		float t = (d4 - d3) / ((d4 - d3) + (d6 - d5));
//		nearPos = DirectX::XMVectorAdd(trianglePos[1], DirectX::XMVectorScale(trianglePos[2] - trianglePos[1], t));
//		return false;
//	}
//
//	// ここまでくれば、nearPosは三角形の内部にある
//	float denom = 1.0f / (u + v + w);
//	float t01 = v * denom;
//	float t02 = w * denom;
//	nearPos = DirectX::XMVectorAdd(
//		DirectX::XMVectorAdd(trianglePos[0],DirectX::XMVectorScale(Vec01, t01)),
//		DirectX::XMVectorScale(Vec02, t02)
//	);
//
//	return true;
//}
//
