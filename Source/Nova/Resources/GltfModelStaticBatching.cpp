#include "GltfModelStaticBatching.h"

#include <stack>
#include <functional>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "../Others/Misc.h"
#include "../Graphics/Graphics.h"
#include "../Resources/Texture.h"

#define USE_SERIALIZE 1

GltfModelStaticBatching::GltfModelStaticBatching(const std::string& filename, const bool setColor, const DirectX::XMFLOAT4 color) : filename_(filename)
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
#if USE_SERIALIZE
	std::filesystem::path cerealFilename(filename);
	cerealFilename.replace_extension("cereal");
	if (std::filesystem::exists(cerealFilename.c_str()))
	{
		std::ifstream ifs(cerealFilename.c_str(), std::ios::binary);
		cereal::BinaryInputArchive deserialization(ifs);
		deserialization(scenes_, nodes_, primitives_, materials_, textures_, images_);

		HRESULT hr;
		D3D11_TEXTURE2D_DESC texture2dDesc;
		for (int i = 0; i < images_.size(); ++i)
		{
			ID3D11ShaderResourceView* shaderResourceView{};
			std::wstring filename = images_.at(i).filename_;
			hr = LoadTextureFromFile(device, filename.c_str(), &shaderResourceView, &texture2dDesc);
			if (hr == S_OK)
			{
				textureResourceViews_.emplace_back().Attach(shaderResourceView);
			}
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		for (int primitiveIndex = 0; primitiveIndex < primitives_.size(); ++primitiveIndex)
		{
			const BufferView& indexBufferView = primitives_.at(primitiveIndex).indexBufferView_;
			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.ByteWidth = static_cast<UINT>(indexBufferView.sizeInBytes_);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			D3D11_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pSysMem = indexBufferView.verticesBinary_.data();
			hr = device->CreateBuffer(&bufferDesc, &subresourceData,
				primitives_.at(primitiveIndex).indexBufferView_.buffer_.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			for (auto& vertexBufferView : primitives_.at(primitiveIndex).vertexBufferViews_)
			{
				if (static_cast<UINT>(vertexBufferView.second.sizeInBytes_) == 0)
				{
					continue;
				}
				bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.second.sizeInBytes_);
				bufferDesc.Usage = D3D11_USAGE_DEFAULT;
				bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				subresourceData.pSysMem = vertexBufferView.second.verticesBinary_.data();
				hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBufferView.second.buffer_.ReleaseAndGetAddressOf());
				if (FAILED(hr))
				{
					_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
				}
			}

		}

		std::vector<Material::Cbuffer> materialData;
		for (std::vector<Material>::const_reference material : materials_)
		{
			materialData.emplace_back(material.data_);
		}
		Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer;
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Material::Cbuffer) * materialData.size());
		bufferDesc.StructureByteStride = sizeof(Material::Cbuffer);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		D3D11_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pSysMem = materialData.data();
		hr = device->CreateBuffer(&bufferDesc, &subresourceData, materialBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(materialData.size());
		hr = device->CreateShaderResourceView(materialBuffer.Get(), &shaderResourceViewDesc, materialResourceView_.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	else
	{
#endif
		tinygltf::TinyGLTF tinyGltf;
		tinyGltf.SetImageLoader(NullLoadImageData, nullptr);

		tinygltf::Model gltfModel;
		std::string error, warning;
		bool succeeded{ false };
		if (filename.find(".glb") != std::string::npos)
		{
			succeeded = tinyGltf.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str());
		}
		else if (filename.find(".gltf") != std::string::npos)
		{
			succeeded = tinyGltf.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());
		}

		_ASSERT_EXPR_A(warning.empty(), warning.c_str());
		_ASSERT_EXPR_A(error.empty(), error.c_str());
		_ASSERT_EXPR_A(succeeded, L"Failed to load glTF file");

		for (std::vector<tinygltf::Scene>::const_reference gltfScene : gltfModel.scenes)
		{
			Scene& scene{ scenes_.emplace_back() };
			scene.name_ = gltfModel.scenes.at(0).name;
			scene.nodes_ = gltfModel.scenes.at(0).nodes;
		}

		FetchNodes(gltfModel);
		FetchMeshes(device, gltfModel);
		FetchMaterials(device, gltfModel, setColor, color);
		FetchTextures(device, gltfModel);

#if USE_SERIALIZE
		std::ofstream ofs(cerealFilename.c_str(), std::ios::binary);
		cereal::BinaryOutputArchive serialization(ofs);
		serialization(scenes_, nodes_, primitives_, materials_, textures_, images_);

	}

