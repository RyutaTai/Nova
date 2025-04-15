#include "Model.h"

#include <sstream>
#include <functional>
#include <filesystem>
#include <fstream>

#include "../Graphics/Shader.h"
#include "../Graphics/Graphics.h"
#include "../Others/Misc.h"
#include "../Resources/Texture.h"
#include "../Resources/StaticMesh.h"
#include "../Others/Converter.h"

//	コンストラクタ
Model::Model(ID3D11Device* device, const char* fbxFileName, bool triangulate/*三角化*/, float samplingRate, bool usedAsCollider)
{
	std::filesystem::path cerealFileName(fbxFileName);
	cerealFileName.replace_extension("cereal");
	if (std::filesystem::exists(cerealFileName.c_str()))	//	シリアライズされたファイルがあれば
	{
		std::ifstream ifs(cerealFileName.c_str(), std::ios::binary);
		cereal::BinaryInputArchive deserialization(ifs);
		deserialization(sceneView_, meshes_, materials_, animationClips_);
	}
	else //	シリアライズされたファイルがなければFBXからロードする
	{
		FbxManager* fbxManager{ FbxManager::Create() };
		FbxScene* fbxScene{ FbxScene::Create(fbxManager,"") };

		FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager,"") };
		bool importStatus{ false };
		importStatus = fbxImporter->Initialize(fbxFileName);
		_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

		importStatus = fbxImporter->Import(fbxScene);
		_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

		FbxGeometryConverter fbx_converter(fbxManager);
		if (triangulate)
		{
			fbx_converter.Triangulate(fbxScene, true/*replace*/, false/*legacy*/);
			fbx_converter.RemoveBadPolygonsFromMeshes(fbxScene);
		}

		std::function<void(FbxNode*)>traverse{ [&](FbxNode* fbxNode) {
			SceneStruct::Node& node{sceneView_.nodes_.emplace_back()};
			node.attribute_ = fbxNode->GetNodeAttribute() ?
				fbxNode->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
			node.name_ = fbxNode->GetName();
			node.uniqueID_ = fbxNode->GetUniqueID();
			node.parentIndex_ = sceneView_.indexof(fbxNode->GetParent() ?
				fbxNode->GetParent()->GetUniqueID() : 0);
			for (int childIndex = 0; childIndex < fbxNode->GetChildCount(); ++childIndex)
			{
				traverse(fbxNode->GetChild(childIndex));
			}
		} };
		traverse(fbxScene->GetRootNode());

		FetchMeshes(fbxScene, meshes_);
		FetchMaterials(fbxScene, materials_);
		FetchAnimations(fbxScene, animationClips_, samplingRate);

#if 0
		for (const Scene::Node& node : sceneView_.nodes_)
		{
			FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name_.c_str()) };
			//Display node data in the output window as debug
			std::string nodeName = fbxNode->GetName();
			uint64_t uid = fbxNode->GetUniqueID();
			uint64_t parentUid = fbxNode->GetParent() ? fbxNode->GetParent()->GetUniqueID() : 0;
			int32_t type = fbxNode->GetNodeAttribute() ? fbxNode->GetNodeAttribute()->GetAttributeType() : 0;

			std::stringstream debugString;
			debugString << nodeName << ":" << uid << ":" << parentUid << ":" << type << "\n";
			OutputDebugStringA(debugString.str().c_str());
		}
#endif
		fbxManager->Destroy();

		//	シリアライズ
		std::ofstream ofs(cerealFileName.c_str(), std::ios::binary);
		cereal::BinaryOutputArchive serialization(ofs);
		serialization(sceneView_, meshes_, materials_, animationClips_);
	}
	CreateComObjects(device, fbxFileName, usedAsCollider);

}

