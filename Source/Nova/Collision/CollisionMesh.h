#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <DirectXCollision.h>
#include <vector>
#include <string>

class CollisionMesh
{
public:
	struct Mesh
	{
		std::string name_;
		struct Subset
		{
			std::string materialName_;
			std::vector<DirectX::XMFLOAT3> positions_;
		};
		std::vector<Subset> subsets_;
		DirectX::XMFLOAT3	meshBoundingBox_[2]
		{
			{ +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX },
			{ -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX }
		};

		struct Triangle
		{
			DirectX::XMFLOAT3 positions_[3];
			DirectX::XMFLOAT3 normal_;
			std::string materialName_;
		};

		struct Area
		{
			DirectX::BoundingBox	areaBoundingBox_;
			std::vector<int>		triangleIndices_;	//	エリアに含まれる三角形の番号
		};
		std::vector<Triangle> triangles_;
		std::vector<Area> areas_;
	};
	std::vector<Mesh> meshes_;

public:
	CollisionMesh(ID3D11Device* device, const std::string& fileName, bool triangulate = false);

	//	デバッグ描画
	void DrawDebug();

	// The coordinate system of all function arguments is world space.
	bool Raycast(_In_ DirectX::XMFLOAT3 rayStartPosition, _In_ DirectX::XMFLOAT3 rayDirection, _In_ const DirectX::XMFLOAT4X4& transform, _Out_ DirectX::XMFLOAT3& intersectionPosition, _Out_ DirectX::XMFLOAT3& intersectionNormal,
		_Out_ std::string& intersectionMesh, _Out_ std::string& intersectionMaterial, _In_ float rayLengthLimit = 1.0e+7f, _In_ bool skipIf = false/*Once the first intersection is found, the process is interrupted.*/) const;

	//	空間分割を利用したレイキャスト
	bool RaycastWithSpaceDivision(_In_ const DirectX::XMFLOAT3& rayStartPosition, _In_ const DirectX::XMFLOAT3& rayDirection, _In_ const DirectX::XMFLOAT4X4& transform,
		_Out_ DirectX::XMFLOAT3& intersectionPosition, _Out_ DirectX::XMFLOAT3& intersectionNormal,
		_Out_ std::string& intersectionMesh, _Out_ std::string& intersectionMaterial,
		_In_ const float& rayLengthLimit, _In_ const bool& skipIf) const;

	//	エリアごとに分割する
	void CreateAreas(const int& gridSizeX, const int& gridSizeZ);

	void SetAreaGridSize(const int& gridSize) { areaGridSize_ = gridSize; }
	const int GetAreaGridSize()const { return areaGridSize_; }

private:
	int areaGridSize_ = 12;

};