#endif
	const std::map<std::string, BufferView>& vertexBufferViews{ primitives_.at(0).vertexBufferViews_ };
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
	{
		{ "POSITION", 0, vertexBufferViews.at("POSITION").format_, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, vertexBufferViews.at("NORMAL").format_, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, vertexBufferViews.at("TANGENT").format_, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, vertexBufferViews.at("TEXCOORD_0").format_, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	Graphics::Instance().GetShader()->CreateVsFromCso(device, "./Resources/Shader/GltfModelStaticBatchingVs.cso", vertexShader_.ReleaseAndGetAddressOf(), inputLayout_.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
	Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/GltfModelPs.cso", pixelShader_.ReleaseAndGetAddressOf());

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(PrimitiveConstants);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr;
	hr = device->CreateBuffer(&bufferDesc, nullptr, primitiveCbuffer_.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void GltfModelStaticBatching::FetchNodes(const tinygltf::Model& gltfModel)
{
	for (std::vector<tinygltf::Node>::const_reference gltfNode : gltfModel.nodes)
	{
		Node& node{ nodes_.emplace_back() };
		node.name_ = gltfNode.name;
		node.skin_ = gltfNode.skin;
		node.mesh_ = gltfNode.mesh;
		node.children_ = gltfNode.children;
		if (!gltfNode.matrix.empty())
		{
			DirectX::XMFLOAT4X4 matrix;
			for (size_t row = 0; row < 4; row++)
			{
				for (size_t column = 0; column < 4; column++)
				{
					matrix(row, column) = static_cast<float>(gltfNode.matrix.at(4 * row + column));
				}
			}

			DirectX::XMVECTOR S, T, R;
			bool succeed = DirectX::XMMatrixDecompose(&S, &R, &T, DirectX::XMLoadFloat4x4(&matrix));
			_ASSERT_EXPR(succeed, L"Failed to decompose matrix.");

			DirectX::XMStoreFloat3(&node.scale_, S);
			DirectX::XMStoreFloat4(&node.rotation_, R);
			DirectX::XMStoreFloat3(&node.translation_, T);
		}
		else
		{
			if (gltfNode.scale.size() > 0)
			{
				node.scale_.x = static_cast<float>(gltfNode.scale.at(0));
				node.scale_.y = static_cast<float>(gltfNode.scale.at(1));
				node.scale_.z = static_cast<float>(gltfNode.scale.at(2));
			}
			if (gltfNode.translation.size() > 0)
			{
				node.translation_.x = static_cast<float>(gltfNode.translation.at(0));
				node.translation_.y = static_cast<float>(gltfNode.translation.at(1));
				node.translation_.z = static_cast<float>(gltfNode.translation.at(2));
			}
			if (gltfNode.rotation.size() > 0)
			{
				node.rotation_.x = static_cast<float>(gltfNode.rotation.at(0));
				node.rotation_.y = static_cast<float>(gltfNode.rotation.at(1));
				node.rotation_.z = static_cast<float>(gltfNode.rotation.at(2));
				node.rotation_.w = static_cast<float>(gltfNode.rotation.at(3));
			}
		}
	}
	CumulateTransforms(nodes_);
}

void GltfModelStaticBatching::CumulateTransforms(std::vector<Node>& nodes)
{
	using namespace DirectX;

	std::stack<XMFLOAT4X4> parentGlobalTransforms;
	std::function<void(int)> traverse{ [&](int nodeIndex)->void
	{
		Node& node{nodes.at(nodeIndex)};
		XMMATRIX S{ XMMatrixScaling(node.scale_.x, node.scale_.y, node.scale_.z) };
		XMMATRIX R{ XMMatrixRotationQuaternion(XMVectorSet(node.rotation_.x, node.rotation_.y, node.rotation_.z, node.rotation_.w)) };
		XMMATRIX T{ XMMatrixTranslation(node.translation_.x, node.translation_.y, node.translation_.z) };
		XMStoreFloat4x4(&node.globalTransform_, S * R * T * XMLoadFloat4x4(&parentGlobalTransforms.top()));
		for (int childIndex : node.children_)
		{
			parentGlobalTransforms.push(node.globalTransform_);
			traverse(childIndex);
			parentGlobalTransforms.pop();
		}
	} };
	for (std::vector<int>::value_type nodeIndex : scenes_.at(0).nodes_)
	{
		parentGlobalTransforms.push({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 });
		traverse(nodeIndex);
		parentGlobalTransforms.pop();
	}
}

GltfModelStaticBatching::BufferView GltfModelStaticBatching::MakeBufferView(const tinygltf::Accessor& accessor)
{
	BufferView bufferView;
	switch (accessor.type)
	{
	case TINYGLTF_TYPE_SCALAR:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			bufferView.format_ = DXGI_FORMAT_R16_UINT;
			bufferView.strideInBytes_ = sizeof(USHORT);
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			bufferView.format_ = DXGI_FORMAT_R32_UINT;
			bufferView.strideInBytes_ = sizeof(UINT);
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;
	case TINYGLTF_TYPE_VEC2:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			bufferView.format_ = DXGI_FORMAT_R32G32_FLOAT;
			bufferView.strideInBytes_ = sizeof(FLOAT) * 2;
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;
	case TINYGLTF_TYPE_VEC3:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			bufferView.format_ = DXGI_FORMAT_R32G32B32_FLOAT;
			bufferView.strideInBytes_ = sizeof(FLOAT) * 3;
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;
	case TINYGLTF_TYPE_VEC4:
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			bufferView.format_ = DXGI_FORMAT_R16G16B16A16_UINT;
			bufferView.strideInBytes_ = sizeof(USHORT) * 4;
			break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			bufferView.format_ = DXGI_FORMAT_R32G32B32A32_UINT;
			bufferView.strideInBytes_ = sizeof(UINT) * 4;
			break;
		case TINYGLTF_COMPONENT_TYPE_FLOAT:
			bufferView.format_ = DXGI_FORMAT_R32G32B32A32_FLOAT;
			bufferView.strideInBytes_ = sizeof(FLOAT) * 4;
			break;
		default:
			_ASSERT_EXPR(FALSE, L"This accessor component type is not supported.");
			break;
		}
		break;
	default:
		_ASSERT_EXPR(FALSE, L"This accessor type is not supported.");
		break;
	}
	bufferView.sizeInBytes_ = static_cast<UINT>(accessor.count * bufferView.strideInBytes_);
	return bufferView;
}

void GltfModelStaticBatching::FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel)
{
	HRESULT hr = S_OK;

	struct CombinedBuffer
	{
		size_t indexCount;
		size_t vertexCount;

		D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		std::vector<unsigned int> indices;
		struct StructureOfArrays
		{
			std::vector<DirectX::XMFLOAT3> positions;
			std::vector<DirectX::XMFLOAT3> normals;
			std::vector<DirectX::XMFLOAT4> tangents;
			std::vector<DirectX::XMFLOAT2> texcoords;
		};
		StructureOfArrays vertices;
	};
	std::unordered_map<int/*Material*/, CombinedBuffer> combinedBuffers;

	//	Collect primitives_ with same Material
	for (decltype(nodes_)::reference node : nodes_)
	{
		const DirectX::XMMATRIX transform = XMLoadFloat4x4(&node.globalTransform_);

		if (node.mesh_ > -1)
		{
			const tinygltf::Mesh& gltfMesh = gltfModel.meshes.at(node.mesh_);

			for (std::vector<tinygltf::Primitive>::const_reference gltfPrimitive : gltfMesh.primitives)
			{
				CombinedBuffer& combinedBuffer = combinedBuffers[gltfPrimitive.material];
				if (gltfPrimitive.indices > -1)
				{
					const tinygltf::Accessor gltfAccessor = gltfModel.accessors.at(gltfPrimitive.indices);
					const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

					if (gltfAccessor.count == 0)
					{
						continue;
					}

					const size_t vertexOffset = combinedBuffer.vertices.positions.size();
					if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
					{
						const unsigned char* buffer = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
						for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex)
						{
							combinedBuffer.indices.emplace_back(static_cast<unsigned int>(buffer[accessorIndex] + vertexOffset));
						}
					}
					else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						const unsigned short* buffer = reinterpret_cast<const unsigned short*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
						for (size_t accessor_index = 0; accessor_index < gltfAccessor.count; ++accessor_index)
						{
							combinedBuffer.indices.emplace_back(static_cast<unsigned int>(buffer[accessor_index] + vertexOffset));
						}
					}
					else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
					{
						const unsigned int* buffer = reinterpret_cast<const unsigned int*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
						for (size_t accessor_index = 0; accessor_index < gltfAccessor.count; ++accessor_index)
						{
							combinedBuffer.indices.emplace_back(static_cast<unsigned int>(buffer[accessor_index] + vertexOffset));
						}
					}
					else
					{
						_ASSERT_EXPR(false, L"This index format is not supported.");
					}
				}

				//	Combine primitives_ using the same Material into a single vertex buffer_. In addition, apply a coordinate transformation to position, normal and tangent of primitives_.
				for (decltype(gltfPrimitive.attributes)::const_reference gltfAttribute : gltfPrimitive.attributes)
				{
					const tinygltf::Accessor gltfAccessor = gltfModel.accessors.at(gltfAttribute.second);
					const tinygltf::BufferView& gltfBufferView = gltfModel.bufferViews.at(gltfAccessor.bufferView);

					if (gltfAccessor.count == 0)
					{
						continue;
					}

					if (gltfAttribute.first == "POSITION")
					{
						_ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && gltfAccessor.type == TINYGLTF_TYPE_VEC3, L"'POSITION' attribute must be of type TINYGLTF_COMPONENT_TYPE_FLOAT & TINYGLTF_TYPE_VEC3.");
						const DirectX::XMFLOAT3* buffer = reinterpret_cast<const DirectX::XMFLOAT3*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
						for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex)
						{
							DirectX::XMFLOAT3 position = buffer[accessorIndex];
							DirectX::XMStoreFloat3(&position, XMVector3TransformCoord(XMLoadFloat3(&position), transform));
							combinedBuffer.vertices.positions.emplace_back(position);
						}
					}
					else if (gltfAttribute.first == "NORMAL")
					{
						_ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && gltfAccessor.type == TINYGLTF_TYPE_VEC3, L"'NORMAL' attribute must be of type TINYGLTF_COMPONENT_TYPE_FLOAT & TINYGLTF_TYPE_VEC3.");
						const DirectX::XMFLOAT3* buffer = reinterpret_cast<const DirectX::XMFLOAT3*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
						for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex)
						{
							DirectX::XMFLOAT3 normal = buffer[accessorIndex];
							DirectX::XMStoreFloat3(&normal, XMVector3TransformNormal(XMLoadFloat3(&normal), transform));
							combinedBuffer.vertices.normals.emplace_back(normal);
						}
					}
					else if (gltfAttribute.first == "TANGENT")
					{
						_ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && gltfAccessor.type == TINYGLTF_TYPE_VEC4, L"'TANGENT' attribute must be of type TINYGLTF_COMPONENT_TYPE_FLOAT & TINYGLTF_TYPE_VEC4.");
						const DirectX::XMFLOAT4* buffer = reinterpret_cast<const DirectX::XMFLOAT4*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
						for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex)
						{
							DirectX::XMFLOAT4 tangent = buffer[accessorIndex];
							float sigma = tangent.w;
							tangent.w = 0;
							XMStoreFloat4(&tangent, XMVector4Transform(XMLoadFloat4(&tangent), transform));
							tangent.w = sigma;
							combinedBuffer.vertices.tangents.emplace_back(tangent);
						}
					}
					else if (gltfAttribute.first == "TEXCOORD_0")
					{
						_ASSERT_EXPR(gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && gltfAccessor.type == TINYGLTF_TYPE_VEC2, L"'TEXCOORD_0' attribute must be of type TINYGLTF_COMPONENT_TYPE_FLOAT & TINYGLTF_TYPE_VEC2.");
						const DirectX::XMFLOAT2* buffer = reinterpret_cast<const DirectX::XMFLOAT2*>(gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset);
						for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count; ++accessorIndex)
						{
							combinedBuffer.vertices.texcoords.emplace_back(buffer[accessorIndex]);
						}
					}
				}
			}
		}
	}

	//	Create GPU buffers
	for (decltype(combinedBuffers)::const_reference combinedBuffer : combinedBuffers)
	{
#if 1
		if (combinedBuffer.second.vertices.positions.size() == 0)
		{
			continue;
		}
#endif
		Primitive& primitive = primitives_.emplace_back();
		primitive.material_ = combinedBuffer.first;

		D3D11_BUFFER_DESC bufferDesc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		if (combinedBuffer.second.indices.size() > 0)
		{
			primitive.indexBufferView_.format_ = DXGI_FORMAT_R32_UINT;
			primitive.indexBufferView_.strideInBytes_ = sizeof(UINT);
			primitive.indexBufferView_.sizeInBytes_ = combinedBuffer.second.indices.size() * primitive.indexBufferView_.strideInBytes_;

			bufferDesc.ByteWidth = static_cast<UINT>(primitive.indexBufferView_.sizeInBytes_);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;
			subresourceData.pSysMem = combinedBuffer.second.indices.data();
			subresourceData.SysMemPitch = 0;
			subresourceData.SysMemSlicePitch = 0;
			
			primitive.indexBufferView_.verticesBinary_.resize(bufferDesc.ByteWidth);
			memcpy(primitive.indexBufferView_.verticesBinary_.data(), subresourceData.pSysMem, bufferDesc.ByteWidth);
			//primitive.indexBufferView_.bufferData_ = subresourceData.pSysMem;
			
			hr = device->CreateBuffer(&bufferDesc, &subresourceData, primitive.indexBufferView_.buffer_.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		BufferView vertexBufferView;
		if (combinedBuffer.second.vertices.positions.size() > 0)
		{
			vertexBufferView.format_ = DXGI_FORMAT_R32G32B32_FLOAT;
			vertexBufferView.strideInBytes_ = sizeof(FLOAT) * 3;
			vertexBufferView.sizeInBytes_ = combinedBuffer.second.vertices.positions.size() * vertexBufferView.strideInBytes_;

			bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.sizeInBytes_);
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			subresourceData.pSysMem = combinedBuffer.second.vertices.positions.data();
			
			vertexBufferView.verticesBinary_.resize(bufferDesc.ByteWidth);
			memcpy(vertexBufferView.verticesBinary_.data(), subresourceData.pSysMem, bufferDesc.ByteWidth);
			
			hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBufferView.buffer_.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
			primitive.vertexBufferViews_.emplace(std::make_pair("POSITION", vertexBufferView));
		}
		if (combinedBuffer.second.vertices.normals.size() > 0)
		{
			vertexBufferView.format_ = DXGI_FORMAT_R32G32B32_FLOAT;
			vertexBufferView.strideInBytes_ = sizeof(FLOAT) * 3;
			vertexBufferView.sizeInBytes_ = combinedBuffer.second.vertices.normals.size() * vertexBufferView.strideInBytes_;

			bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.sizeInBytes_);
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			subresourceData.pSysMem = combinedBuffer.second.vertices.normals.data();

			vertexBufferView.verticesBinary_.resize(bufferDesc.ByteWidth);
			memcpy(vertexBufferView.verticesBinary_.data(), subresourceData.pSysMem, bufferDesc.ByteWidth);
			
			hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBufferView.buffer_.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
			primitive.vertexBufferViews_.emplace(std::make_pair("NORMAL", vertexBufferView));
		}
		if (combinedBuffer.second.vertices.tangents.size() > 0)
		{
			vertexBufferView.format_ = DXGI_FORMAT_R32G32B32A32_FLOAT;
			vertexBufferView.strideInBytes_ = sizeof(FLOAT) * 4;
			vertexBufferView.sizeInBytes_ = combinedBuffer.second.vertices.tangents.size() * vertexBufferView.strideInBytes_;

			bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.sizeInBytes_);
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			subresourceData.pSysMem = combinedBuffer.second.vertices.tangents.data();

			vertexBufferView.verticesBinary_.resize(bufferDesc.ByteWidth);
			memcpy(vertexBufferView.verticesBinary_.data(), subresourceData.pSysMem, bufferDesc.ByteWidth);
			
			hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBufferView.buffer_.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
			primitive.vertexBufferViews_.emplace(std::make_pair("TANGENT", vertexBufferView));
		}
		if (combinedBuffer.second.vertices.texcoords.size() > 0)
		{
			vertexBufferView.format_ = DXGI_FORMAT_R32G32_FLOAT;
			vertexBufferView.strideInBytes_ = sizeof(FLOAT) * 2;
			vertexBufferView.sizeInBytes_ = combinedBuffer.second.vertices.texcoords.size() * vertexBufferView.strideInBytes_;

			bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.sizeInBytes_);
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			subresourceData.pSysMem = combinedBuffer.second.vertices.texcoords.data();

			vertexBufferView.verticesBinary_.resize(bufferDesc.ByteWidth);
			memcpy(vertexBufferView.verticesBinary_.data(), subresourceData.pSysMem, bufferDesc.ByteWidth);

			hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBufferView.buffer_.ReleaseAndGetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
			primitive.vertexBufferViews_.emplace(std::make_pair("TEXCOORD_0", vertexBufferView));
		}

		// Add dummy attributes if any are missing.
		const std::unordered_map<std::string, BufferView> attributes =
		{
			{ "POSITION", { DXGI_FORMAT_R32G32B32_FLOAT } },
			{ "NORMAL", { DXGI_FORMAT_R32G32B32_FLOAT } },
			{ "TANGENT", { DXGI_FORMAT_R32G32B32A32_FLOAT } },
			{ "TEXCOORD_0", { DXGI_FORMAT_R32G32_FLOAT } },
		};
		for (std::unordered_map<std::string, BufferView>::const_reference attribute : attributes)
		{
			if (primitive.vertexBufferViews_.find(attribute.first) == primitive.vertexBufferViews_.end())
			{
				primitive.vertexBufferViews_.insert(std::make_pair(attribute.first, attribute.second));
			}
		}
	}
}