void Model::CreateComObjects(ID3D11Device* device, const char* fbxFileName, bool usedAsCollider)
{
	for (Mesh& mesh : meshes_)
	{
		HRESULT hr{ S_OK };
		D3D11_BUFFER_DESC bufferDesc{};
		D3D11_SUBRESOURCE_DATA subresourceData{};
		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices_.size());
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		subresourceData.pSysMem = mesh.vertices_.data();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.vertexBuffer_.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mesh.indices_.size());
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		subresourceData.pSysMem = mesh.indices_.data();
		hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.indexBuffer_.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	/*	if (!usedAsCollider) // TODO:レイキャスト
		{
			mesh.vertices_.clear();
			mesh.indices_.clear();
		}*/
	}

	for (std::unordered_map<uint64_t, Material>::iterator iterator = materials_.begin();
		iterator != materials_.end(); ++iterator)
	{
		for (size_t textureIndex = 0; textureIndex < 2; ++textureIndex)
		{
			if (iterator->second.textureFileNames_[textureIndex].size() > 0)
			{
				std::filesystem::path path(fbxFileName);
				path.replace_filename(iterator->second.textureFileNames_[textureIndex]);
				D3D11_TEXTURE2D_DESC texture2dDesc;
				LoadTextureFromFile(device, path.c_str(),
					iterator->second.shaderResourceViews_[textureIndex].GetAddressOf(), &texture2dDesc);
			}
			else
			{
				MakeDummyTexture(device, iterator->second.shaderResourceViews_[textureIndex].GetAddressOf(),
					textureIndex == 1 ? 0xFFFF7F7F : 0xFFFFFFFF, 16);
			}
		}
		if (iterator->second.textureFileNames_[4].size())
		{
			std::filesystem::path path(fbxFileName);
			path.replace_filename(iterator->second.textureFileNames_[4]);
			D3D11_TEXTURE2D_DESC texture2dDesc;
			LoadTextureFromFile(device, path.c_str(),
				iterator->second.shaderResourceViews_[4].GetAddressOf(), &texture2dDesc);
		}
	}

	HRESULT hr = S_OK;
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
		{"WEIGHTS",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT},
		{"BONES",0,DXGI_FORMAT_R32G32B32A32_UINT,0,D3D11_APPEND_ALIGNED_ELEMENT},
	};
	Graphics::Instance().GetShader()->CreateVsFromCso(device, "./Resources/Shader/ModelVs.cso", vertexShader_.ReleaseAndGetAddressOf(),
		inputLayout_.ReleaseAndGetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/ModelPs.cso", pixelShader_.ReleaseAndGetAddressOf());

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(Constants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constantBufer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
}

void Model::FetchMeshes(FbxScene* fbxScene, std::vector<Mesh>& meshes)
{
	for (const SceneStruct::Node& node : sceneView_.nodes_)
	{
		if (node.attribute_ != FbxNodeAttribute::EType::eMesh)
		{
			continue;
		}

		FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name_.c_str()) };
		FbxMesh* fbxMesh{ fbxNode->GetMesh() };

		Mesh& mesh{ meshes.emplace_back() };
#if 0
		mesh.uniqueID_ = fbxMesh->GetNode()->GetUniqueID();
		mesh.name_ = fbxMesh->GetNode()->GetName();
		mesh.nodeIndex_ = sceneView_.indexof(mesh.uniqueID_);
		mesh.defaultGlobalTransform_ = ToXMFLOAT4X4(fbxMesh->GetNode()->EvaluateGlobalTransform());
#else
		mesh.uniqueID_ = fbxNode->GetUniqueID();
		mesh.name_ = fbxNode->GetName();
		mesh.nodeIndex_ = sceneView_.indexof(mesh.uniqueID_);
		mesh.defaultGlobalTransform_ = ToXMFLOAT4X4(fbxNode->EvaluateGlobalTransform());
#endif


		std::vector<BoneInfluencesPerControlPoint>boneInfluences;
		FetchBoneInfluences(fbxMesh, boneInfluences);
		FetchSkeleton(fbxMesh, mesh.bindPose_);
		
		std::vector<Mesh::Subset>& subsets{ mesh.subsets_ };
		const int materialCount{ fbxMesh->GetNode()->GetMaterialCount() };
		subsets.resize(materialCount > 0 ? materialCount : 1);
		for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
		{
			const FbxSurfaceMaterial* fbxMaterial{ fbxMesh->GetNode()->GetMaterial(materialIndex) };
			subsets.at(materialIndex).materialName_ = fbxMaterial->GetName();
			subsets.at(materialIndex).materialUniqueID_ = fbxMaterial->GetUniqueID();
		}
		if (materialCount > 0)
		{
			const int polygonCount{ fbxMesh->GetPolygonCount() };
			for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
			{
				const int materialIndex{ fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) };
				subsets.at(materialIndex).indexCount_ += 3;
			}
			uint32_t offset{ 0 };
			for (Mesh::Subset& subset : subsets)
			{
				subset.startIndexLocation_ = offset;
				offset += subset.indexCount_;
				//This will be as counter in the following procedures,reset to zero
				subset.indexCount_ = 0;
			}
		}

		const int polygonCount{ fbxMesh->GetPolygonCount() };
		mesh.vertices_.resize(polygonCount * 3LL);
		mesh.indices_.resize(polygonCount * 3LL);

		FbxStringList uvNames;
		fbxMesh->GetUVSetNames(uvNames);
		const FbxVector4* controlPoints{ fbxMesh->GetControlPoints() };
		for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
		{
			const int materialIndex{ materialCount > 0 ?
			fbxMesh->GetElementMaterial()->GetIndexArray().GetAt(polygonIndex) : 0 };
			Mesh::Subset& subset{ subsets.at(materialIndex) };
			const uint32_t offset{ subset.startIndexLocation_ + subset.indexCount_ };

			for (int positionInPolygon = 0; positionInPolygon < 3; ++positionInPolygon)
			{
				const int vertexIndex{ polygonIndex * 3 + positionInPolygon };

				Vertex vertex;
				const int polygonVertex{ fbxMesh->GetPolygonVertex(polygonIndex,positionInPolygon) };
				vertex.position_.x = static_cast<float>(controlPoints[polygonVertex][0]);
				vertex.position_.y = static_cast<float>(controlPoints[polygonVertex][1]);
				vertex.position_.z = static_cast<float>(controlPoints[polygonVertex][2]);

				float indexStock[100] = {};		//	boneIndexを格納するため ゼロクリア必須
				float weightStock[100] = {};	//	boneWeightを格納するため

				const BoneInfluencesPerControlPoint& influencesPerControlPoint{ boneInfluences.at(polygonVertex) };
				for (size_t influenceIndex = 0; influenceIndex < influencesPerControlPoint.size(); ++influenceIndex)
				{
					if (influenceIndex < MAX_BONE_INFLUENCES)	//	ボーンの影響を受ける数は4つまでにする	
					{
						vertex.boneWeights_[influenceIndex] = influencesPerControlPoint.at(influenceIndex).boneWeight_;
						vertex.boneIndices_[influenceIndex] = influencesPerControlPoint.at(influenceIndex).boneIndex_;
					}
					else// TODO:影響を受けるボーンの数がMAX_BONE_INFLUENCES以上だったらどうするか考える
					{
						//_ASSERT_EXPR(FALSE, L"'influenceIndex' > MAX_BONE_INFLUENCES");//アサートで落とす

						//	boneWeightを大きい順にソートする
						for (int i = 0; i < influenceIndex; i++)
						{
							for (int j = i + 1; j < influenceIndex; j++)
							{
								if (weightStock[i] < weightStock[j])
								{
									float copyWeight = weightStock[i];
									weightStock[i] = weightStock[j];
									weightStock[j] = copyWeight;

									float copyIndex = indexStock[i];
									indexStock[i] = indexStock[j];
									indexStock[j] = copyIndex;
								}
							}
						}
					}
					//	boneWeightを格納していく
					weightStock[influenceIndex] = vertex.boneWeights_[influenceIndex];
					indexStock[influenceIndex] = vertex.boneIndices_[influenceIndex];
				}

				//	weightStockに格納しておいた値からMAX_BONE_INFLUENCES(４つ)の分取ってくる
				for (int influenceIndex = 0; influenceIndex < influencesPerControlPoint.size(); ++influenceIndex)
				{
					if (influenceIndex < MAX_BONE_INFLUENCES)
					{
						vertex.boneWeights_[influenceIndex] = weightStock[influenceIndex];
						vertex.boneIndices_[influenceIndex] = indexStock[influenceIndex];
					}
				}
				//	４つのweightのトータルは必ず1.0にする
				float totalInfluence;
				totalInfluence = vertex.boneWeights_[0] + vertex.boneWeights_[1] + vertex.boneWeights_[2] + vertex.boneWeights_[3];
				for (size_t influenceIndex = 0; influenceIndex < MAX_BONE_INFLUENCES; ++influenceIndex)
				{
					if (totalInfluence > 0.0f)
					{
						vertex.boneWeights_[influenceIndex] /= totalInfluence;
					}
				}

				if (fbxMesh->GetElementNormalCount() > 0)
				{
					FbxVector4 normal;
					fbxMesh->GetPolygonVertexNormal(polygonIndex, positionInPolygon, normal);
					vertex.normal_.x = static_cast<float>(normal[0]);
					vertex.normal_.y = static_cast<float>(normal[1]);
					vertex.normal_.z = static_cast<float>(normal[2]);
				}
				if (fbxMesh->GetElementUVCount() > 0)
				{
					FbxVector2 uv;
					bool unmappedUv;
					fbxMesh->GetPolygonVertexUV(polygonIndex, positionInPolygon,
						uvNames[0], uv, unmappedUv);
					vertex.texcoord_.x = static_cast<float>(uv[0]);
					vertex.texcoord_.y = 1.0f - static_cast<float>(uv[1]);
				}
				//	法線ベクトルの値を取得
				if (fbxMesh->GenerateTangentsData(0, false))
				{
					const FbxGeometryElementTangent* tangent = fbxMesh->GetElementTangent(0);
					vertex.tangent_.x = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[0]);
					vertex.tangent_.y = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[1]);
					vertex.tangent_.z = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[2]);
					vertex.tangent_.w = static_cast<float>(tangent->GetDirectArray().GetAt(vertexIndex)[3]);
				}
				if (fbxMesh->GenerateTangentsData(0, false))	//	タンジェントが無かったらダミーで値入れる,この引数がfalseならデータの上書きがされないらしい
				{
					vertex.tangent_.x = 1.001f;
					vertex.tangent_.y = 0.0f;
					vertex.tangent_.z = 0.0f;
					vertex.tangent_.w = 1.0f;
				}

				mesh.vertices_.at(vertexIndex) = std::move(vertex);
				mesh.indices_.at(static_cast<size_t>(offset) + positionInPolygon) = vertexIndex;
				subset.indexCount_++;
			}

		}

		for (const Vertex& v : mesh.vertices_)
		{
			mesh.boundingBox_[0].x = std::min<float>(mesh.boundingBox_[0].x, v.position_.x);
			mesh.boundingBox_[0].y = std::min<float>(mesh.boundingBox_[0].y, v.position_.y);
			mesh.boundingBox_[0].z = std::min<float>(mesh.boundingBox_[0].z, v.position_.z);
			mesh.boundingBox_[1].x = std::min<float>(mesh.boundingBox_[1].x, v.position_.x);
			mesh.boundingBox_[1].y = std::min<float>(mesh.boundingBox_[1].y, v.position_.y);
			mesh.boundingBox_[1].z = std::min<float>(mesh.boundingBox_[1].z, v.position_.z);
		}
	}
}

