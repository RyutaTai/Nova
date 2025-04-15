#pragma once

#include <DirectXMath.h>

//	当たった結果をまとめて持つ
struct HitResult
{
	DirectX::XMFLOAT3	position = { 0, 0, 0 };	// レイとポリゴンの交点
	DirectX::XMFLOAT3	normal = { 0, 0, 0 };	// 衝突したポリゴンの法線ベクトル
	float				distance = 0.0f; 		// レイの始点から交点までの距離
	int					materialIndex = -1; 	// 衝突したポリゴンのマテリアル番号
	DirectX::XMFLOAT3	triangleVerts[3] = {};
};
struct HitResultVector
{
	DirectX::XMVECTOR	position = { 0, 0, 0 };	// レイとポリゴンの交点
	DirectX::XMVECTOR	normal = { 0, 0, 0 };	// 衝突したポリゴンの法線ベクトル
	float				distance = 0.0f; 		// レイの始点から交点までの距離
	DirectX::XMVECTOR	triangleVerts[3] = {};
	int					materialIndex = -1; 	// 衝突したポリゴンのマテリアル番号
};
//	オブジェクトの交差結果の構造体
struct IntersectionResult
{
	DirectX::XMVECTOR	pointA = {};			// オブジェクトA側の交点。オブジェクトA Vs オブジェクトBで関数が構成される
	DirectX::XMVECTOR	pointB = {};			// オブジェクトB側の交点。オブジェクトA Vs オブジェクトBで関数が構成される
	DirectX::XMVECTOR	normal = {};			// 交点を結ぶ衝突の単位法線ベクトル。方向はオブジェクトB→オブジェクトA
	float				penetration = 0.0f; 	// 法線ベクトルを元にしためり込み量。交差している場合にプラスの値が返却される
};

//	コリジョン
class Collision
{
public:
	//	球と球の交差判定
	static bool IntersectSphereVsSphere(const DirectX::XMFLOAT3& positionA, const float& radiusA,
		const DirectX::XMFLOAT3& positionB, const float& radiusB,
		DirectX::XMFLOAT3& outPositionB);

	//	円柱と円柱の交差判定
	static bool IntersectCylinderVsCyliner(const DirectX::XMFLOAT3& positionA, const float& radiusA, const float& heightA,
		const DirectX::XMFLOAT3& positionB, const float& radiusB, const float& heightB,
		DirectX::XMFLOAT3& outPosition, const bool& outIsA = false/*Aを押すかBを押すか*/);

	//	球と円柱の交差判定
	static bool IntersectSphereVsCylinder(const DirectX::XMFLOAT3& spherePosition, const float& sphereRadius,
		const DirectX::XMFLOAT3& cylinderPosition, const float& cylinderRadius, const float& cylinderHeight,
		DirectX::XMFLOAT3& outCylinderPosition);
	
	////	球Vs三角形
	//static bool IntersectSphereVsTriangle(
	//	const DirectX::XMVECTOR& spherePos,
	//	float radius,
	//	const DirectX::XMVECTOR trianglePos[3],
	//	IntersectionResult* result = {},
	//	bool triangleNormOnly = false);

	////	スフィアキャストVsモデル（メッシュ）
	//static bool IntersectSphereCastVsModel(
	//	const DirectX::XMFLOAT3& start,			//	スフィアキャストのスタートの位置の球の中心
	//	const DirectX::XMFLOAT3& end,  			//	スフィアキャストのエンドの位置の球の中心
	//	float radius,
	//	const Model* model,
	//	HitResultVector* result = {});
	//static bool IntersectSphereCastVsModel(
	//	const DirectX::XMVECTOR& start,			//	スフィアキャストのスタートの位置の球の中心
	//	const DirectX::XMVECTOR& end,  			//	スフィアキャストのエンドの位置の球の中心
	//	float radius,
	//	const Model* model,
	//	HitResultVector* result = {});

	////	スフィアキャストVs三角形
	//static bool IntersectSphereCastVsTriangle(
	//	const DirectX::XMVECTOR& sphereCastStart,		//	スフィアキャストの発射点（球の中心）
	//	const DirectX::XMVECTOR& sphereCastDirection,	//	スフィアキャストの向き。要正規化
	//	float sphereCastDist,
	//	float sphereCastRadius,							//	スフィアキャストの半径
	//	const DirectX::XMVECTOR trianglePos[3],
	//	HitResult* result = {},
	//	bool firstSphereChk = false);					//	スフィアキャストの発射点の球が既に三角形に接している場合にfalseを返すチェックを行うかどうか

	////	レイVs球
	//static bool IntersectRayVsSphere(
	//	const DirectX::XMVECTOR& rayStart,
	//	const DirectX::XMVECTOR& rayDirection,		// 要正規化
	//	float rayDist,
	//	const DirectX::XMVECTOR& spherePos,
	//	float radius,
	//	HitResultVector* result = {});

	////	レイVs円柱
	//static bool IntersectRayVsOrientedCylinder(
	//	const DirectX::XMVECTOR& rayStart,
	//	const DirectX::XMVECTOR& rayDirection,		// 要正規化
	//	float rayDist,
	//	const DirectX::XMVECTOR& startCylinder,
	//	const DirectX::XMVECTOR& endCylinder,
	//	float radius,
	//	HitResultVector* result = {},
	//	DirectX::XMVECTOR* onCenterLinPos = {});

	////	外部の点に対する三角形内部の最近点を取得する
	//static bool GetClosestPointPointTriangle(
	//	const DirectX::XMVECTOR& point,
	//	const DirectX::XMVECTOR trianglePos[3],
	//	DirectX::XMVECTOR& nearPos);

};