void GltfModelStaticBatching::FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel, const bool setColor, const DirectX::XMFLOAT4 color)
{
	for (std::vector<tinygltf::Material>::const_reference gltfMaterial : gltfModel.materials)
	{
		std::vector<Material>::reference material = materials_.emplace_back();

		material.name_ = gltfMaterial.name;

		material.data_.emissiveFactor_[0] = static_cast<float>(gltfMaterial.emissiveFactor.at(0));
		material.data_.emissiveFactor_[1] = static_cast<float>(gltfMaterial.emissiveFactor.at(1));
		material.data_.emissiveFactor_[2] = static_cast<float>(gltfMaterial.emissiveFactor.at(2));

		material.data_.alphaMode_ = gltfMaterial.alphaMode == "OPAQUE" ? 0 : gltfMaterial.alphaMode == "MASK" ? 1 : gltfMaterial.alphaMode == "BLEND" ? 2 : 0;
		material.data_.alphaCutoff_ = static_cast<float>(gltfMaterial.alphaCutoff);
		material.data_.doubleSided_ = gltfMaterial.doubleSided ? 1 : 0;

		if (setColor)
		{
			material.data_.pbrMetallicRoughness_.baseColorFactor_[0] = color.x;
			material.data_.pbrMetallicRoughness_.baseColorFactor_[1] = color.y;
			material.data_.pbrMetallicRoughness_.baseColorFactor_[2] = color.z;
			material.data_.pbrMetallicRoughness_.baseColorFactor_[3] = color.w;
		}
		else
		{
			material.data_.pbrMetallicRoughness_.baseColorFactor_[0] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(0));
			material.data_.pbrMetallicRoughness_.baseColorFactor_[1] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(1));
			material.data_.pbrMetallicRoughness_.baseColorFactor_[2] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(2));
			material.data_.pbrMetallicRoughness_.baseColorFactor_[3] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(3));
		}
		material.data_.pbrMetallicRoughness_.basecolorTexture_.index_ = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
		material.data_.pbrMetallicRoughness_.basecolorTexture_.texcoord_ = gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord;
		material.data_.pbrMetallicRoughness_.metallicFactor_ = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
		material.data_.pbrMetallicRoughness_.roughnessFactor_ = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
		material.data_.pbrMetallicRoughness_.metallicRoughnessTexture_.index_ = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
		material.data_.pbrMetallicRoughness_.metallicRoughnessTexture_.texcoord_ = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

		material.data_.normalTexture_.index_ = gltfMaterial.normalTexture.index;
		material.data_.normalTexture_.texcoord_ = gltfMaterial.normalTexture.texCoord;
		material.data_.normalTexture_.scale_ = static_cast<float>(gltfMaterial.normalTexture.scale);

		material.data_.occlusionTexture_.index_ = gltfMaterial.occlusionTexture.index;
		material.data_.occlusionTexture_.texcoord_ = gltfMaterial.occlusionTexture.texCoord;
		material.data_.occlusionTexture_.strength_ = static_cast<float>(gltfMaterial.occlusionTexture.strength);

		material.data_.emissiveTexture_.index_ = gltfMaterial.emissiveTexture.index;
		material.data_.emissiveTexture_.texcoord_ = gltfMaterial.emissiveTexture.texCoord;
	}

	// Create Material data_ as shader resource view on GPU
	std::vector<Material::Cbuffer> materialData;
	for (std::vector<Material>::const_reference material : materials_)
	{
		materialData.emplace_back(material.data_);
	}

	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> materialbuffer;
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Material::Cbuffer) * materialData.size());
	bufferDesc.StructureByteStride = sizeof(Material::Cbuffer);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = materialData.data();
	hr = device->CreateBuffer(&bufferDesc, &subresourceData, materialbuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(materialData.size());
	hr = device->CreateShaderResourceView(materialbuffer.Get(), &shaderResourceViewDesc, materialResourceView_.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void GltfModelStaticBatching::FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel)
{
	HRESULT hr{ S_OK };
	for (const tinygltf::Texture& gltfTexture : gltfModel.textures)
	{
		Texture& texture{ textures_.emplace_back() };
		texture.name_ = gltfTexture.name;
		texture.source_ = gltfTexture.source;
	}
	for (const tinygltf::Image& gltfImage : gltfModel.images)
	{
		Image& image{ images_.emplace_back() };
		image.name_ = gltfImage.name;
		image.width_ = gltfImage.width;
		image.height_ = gltfImage.height;
		image.component_ = gltfImage.component;
		image.bits_ = gltfImage.bits;
		image.pixelType_ = gltfImage.pixel_type;
		image.bufferView_ = gltfImage.bufferView;
		image.mimeType_ = gltfImage.mimeType;
		image.uri_ = gltfImage.uri;
		image.asIs_ = gltfImage.as_is;

		if (gltfImage.bufferView > -1)
		{
			const tinygltf::BufferView& bufferView{ gltfModel.bufferViews.at(gltfImage.bufferView) };
			const tinygltf::Buffer& buffer{ gltfModel.buffers.at(bufferView.buffer) };
			const byte* data = buffer.data.data() + bufferView.byteOffset;

			ID3D11ShaderResourceView* textureResourceView{};
			hr = LoadTextureFromMemory(device, data, bufferView.byteLength, &textureResourceView);
			if (hr == S_OK)
			{
				textureResourceViews_.emplace_back().Attach(textureResourceView);
			}
		}
		else
		{
			const std::filesystem::path path(filename_);
			ID3D11ShaderResourceView* shaderResourceView{};
			D3D11_TEXTURE2D_DESC texture2d_desc;
			std::wstring fileName
			{ 
				path.parent_path().concat(L"/").wstring() + std::wstring(gltfImage.uri.begin(), gltfImage.uri.end()) 
			};
			image.filename_ = fileName;
			hr = LoadTextureFromFile(device, fileName.c_str(), &shaderResourceView, &texture2d_desc);
			if (hr == S_OK)
			{
				textureResourceViews_.emplace_back().Attach(shaderResourceView);
			}
		}
	}
}

void GltfModelStaticBatching::SetPixelShaderFromName(const char* csoName)
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	Graphics::Instance().GetShader()->CreatePsFromCso(device, csoName, pixelShader_.ReleaseAndGetAddressOf());
	SetPixelShader(pixelShader_.Get());
}