void Model::FetchMaterials(FbxScene* fbxScene, std::unordered_map<uint64_t, Material>& materials)
{
	const size_t nodeCount{ sceneView_.nodes_.size() };
	for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
	{
		const SceneStruct::Node& node{ sceneView_.nodes_.at(nodeIndex) };
		const FbxNode* fbxNode{ fbxScene->FindNodeByName(node.name_.c_str()) };
		const int materialCount{ fbxNode->GetMaterialCount() };

		for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
		{
			const FbxSurfaceMaterial* fbxSurfaceMaterial{ fbxNode->GetMaterial(materialIndex) };

			Material material;
			material.name_ = fbxSurfaceMaterial->GetName();
			material.uniqueID_ = fbxSurfaceMaterial->GetUniqueID();
			FbxProperty fbxProperty;

			fbxProperty = fbxSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
			if (fbxProperty.IsValid())
			{
				const FbxDouble3 color{ fbxProperty.Get<FbxDouble3>() };
				material.Kd_.x = static_cast<float>(color[0]);
				material.Kd_.y = static_cast<float>(color[1]);
				material.Kd_.z = static_cast<float>(color[2]);
				material.Kd_.w = 1.0f;

				const FbxFileTexture* fbxTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
				material.textureFileNames_[0] = fbxTexture ? fbxTexture->GetRelativeFileName() : "";
			}
			//	NormalMap
			fbxProperty = fbxSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
			if (fbxProperty.IsValid())
			{
				const FbxFileTexture* fileTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
				material.textureFileNames_[1] = fileTexture ? fileTexture->GetRelativeFileName() : "";
			}
			//	Specular
			fbxProperty = fbxSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
			if (fbxProperty.IsValid())
			{
				const FbxDouble3 color{ fbxProperty.Get<FbxDouble3>() };
				material.Ks_.x = static_cast<float>(color[0]);
				material.Ks_.y = static_cast<float>(color[1]);
				material.Ks_.z = static_cast<float>(color[2]);
				material.Ks_.w = 1.0f;

				const FbxFileTexture* fileTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
				material.textureFileNames_[2] = fileTexture ? fileTexture->GetRelativeFileName() : "";
			}
			//	Ambient
			fbxProperty = fbxSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sAmbient);
			if (fbxProperty.IsValid())
			{
				const FbxDouble3 color{ fbxProperty.Get<FbxDouble3>() };
				material.Ka_.x = static_cast<float>(color[0]);
				material.Ka_.y = static_cast<float>(color[1]);
				material.Ka_.z = static_cast<float>(color[2]);
				material.Ka_.w = 1.0f;

				const FbxFileTexture* fileTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
				material.textureFileNames_[3] = fileTexture ? fileTexture->GetRelativeFileName() : "";
			}
			//	Emissive
			fbxProperty = fbxSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sEmissive);
			if (fbxProperty.IsValid())
			{
				const FbxFileTexture* fileTexture{ fbxProperty.GetSrcObject<FbxFileTexture>() };
				material.textureFileNames_[4] = fileTexture ? fileTexture->GetRelativeFileName() : "";
			}
			materials.emplace(material.uniqueID_, std::move(material));

		}
	}
	materials.emplace();	//	最初のマテリアルが読み込まれる
}

void Model::FetchSkeleton(FbxMesh* fbxMesh, Skeleton& bindPose)
{
	const int deformerCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int deformerIndex = 0; deformerIndex < deformerCount; ++deformerIndex)
	{
		FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		const int clusterCount = skin->GetClusterCount();
		bindPose.bones_.resize(clusterCount);
		for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
		{
			FbxCluster* cluster = skin->GetCluster(clusterIndex);

			Skeleton::Bone& bone{ bindPose.bones_.at(clusterIndex) };
			bone.name_ = cluster->GetLink()->GetName();
			bone.uniqueID_ = cluster->GetLink()->GetUniqueID();
			bone.parentIndex_ = bindPose.indexof(cluster->GetLink()->GetParent()->GetUniqueID());
			bone.nodeIndex_ = sceneView_.indexof(bone.uniqueID_);

			//'reference_global_init_position' is used to convert from local space of model(mesh) to
		    // global space of scene.
			FbxAMatrix referenceGlobalInitPosition;
			cluster->GetTransformMatrix(referenceGlobalInitPosition);

			// 'cluster_global_init_position' is used to convert from local space of bone to
			// global space of scene.
			FbxAMatrix clusterGlobalInitPosition;
			cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);

			// Matrices are defined using the Column Major scheme. When a FbxAMatrix represents a transformation
			// (translation, rotation and scale), the last row of the matrix represents the translation part of
			//the transformation.
			// Compose 'bone.offset_transform' matrix that trnasforms position from mesh space to bone space.
			// This matrix is called the offset matrix.
			bone.offsetTransform_ = ToXMFLOAT4X4(clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition);

		}
	}
}

void Model::FetchAnimations(FbxScene* fbxScene, std::vector<Animation>& animationClips,
	float samplingRate /*If this value is 0, the animation data will be sampled at the default frame rate.*/)
{
	FbxArray<FbxString*>animationStackNames;
	fbxScene->FillAnimStackNameArray(animationStackNames);
	const int animationStackCount{ animationStackNames.GetCount() };
	for (int animationStackIndex = 0; animationStackIndex < animationStackCount; ++animationStackIndex)
	{
		Animation& animationClip{ animationClips.emplace_back() };
		animationClip.name_ = animationStackNames[animationStackIndex]->Buffer();

		//	アニメーションスタックを取得
		FbxAnimStack* animationStack{ fbxScene->FindMember<FbxAnimStack>(animationClip.name_.c_str()) };
		fbxScene->SetCurrentAnimationStack(animationStack);

		//	アニメーションのサンプリングレートを設定
		const FbxTime::EMode timeMode{ fbxScene->GetGlobalSettings().GetTimeMode() };
		FbxTime oneSecond;
		oneSecond.SetTime(0, 0, 1, 0, 0, timeMode);
		animationClip.samplingRate_ = samplingRate > 0 ? samplingRate : static_cast<float>(oneSecond.GetFrameRate(timeMode));
		const FbxTime samplingInterval{ static_cast<FbxLongLong>(oneSecond.Get() / animationClip.samplingRate_) };

		//	アニメーションの時間範囲を取得
		const FbxTakeInfo* takeInfo{ fbxScene->GetTakeInfo(animationClip.name_.c_str()) };
		const FbxTime startTime{ takeInfo->mLocalTimeSpan.GetStart() };
		const FbxTime stopTime{ takeInfo->mLocalTimeSpan.GetStop() };
		//	アニメーションの各フレームに対して処理を行う
		for (FbxTime time = startTime; time < stopTime; time += samplingInterval)
		{
			Animation::Keyframe& keyframe{ animationClip.sequence_.emplace_back() };

			//	各ノードの変換行列を取得し、アニメーションキーフレームに追加する
			const size_t nodeCount{ sceneView_.nodes_.size() };
			keyframe.nodes_.resize(nodeCount);
			for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
			{
				FbxNode* fbxNode{ fbxScene->FindNodeByName(sceneView_.nodes_.at(nodeIndex).name_.c_str()) };
				if (fbxNode)
				{
					Animation::Keyframe::Node& node{ keyframe.nodes_.at(nodeIndex) };

					// 'global_transform' is a transformation matrix of a node with respect to
					// the scene's global coordinate system.
					node.globalTransform_ = ToXMFLOAT4X4(fbxNode->EvaluateGlobalTransform(time));
					
					//'local_transform' is a transformation matrix of a node with respect to
					//its parent's local coordinate system.
					const FbxAMatrix& localTransform{ fbxNode->EvaluateLocalTransform(time) };
					node.scaling_ = ToXMFLOAT3(localTransform.GetS());
					node.rotation_ = ToXMFLOAT4(localTransform.GetQ());
					node.translation_ = ToXMFLOAT3(localTransform.GetT());

				}
			}
		}
	}
	for (int animationStackIndex = 0; animationStackIndex < animationStackCount; ++animationStackIndex)
	{
		delete animationStackNames[animationStackIndex];
	}
}