void GltfModelStaticBatching::Render()
{
	ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
	deviceContext->PSSetShaderResources(0, 1, materialResourceView_.GetAddressOf());

	deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
	deviceContext->PSSetShader(pixelShader_.Get(), nullptr, 0);
	deviceContext->IASetInputLayout(inputLayout_.Get());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (decltype(primitives_)::const_reference primitive : primitives_)
	{
		ID3D11Buffer* vertexBuffers[] = {
			primitive.vertexBufferViews_.at("POSITION").buffer_.Get(),
			primitive.vertexBufferViews_.at("NORMAL").buffer_.Get(),
			primitive.vertexBufferViews_.at("TANGENT").buffer_.Get(),
			primitive.vertexBufferViews_.at("TEXCOORD_0").buffer_.Get(),
		};
		UINT strides[] = {
			static_cast<UINT>(primitive.vertexBufferViews_.at("POSITION").strideInBytes_),
			static_cast<UINT>(primitive.vertexBufferViews_.at("NORMAL").strideInBytes_),
			static_cast<UINT>(primitive.vertexBufferViews_.at("TANGENT").strideInBytes_),
			static_cast<UINT>(primitive.vertexBufferViews_.at("TEXCOORD_0").strideInBytes_),
		};
		UINT offsets[_countof(vertexBuffers)] = {};
		deviceContext->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, strides, offsets);
		deviceContext->IASetIndexBuffer(primitive.indexBufferView_.buffer_.Get(), primitive.indexBufferView_.format_, 0);

		PrimitiveConstants primitiveData = {};
		primitiveData.material_ = primitive.material_;
		primitiveData.hasTangent_ = primitive.vertexBufferViews_.at("TANGENT").buffer_ != NULL;
		XMStoreFloat4x4(&primitiveData.world_, GetTransform()->CalcWorld());
		deviceContext->UpdateSubresource(primitiveCbuffer_.Get(), 0, 0, &primitiveData, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, primitiveCbuffer_.GetAddressOf());
		deviceContext->PSSetConstantBuffers(0, 1, primitiveCbuffer_.GetAddressOf());

		const Material& material = materials_.at(primitive.material_);
		const int textureIndices[] =
		{
			material.data_.pbrMetallicRoughness_.basecolorTexture_.index_,
			material.data_.pbrMetallicRoughness_.metallicRoughnessTexture_.index_,
			material.data_.normalTexture_.index_,
			material.data_.emissiveTexture_.index_,
			material.data_.occlusionTexture_.index_,
		};
		ID3D11ShaderResourceView* nullShaderResourceView = {};
		std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
		for (int textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex)
		{
			shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ? textureResourceViews_.at(textures_.at(textureIndices[textureIndex]).source_).Get() : nullShaderResourceView;
		}
		deviceContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());

		deviceContext->DrawIndexed(static_cast<UINT>(primitive.indexBufferView_.count()), 0, 0);

	}
}

//  デバッグ描画
void GltfModelStaticBatching::DrawDebug()
{
	if (ImGui::TreeNode(u8"GltfStaticModel"))
	{
		ImGui::TreePop();
	}
}