//	ボーン影響度をFBXから取得する
void FetchBoneInfluences(const FbxMesh* fbxMesh,
	std::vector<BoneInfluencesPerControlPoint>& boneInfluences)
{
	const int controlPointsCount{ fbxMesh->GetControlPointsCount() };
	boneInfluences.resize(controlPointsCount);

	const int skinCount{ fbxMesh->GetDeformerCount(FbxDeformer::eSkin) };
	for (int skinIndex = 0; skinIndex < skinCount; ++skinIndex)
	{
		const FbxSkin* fbxSkin
		{ static_cast<FbxSkin*>(fbxMesh->GetDeformer(skinIndex,FbxDeformer::eSkin)) };
		
		const int clusterCount{ fbxSkin->GetClusterCount() };
		for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
		{
			const FbxCluster* fbxCluster{ fbxSkin->GetCluster(clusterIndex) };

			const int controlPointIndicesCount{ fbxCluster->GetControlPointIndicesCount() };
			for (int controlPointIndicesIndex = 0; controlPointIndicesIndex < controlPointIndicesCount;
				++controlPointIndicesIndex)
			{
				int controlPointIndex{ fbxCluster->GetControlPointIndices() [controlPointIndicesIndex ] };
				double controlPointWeight
				{ fbxCluster->GetControlPointWeights()[controlPointIndicesIndex] };
				BoneInfluence& boneInfluence{ boneInfluences.at(controlPointIndex).emplace_back() };
				boneInfluence.boneIndex_ = static_cast<uint32_t>(clusterIndex);
				boneInfluence.boneWeight_ = static_cast<float>(controlPointWeight);
			}
		}
	
	}
}

void Model::PlayAnimation(int index, bool loop, float speed, float blendSeconds)
{
	//	現在のアニメーション番号とセットするアニメーション番号が同じならreturn
	if (currentAnimationIndex_ == index)
	{
		return;
	}

	currentAnimationIndex_		= index;			//	現在のアニメーション番号
	currentAnimationSeconds_	= 0.0f;				//	現在のアニメーション再生時間
	animationLoopFlag_			= loop;				//	アニメーションループ再生フラグ
	animationSpeed_				= speed;			//	アニメーション再生フラグ
	animationEndFlag_			= false;			//	アニメーション終了フラグ
	animationBlendTime_			= 0.0f;				//	ブレンドしてから今まで何秒経過したか	
	animationBlendSeconds_		= blendSeconds;		//	現在の姿勢から次のアニメーションの姿勢までどのくらい時間をかけるか
	blendAnimationIndex1_		= -1;				//	ブレンドさせるアニメーション番号1
	blendAnimationIndex2_		= -1;				//	ブレンドさせるアニメーション番号2

}

void Model::PlayBlendAnimation(int index1, int index2, bool loop, float speed)
{
	//	現在のアニメーション番号とセットするアニメーション番号が同じ場合はreturn
	if (blendAnimationIndex1_ == index1 && blendAnimationIndex2_ == index2)
	{
		return;
	}

	currentAnimationIndex_		= -1;				//	現在のアニメーション番号
	currentAnimationSeconds_	= 0.0f;				//	現在のアニメーション再生時間リセット
	blendAnimationIndex1_		= index1;			//	ブレンド元のアニメーション番号セット
	blendAnimationIndex2_		= index2;			//	ブレンド先のアニメーション番号セット
	animationLoopFlag_			= loop;				//	ループ再生フラグセット
	animationEndFlag_			= false;			//	アニメーション終了フラグリセット
	animationSpeed_				= speed;			//	アニメーション再生フラグ
	animationBlendTime_			= 0.0f;				//	ブレンドしてから今まで何秒経過したか
	animationBlendSeconds_ = 0.0f;
	
}

//	プログラム側でアニメーションを変更したときに、更新するための関数
void Model::UpdateAnimation(Animation::Keyframe& keyframe)
{
	size_t nodeCount{ keyframe.nodes_.size() };
	for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
	{
		Animation::Keyframe::Node& node{ keyframe.nodes_.at(nodeIndex) };
		DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(node.scaling_.x, node.scaling_.y, node.scaling_.z) };
		DirectX::XMMATRIX R{ DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&node.rotation_)) };
		DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(node.translation_.x, node.translation_.y, node.translation_.z) };

		int64_t parentIndex{ sceneView_.nodes_.at(nodeIndex).parentIndex_ };
		DirectX::XMMATRIX P{ parentIndex < 0 ? DirectX::XMMatrixIdentity() :
			DirectX::XMLoadFloat4x4(&keyframe.nodes_.at(parentIndex).globalTransform_) };

		XMStoreFloat4x4(&node.globalTransform_, S * R * T * P);
	}
}

//	アニメーション更新処理
void Model::UpdateAnimation(const float& elapsedTime)
{
	//	再生中でないなら処理しない
	if (!IsPlayAnimation()) return;

	//	最終フレーム処理(再生終了フラグが立っていたら再生終了)
	if (animationEndFlag_)
	{
		animationEndFlag_		= false;	//	終了フラグをリセット
		currentAnimationIndex_	= -1;		//	アニメーション番号リセット
		return;
	}

	//	ブレンドアニメーションの再生（再生中の場合これ以降の通常再生の処理はしない）
	if (UpdateBlendAnimation(elapsedTime)) return;

	//	アニメーション再生時間経過
	currentAnimationSeconds_ += elapsedTime;

	//	指定のアニメーションデータを取得
	Animation& animation = animationClips_.at(currentAnimationIndex_);

	//	現在のフレームを取得
	const float  frameIndexFloat = (currentAnimationSeconds_ * animation.samplingRate_) * animationSpeed_;	//	警告がじゃまなので一時的にfloat変数に格納
	const size_t frameIndex = static_cast<size_t>(frameIndexFloat);

	//	外部から現在のフレームを取ってこれるようにする
	currentKeyframeIndex_ = static_cast<int>(frameIndex);

	//	最後のフレームを取得
	const size_t frameEnd = (animation.sequence_.size() - 1);


	//	アニメーションが再生しきっていた場合
	if (frameIndex > frameEnd)
	{
		//	ループフラグが立っていれば再生時間を巻き戻す
		if (animationLoopFlag_)
		{
			currentAnimationSeconds_ = 0.0f;
			return;
		}
		//	ループなしなら再生終了フラグを立てる
		else
		{
			animationEndFlag_ = true;
			return;
		}
	}
	//	キーフレームが更新されていてアニメーションが再生しきっていないときはアニメーションをスムーズに切り替える
	else if (isDebugBlendAnimation_ && (keyframe_.nodes_.size() > 0) && frameIndex < frameEnd)
	{
		// ブレンド率の計算
		float blendRate = 1.0f;
		UpdateBlendRate(blendRate, elapsedTime);

		// キーフレーム取得
		const std::vector<Animation::Keyframe>& keyframes = animation.sequence_;

		// 現在の前後のキーフレームを取得
		const Animation::Keyframe* keyframeArr[2] = {
			&keyframe_,
			&keyframes.at(frameIndex + 1)
		};

		//	アニメーションを滑らかに切り替える
		BlendAnimations(keyframeArr, blendRate, keyframe_);

		//	アニメーショントランスフォーム更新
		UpdateAnimation(keyframe_);
	}
	//	キーフレームが一度も更新されていなくてアニメーションが再生しきっていなければ現在のフレームを保存
	else
	{
		keyframe_ = animation.sequence_.at(frameIndex);
	}

}

//	ブレンドアニメーション更新処理
bool Model::UpdateBlendAnimation(const float& elapsedTime)
{
	//	ブレンドするアニメーションが無かったら処理しない
	if (blendAnimationIndex1_ < 0)return false;
	if (blendAnimationIndex2_ < 0)return false;

	if (animationEndFlag_)
	{
		animationEndFlag_ = false;		//	終了フラグをリセット
		blendAnimationIndex1_ = -1;		//	アニメーション番号リセット
		blendAnimationIndex2_ = -1;
		// TODO:アニメーションブレンド時間リセット(animationBlendSeconds_)
		animationBlendSeconds_ = 0.0f;
		animationBlendTime_ = 0.0f;
		currentAnimationSeconds_ = 0.0f;
		weight_ = 0.0f;


		return false;
	}

	if (weight_ < 0)weight_ = 0;
	else if (weight_ > 1)weight_ = 1;

	//	アニメーション再生時間更新
	animationBlendSeconds_ += elapsedTime;

	Animation& bAnimation1 = animationClips_.at(blendAnimationIndex1_);
	Animation& bAnimation2 = animationClips_.at(blendAnimationIndex2_);

	const size_t frameCount1 = bAnimation1.sequence_.size();
	const size_t frameCount2 = bAnimation2.sequence_.size();

	//	基準となるアニメーションにフレーム数を合わせる
	size_t maxFrameCount = frameCount1;
	if (blendThreshold_ < weight_)maxFrameCount = frameCount2;

	const float frameIndex = (animationBlendSeconds_ * bAnimation1.samplingRate_) * animationSpeed_;

	//	アニメーションが再生しきっている場合
	if (frameIndex > maxFrameCount)
	{
		if (animationLoopFlag_)
		{
			animationBlendSeconds_ = 0.0f;
			return true;
		}
		else
		{
			animationEndFlag_ = true;
			return true;
		}
	}

	//	再生フレームを正規化して再生時間の長さを合わせる
	UINT64 frameIndex1 = static_cast<UINT64>(frameIndex / maxFrameCount * (frameCount1 - 1));
	UINT64 frameIndex2 = static_cast<UINT64>(frameIndex / maxFrameCount * (frameCount2 - 1));

	blendRate_ = 1.0f;
	if (animationBlendTime_ < animationBlendSeconds_)
	{
		animationBlendTime_ += elapsedTime;
		if (animationBlendTime_ >= animationBlendSeconds_)
		{
			animationBlendTime_ = animationBlendSeconds_;
		}
		blendRate_ = animationBlendTime_ / animationBlendSeconds_;
		blendRate_ *= blendRate_;
	}

	//	キーフレーム取得
	const std::vector<Animation::Keyframe>& keyframes1 = bAnimation1.sequence_;

	//	現在の前後のキーフレームを取得
	const Animation::Keyframe* keyframeArr1[2] = {
		&keyframes1.at(frameIndex1),
		&keyframes1.at(frameIndex1 + 1)
	};

	//	アニメーションを滑らかに切り替える
	Animation::Keyframe keyframe1;
	BlendAnimations(keyframeArr1, blendRate_, keyframe1);

	//	キーフレーム取得
	const std::vector<Animation::Keyframe>& keyframes2 = bAnimation2.sequence_;

	//	現在の前後のキーフレームを取得
	const Animation::Keyframe* keyframeArr2[2] = {
		&keyframes2.at(frameIndex2),
		&keyframes2.at(frameIndex2 + 1)
	};

	//	アニメーションを滑らかに切り替える
	Animation::Keyframe keyframe2;
	BlendAnimations(keyframeArr2, blendRate_, keyframe2);

	const Animation::Keyframe* resultKeyframeArr[2] = { &keyframe1 ,&keyframe2 };
	BlendAnimations(resultKeyframeArr, weight_, keyframe_);

	UpdateAnimation(keyframe_);
	
}

//	ブレンド率の更新処理
void Model::UpdateBlendRate(float blendRate, const float& elapsedTime)
{
	if (animationBlendTime_ < animationBlendSeconds_)
	{
		animationBlendTime_ += elapsedTime;
		if (animationBlendTime_ >= animationBlendSeconds_)
		{
			animationBlendTime_ = animationBlendSeconds_;
		}
		blendRate = animationBlendTime_ / animationBlendSeconds_;
		blendRate *= blendRate;
	}
}

//	アニメーションだけ別のFBXデータでもらった時に追加する
bool Model::AppendAnimations(const char* animationFileName, float samplingRate)
{
	FbxManager* fbxManager{ FbxManager::Create() };
	FbxScene* fbxScene{ FbxScene::Create(fbxManager,"") };

	FbxImporter* fbxImporter{ FbxImporter::Create(fbxManager,"") };
	bool importStatus{ false };
	importStatus = fbxImporter->Initialize(animationFileName);
	_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());
	importStatus = fbxImporter->Import(fbxScene);
	_ASSERT_EXPR_A(importStatus, fbxImporter->GetStatus().GetErrorString());

	FetchAnimations(fbxScene, animationClips_, samplingRate);

	fbxManager->Destroy();

	return true;
}

//	アニメーションブレンド
void Model::BlendAnimations(const Animation::Keyframe* keyframes[2], float factor, Animation::Keyframe& keyframe)
{
	size_t nodeCount{ keyframes[0]->nodes_.size() };
	keyframe.nodes_.resize(nodeCount);
	for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
	{
		DirectX::XMVECTOR S[2]{
			XMLoadFloat3(&keyframes[0]->nodes_.at(nodeIndex).scaling_),
			XMLoadFloat3(&keyframes[1]->nodes_.at(nodeIndex).scaling_) };
		XMStoreFloat3(&keyframe.nodes_.at(nodeIndex).scaling_, DirectX::XMVectorLerp(S[0], S[1], factor));

		DirectX::XMVECTOR R[2]{
			XMLoadFloat4(&keyframes[0]->nodes_.at(nodeIndex).rotation_),
			XMLoadFloat4(&keyframes[1]->nodes_.at(nodeIndex).rotation_) };
		XMStoreFloat4(&keyframe.nodes_.at(nodeIndex).rotation_, DirectX::XMQuaternionSlerp(R[0], R[1], factor));

		DirectX::XMVECTOR T[2]{
			XMLoadFloat3(&keyframes[0]->nodes_.at(nodeIndex).translation_),
			XMLoadFloat3(&keyframes[1]->nodes_.at(nodeIndex).translation_) };
		XMStoreFloat3(&keyframe.nodes_.at(nodeIndex).translation_, DirectX::XMVectorLerp(T[0], T[1], factor));
	}
}

//	アニメーション再生中かどうか
bool Model::IsPlayAnimation()const
{
	if (currentAnimationIndex_ < 0 && blendAnimationIndex1_ < 0) return false;

	const int animationIndexEnd = static_cast<int>(animationClips_.size());
	if (currentAnimationIndex_ >= animationIndexEnd && blendAnimationIndex1_ >= animationIndexEnd) return false;

	return true;

}

//	ジョイントポジション取得
DirectX::XMFLOAT3 Model::GetJointPosition(
	const std::string& meshName,
	const std::string& boneName,
	const Animation::Keyframe* keyframe,
	const DirectX::XMFLOAT4X4& transform)
{
	DirectX::XMFLOAT3 position = {};/*world space*/

	for (const Model::Mesh& mesh : meshes_)
	{
		if (mesh.name_ == meshName)
		{
			for (const Skeleton::Bone& bone : mesh.bindPose_.bones_)
			{
				if (bone.name_ == boneName)
				{
					const Animation::Keyframe::Node& node = keyframe->nodes_.at(bone.nodeIndex_);
					DirectX::XMFLOAT4X4 globalTransform = node.globalTransform_;
					position.x = globalTransform._41;
					position.y = globalTransform._42;
					position.z = globalTransform._43;
					DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform_) * DirectX::XMLoadFloat4x4(&transform);
					DirectX::XMStoreFloat3(&position, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&position), M));
					return position;
				}
			}
		}
	}
	_ASSERT_EXPR(FALSE, L"Joint not found.");

	return {};
}

//	ジョイントポジション取得
DirectX::XMFLOAT3 Model::GetJointPosition(
	const std::string& meshName,
	const std::string& boneName,
	const DirectX::XMFLOAT4X4& transform)
{
	DirectX::XMFLOAT3 position = {};/*world space*/

	for (const Model::Mesh& mesh : meshes_)
	{
		if (mesh.name_ == meshName)
		{
			for (const Skeleton::Bone& bone : mesh.bindPose_.bones_)
			{
				if (bone.name_ == boneName)
				{
					const Animation::Keyframe::Node& node = keyframe_.nodes_.at(bone.nodeIndex_);
					DirectX::XMFLOAT4X4 globalTransform = node.globalTransform_;
					position.x = globalTransform._41;
					position.y = globalTransform._42;
					position.z = globalTransform._43;
					DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform_) * DirectX::XMLoadFloat4x4(&transform);
					DirectX::XMStoreFloat3(&position, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&position), M));
					return position;
				}
			}
		}
	}
	_ASSERT_EXPR(FALSE, L"Joint not found.");

	return {};
}

//	ジョイントポジション取得
DirectX::XMFLOAT3 Model::GetJointPosition(
	size_t meshIndex,
	size_t boneIndex,
	const Animation::Keyframe* keyframe,
	const DirectX::XMFLOAT4X4& transform)
{
	DirectX::XMFLOAT3 position = {};/*world space*/

	const Model::Mesh& mesh = meshes_.at(meshIndex);
	const Skeleton::Bone& bone = mesh.bindPose_.bones_.at(boneIndex);
	const Animation::Keyframe::Node& node = keyframe->nodes_.at(bone.nodeIndex_);
	DirectX::XMFLOAT4X4 globalTransform = node.globalTransform_;
	position.x = globalTransform._41;
	position.y = globalTransform._42;
	position.z = globalTransform._43;
	DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform_) * DirectX::XMLoadFloat4x4(&transform);
	DirectX::XMStoreFloat3(&position, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&position), M));
	return position;
}

//	描画処理
void Model::Render(const DirectX::XMMATRIX& world, const DirectX::XMFLOAT4& materialColor,
	const Animation::Keyframe* keyframe, ID3D11PixelShader** replacementPixelShader/*SHADOW*/)
{
	D3D11_VIEWPORT viewport{};
	UINT numViewports{ 1 };

	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
	deviceContext->RSGetViewports(&numViewports, &viewport);

	for (const Mesh& mesh : meshes_)
	{
		uint32_t stride{ sizeof(Vertex) };
		uint32_t offset{ 0 };
		deviceContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer_.GetAddressOf(), &stride, &offset);
		deviceContext->IASetIndexBuffer(mesh.indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->IASetInputLayout(inputLayout_.Get());
		deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
		deviceContext->PSSetShader((replacementPixelShader ? *replacementPixelShader : pixelShader_.Get()/*SHADOW*/), nullptr, 0);
		//graphics.GetDeviceContext()->PSSetShader(pixelShader_.Get(), nullptr, 0);

		Constants data;
		if (&keyframe_ && keyframe_.nodes_.size() > 0)
		{
			const Animation::Keyframe::Node& meshNode{ keyframe_.nodes_.at(mesh.nodeIndex_) };
			XMStoreFloat4x4(&data.world_, XMLoadFloat4x4(&meshNode.globalTransform_) * world);

			const size_t boneCount{ mesh.bindPose_.bones_.size() };
			_ASSERT_EXPR(boneCount < MAX_BONES, L"The value of the 'bone_count' has exceeded MAX_BONES.");

			for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex)
			{
				const Skeleton::Bone& bone{ mesh.bindPose_.bones_.at(boneIndex) };
				const Animation::Keyframe::Node& boneNode{ keyframe_.nodes_.at(bone.nodeIndex_) };
				XMStoreFloat4x4(&data.boneTransforms_[boneIndex],
					XMLoadFloat4x4(&bone.offsetTransform_) *
					XMLoadFloat4x4(&boneNode.globalTransform_) *
					XMMatrixInverse(nullptr, XMLoadFloat4x4(&mesh.defaultGlobalTransform_))
				);
			}
		}
		else
		{
			DirectX::XMStoreFloat4x4(&data.world_,
				XMLoadFloat4x4(&mesh.defaultGlobalTransform_) * world);
			for (size_t boneIndex = 0; boneIndex < MAX_BONES; ++boneIndex)
			{
				data.boneTransforms_[boneIndex] = { 1,0,0,0,
													0,1,0,0,
													0,0,1,0,
													0,0,0,1 };
			}
		}
		for (const Mesh::Subset& subset : mesh.subsets_)
		{
			const Material& material{ materials_.at(subset.materialUniqueID_) };
			XMStoreFloat4(&data.materialColor_, DirectX::XMVectorMultiply(XMLoadFloat4(&materialColor), XMLoadFloat4(&material.Kd_)));
			deviceContext->UpdateSubresource(constantBufer_.Get(), 0, 0, &data, 0, 0);
			deviceContext->VSSetConstantBuffers(0, 1, constantBufer_.GetAddressOf());
			deviceContext->PSSetShaderResources(0, 1, material.shaderResourceViews_[0].GetAddressOf());
			deviceContext->PSSetShaderResources(1, 1, material.shaderResourceViews_[1].GetAddressOf());
			deviceContext->PSSetShaderResources(2, 1, material.shaderResourceViews_[4].GetAddressOf());

			deviceContext->DrawIndexed(subset.indexCount_, subset.startIndexLocation_, 0);
		}
	}
}

// The coordinate system of all function arguments is world space.
bool Model::Raycast(const DirectX::XMFLOAT3& position/*ray position*/, const DirectX::XMFLOAT3& direction/*ray direction*/, const DirectX::XMFLOAT4X4& worldTransform, 
	DirectX::XMFLOAT4& closesPoint, DirectX::XMFLOAT3& intersectedNormal,
	std::string& intersectedMesh, std::string& intersectedMaterial)
{
	float closestDistance{ FLT_MAX };

	for (const Mesh& mesh : meshes_)
	{
		DirectX::XMFLOAT3 rayPosition = position;
		DirectX::XMFLOAT3 rayDirection = direction;
		//ray_direction.w = 0;

		// Convert to model space.
		DirectX::XMMATRIX concatenatedMatrix{
			DirectX::XMLoadFloat4x4(&mesh.defaultGlobalTransform_) *
			DirectX::XMLoadFloat4x4(&worldTransform) };
		DirectX::XMMATRIX inverseConcatenatedMatrix{ XMMatrixInverse(nullptr, concatenatedMatrix) };
		XMStoreFloat3(&rayPosition, DirectX::XMVector3TransformCoord(XMLoadFloat3(&rayPosition), inverseConcatenatedMatrix));
		XMStoreFloat3(&rayDirection, DirectX::XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&rayDirection), inverseConcatenatedMatrix)));

#if 0
		const float* min{ reinterpret_cast<const float*>(&mesh.boundingBox_[0]) };
		const float* max{ reinterpret_cast<const float*>(&mesh.boundingBox_[1]) };
		if (!IntersectRayAABB(reinterpret_cast<const float*>(&rayPosition), reinterpret_cast<const float*>(&rayDirection), min, max))
		{
			continue;
		}
#endif

		float distance{ 1.0e+7f };
		DirectX::XMFLOAT4 intersection{};
		const float* vertexPositions{ reinterpret_cast<const float*>(mesh.vertices_.data()) };
		const uint32_t* indices{ mesh.indices_.data() };
		const size_t indexCount{ mesh.indices_.size() };

		const int intersectedTriangleIndex{ IntersectRayTriangles(vertexPositions, 0L, sizeof(Vertex), indices, indexCount, rayPosition, rayDirection, intersection, distance) };
		if (intersectedTriangleIndex >= 0)
		{
			if (closestDistance > distance)
			{
				closestDistance = distance;
				// Convert model space to original space.
				XMStoreFloat4(&closesPoint, XMVector3TransformCoord(XMLoadFloat4(&intersection), concatenatedMatrix));
				intersectedMesh = mesh.name_;
				intersectedMaterial = mesh.findSubset(intersectedTriangleIndex * 3)->materialName_;
				intersectedNormal = mesh.vertices_.at(indices[intersectedTriangleIndex + 0]).normal_;
			}
		}
	}
	return closestDistance < FLT_MAX;
}

//	デバッグ描画
void Model::DrawDebug()
{
	int animationIndexEnd = static_cast<int>(animationClips_.size());

	if (ImGui::TreeNode(u8"Model"))
	{
		ImGui::InputInt("CurrentAnimationIndex", &currentAnimationIndex_);					//	現在のアニメーション番号
		ImGui::InputInt("BlendAnimation1", &blendAnimationIndex1_);							//	現在のアニメーション番号
		ImGui::InputInt("BlendAnimation2", &blendAnimationIndex2_);							//	現在のアニメーション番号
		ImGui::InputInt("animationIndexEnd", &animationIndexEnd);							//
		ImGui::TreePop();
	}
}