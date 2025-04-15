#include "GltfModel.h"

#include <stack>
#include <filesystem>
#include <functional>
#include <iostream>
#include <fstream>

#include "../Others/Misc.h"
#include "../Graphics/Graphics.h"
#include "../Resources/Texture.h"
#include "../Others/MathHelper.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_EXTERANL_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT

#define USE_SERIALIZE 1

GltfModel::GltfModel(const std::string& filename, const std::string& rootNodeName) : filename_(filename)
{
    ID3D11Device* device = Graphics::Instance().GetDevice();

#if USE_SERIALIZE
    std::filesystem::path cerealFilename(filename);
    cerealFilename.replace_extension("cereal");
    //  シリアライズされたファイルがあれば読み込む
    if (std::filesystem::exists(cerealFilename.c_str()))
    {
        std::ifstream ifs(cerealFilename.c_str(), std::ios::binary);
        cereal::BinaryInputArchive deserialization(ifs);
        deserialization(scenes_, nodes_, meshes_, materials_, textures_, images_, skins_, animations_);

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

        for (int meshIndex = 0; meshIndex < meshes_.size(); ++meshIndex)
        {
            for (int primitiveIndex = 0; primitiveIndex < meshes_.at(meshIndex).primitives_.size(); ++primitiveIndex)
            {
                const BufferView& indexBufferView = meshes_.at(meshIndex).primitives_.at(primitiveIndex).indexBufferView_;
                D3D11_BUFFER_DESC bufferDesc = {};
                bufferDesc.ByteWidth = static_cast<UINT>(indexBufferView.sizeInBytes_);
                bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                D3D11_SUBRESOURCE_DATA subresourceData = {};
                subresourceData.pSysMem = indexBufferView.verticesBinary_.data();
                hr = device->CreateBuffer(&bufferDesc, &subresourceData,
                    meshes_.at(meshIndex).primitives_.at(primitiveIndex).indexBufferView_.buffer_.ReleaseAndGetAddressOf());
                _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

                for (auto& vertexBufferView : meshes_.at(meshIndex).primitives_.at(primitiveIndex).vertexBufferViews_)
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
            scene.name_ = gltfScene.name;
            scene.nodes_ = gltfScene.nodes;
        }

        FetchNodes(gltfModel, rootNodeName);
        FetchMeshes(device, gltfModel);
        FetchMaterials(device, gltfModel);
        FetchTextures(device, gltfModel);
        FetchAnimations(gltfModel);

#if USE_SERIALIZE
        std::ofstream ofs(cerealFilename.c_str(), std::ios::binary);
        cereal::BinaryOutputArchive serialization(ofs);
        serialization(scenes_, nodes_, meshes_, materials_, textures_, images_, skins_, animations_);

    }
#endif

    const std::map<std::string, BufferView>& vertexBufferViews{ meshes_.at(0).primitives_.at(0).vertexBufferViews_ };
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
    {
        {"POSITION",0,vertexBufferViews.at("POSITION").format_,     0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",  0,vertexBufferViews.at("NORMAL").format_,       1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0,vertexBufferViews.at("TANGENT").format_,      2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",0,vertexBufferViews.at("TEXCOORD_0").format_,   3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"JOINTS",  0,vertexBufferViews.at("JOINTS_0").format_,     4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"WEIGHTS", 0,vertexBufferViews.at("WEIGHTS_0").format_,    5, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "JOINTS", 1,vertexBufferViews.at("JOINTS_1").format_,     6, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "WEIGHTS",1,vertexBufferViews.at("WEIGHTS_1").format_,    7, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },

    };
    Graphics::Instance().GetShader()->CreateVsFromCso(device, "./Resources/Shader/GltfModelVS.cso", vertexShader_.ReleaseAndGetAddressOf(),
        inputLayout_.ReleaseAndGetAddressOf(), inputElementDesc, _countof(inputElementDesc));
    Graphics::Instance().GetShader()->CreatePsFromCso(device, "./Resources/Shader/GltfModelPS.cso", pixelShader_.ReleaseAndGetAddressOf());

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(PrimitiveConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    HRESULT hr;
    hr = device->CreateBuffer(&bufferDesc, nullptr, primitiveCbuffer_.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //  ボーン行列の定数バッファ生成
    bufferDesc.ByteWidth = sizeof(PrimitiveJointConstants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&bufferDesc, NULL, primitiveJointCbuffer_.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
    
    animatedNodes_[0] = nodes_;
    animatedNodes_[1] = nodes_;
    blendedAnimatedNodes_ = nodes_;

    initAnimatedNode_ = nodes_;
}

void GltfModel::FetchNodes(const tinygltf::Model& gltfModel, const std::string& rootNodeName)
{
    for (std::vector<tinygltf::Node>::const_reference gltfNode : gltfModel.nodes)
    {
        Node& node{ nodes_.emplace_back() };
        node.name_ = gltfNode.name;
        node.skin_ = gltfNode.skin;
        node.mesh_ = gltfNode.mesh;
        node.children_ = gltfNode.children;
        
        //  rootを見つけたらフラグを立てる
        node.isRoot_ = false;
        if (gltfNode.name == "root")
        {
            node.isRoot_ = true;
        }

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

void GltfModel::FetchMeshes(ID3D11Device* device, const tinygltf::Model& gltfModel)
{
    HRESULT hr;
    for (std::vector<tinygltf::Mesh>::const_reference gltfMesh : gltfModel.meshes)
    {
        Mesh& mesh{ meshes_.emplace_back() };
        mesh.name_ = gltfMesh.name;
        for (std::vector<tinygltf::Primitive>::const_reference gltfPrimitive : gltfMesh.primitives)
        {
            Mesh::Primitive& primitive{mesh.primitives_.emplace_back()};
            primitive.material_ = gltfPrimitive.material;

            //Create Index Buffer
            const tinygltf::Accessor& gltfAccessor{gltfModel.accessors.at(gltfPrimitive.indices)};
            const tinygltf::BufferView& gltfBufferView{gltfModel.bufferViews.at(gltfAccessor.bufferView)};

            primitive.indexBufferView_ = MakeBufferView(gltfAccessor);

            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = static_cast<UINT>(primitive.indexBufferView_.sizeInBytes_);
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA subresourceData{};
            subresourceData.pSysMem = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
            
            primitive.indexBufferView_.verticesBinary_.resize(bufferDesc.ByteWidth);
            memcpy(primitive.indexBufferView_.verticesBinary_.data(), subresourceData.pSysMem, bufferDesc.ByteWidth);

            hr = device->CreateBuffer(&bufferDesc, &subresourceData, primitive.indexBufferView_.buffer_.ReleaseAndGetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

            for (std::map<std::string, int>::const_reference gltfAttribute : gltfPrimitive.attributes)
            {
                //  Create vertex buffers
                tinygltf::Accessor gltfAccessor{gltfModel.accessors.at(gltfAttribute.second)};
                const tinygltf::BufferView& gltfBufferView{gltfModel.bufferViews.at(gltfAccessor.bufferView)};

                const void* buffer = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                std::vector<USHORT> joints0;
                std::vector<FLOAT>  weights0;
                if (gltfAttribute.first == "JOINTS_0")
                {
                    if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        const BYTE* data = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count * 4; ++accessorIndex)
                        {
                            joints0.emplace_back(static_cast<USHORT>(data[accessorIndex]));
                        }
                        buffer = joints0.data();
                        gltfAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
                    }
                }
                else if (gltfAttribute.first == "JOINTS_1")
                {
                    continue;
                }
                else if (gltfAttribute.first == "WEIGHTS_0")
                {
                    if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        const BYTE* data = gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                        for (size_t accessorIndex = 0; accessorIndex < gltfAccessor.count * 4; ++accessorIndex)
                        {
                            weights0.emplace_back(static_cast<FLOAT>(data[accessorIndex]) / 0xFF);
                        }
                        buffer = weights0.data();
                        gltfAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                    }
                }
                else if (gltfAttribute.first == "WEIGHTS_1")
                {
                    continue;
                }

                BufferView vertexBufferView{ MakeBufferView(gltfAccessor) };

                D3D11_BUFFER_DESC bufferDesc{};
                bufferDesc.ByteWidth = static_cast<UINT>(vertexBufferView.sizeInBytes_);
                bufferDesc.Usage = D3D11_USAGE_DEFAULT;
                bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                D3D11_SUBRESOURCE_DATA subresourceData{};
                subresourceData.pSysMem = buffer;
                
                vertexBufferView.verticesBinary_.resize(bufferDesc.ByteWidth);
                memcpy(vertexBufferView.verticesBinary_.data(), subresourceData.pSysMem, bufferDesc.ByteWidth);

                hr = device->CreateBuffer(&bufferDesc, &subresourceData, vertexBufferView.buffer_.ReleaseAndGetAddressOf());
                _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

                primitive.vertexBufferViews_.emplace(std::make_pair(gltfAttribute.first, vertexBufferView));

            }
            //  Add dummy attributes if any missing.
            const std::unordered_map<std::string, BufferView> attributes
            {
                { "TANGENT",    { DXGI_FORMAT_R32G32B32A32_FLOAT } },
                { "TEXCOORD_0", { DXGI_FORMAT_R32G32_FLOAT } },
                { "JOINTS_0",   { DXGI_FORMAT_R16G16B16A16_UINT } },
                { "WEIGHTS_0",  { DXGI_FORMAT_R32G32B32A32_FLOAT } },
                { "JOINTS_1",   { DXGI_FORMAT_R16G16B16A16_UINT } },
                { "WEIGHTS_1",  { DXGI_FORMAT_R32G32B32A32_FLOAT } }, 
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
}

void GltfModel::FetchMaterials(ID3D11Device* device, const tinygltf::Model& gltfModel, const bool setColor, const DirectX::XMFLOAT4 color)
{
    for (std::vector<tinygltf::Material>::const_reference gltfMaterial : gltfModel.materials)
    {
        std::vector<Material>::reference material = materials_.emplace_back();

        material.name_ = gltfMaterial.name;

        material.data_.emissiveFactor_[0] = static_cast<float>(gltfMaterial.emissiveFactor.at(0));
        material.data_.emissiveFactor_[1] = static_cast<float>(gltfMaterial.emissiveFactor.at(1));
        material.data_.emissiveFactor_[2] = static_cast<float>(gltfMaterial.emissiveFactor.at(2));

        material.data_.alphaMode_ = gltfMaterial.alphaMode == "OPAQUE" ? 0 : gltfMaterial.alphaMode == "MASK" ? 1 : gltfMaterial.alphaMode == "BLEND" ? 2 : 0;
        material.data_.alphaCutOff_ = static_cast<float>(gltfMaterial.alphaCutoff);
        material.data_.doubleSided_ = gltfMaterial.doubleSided ? 1 : 0;

        material.data_.pbrMetallicRoughness_.baseColorFactor_[0] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(0));
        material.data_.pbrMetallicRoughness_.baseColorFactor_[1] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(1));
        material.data_.pbrMetallicRoughness_.baseColorFactor_[2] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(2));
        material.data_.pbrMetallicRoughness_.baseColorFactor_[3] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(3));
        material.data_.pbrMetallicRoughness_.baseColorTexture_.index_ = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        material.data_.pbrMetallicRoughness_.baseColorTexture_.texcoord_ = gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord;
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

    //  Create material data as shader resource view on GPU
    std::vector<Material::Cbuffer> materialData;
    for (std::vector<Material>::const_reference material : materials_)
    {
        materialData.emplace_back(material.data_);
    }

    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer;
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Material::Cbuffer) * materialData.size());
    bufferDesc.StructureByteStride = sizeof(Material::Cbuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    D3D11_SUBRESOURCE_DATA subresourceData{};
    subresourceData.pSysMem = materialData.data();
    hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, materialBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
    shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(materialData.size());
    hr = Graphics::Instance().GetDevice()->CreateShaderResourceView(materialBuffer.Get(), &shaderResourceViewDesc, materialResourceView_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

}

void GltfModel::FetchTextures(ID3D11Device* device, const tinygltf::Model& gltfModel)
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
        image.name_         = gltfImage.name;
        image.width_        = gltfImage.width;
        image.height_       = gltfImage.height;
        image.component_    = gltfImage.component;
        image.bits_         = gltfImage.bits;
        image.pixelType_    = gltfImage.pixel_type;
        image.bufferView_   = gltfImage.bufferView;
        image.mimeType_     = gltfImage.mimeType;
        image.uri_          = gltfImage.uri;
        image.asIs_         = gltfImage.as_is;

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
            D3D11_TEXTURE2D_DESC texture2dDesc;
            std::wstring filename
            { 
                path.parent_path().concat(L"/").wstring() + std::wstring(gltfImage.uri.begin(), gltfImage.uri.end())
            };
            image.filename_ = filename;
            hr = LoadTextureFromFile(device, filename.c_str(), &shaderResourceView, &texture2dDesc);
            if (hr == S_OK)
            {
                textureResourceViews_.emplace_back().Attach(shaderResourceView);
            }
        }
    }
}

void GltfModel::FetchAnimations(const tinygltf::Model& gltfModel)
{
    for (std::vector<tinygltf::Skin>::const_reference transmissionSkin : gltfModel.skins)
    {
        Skin& skin{ skins_.emplace_back() };
        const tinygltf::Accessor& gltfAccessor{gltfModel.accessors.at(transmissionSkin.inverseBindMatrices)};
        const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };
        skin.inverseBindMatrices_.resize(gltfAccessor.count);
        memcpy(skin.inverseBindMatrices_.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
            gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT4X4));
        skin.joints_ = transmissionSkin.joints;
    }

    for (std::vector<tinygltf::Animation>::const_reference gltfAnimation : gltfModel.animations)
    {
        Animation& animation{ animations_.emplace_back() };
        animation.name_ = gltfAnimation.name;
        for (std::vector<tinygltf::AnimationSampler>::const_reference gltfSampler : gltfAnimation.samplers)
        {
            Animation::Sampler& sampler{animation.samplers_.emplace_back()};
            sampler.input_ = gltfSampler.input;
            sampler.output_ = gltfSampler.output;
            sampler.interpolation_ = gltfSampler.interpolation;

            const tinygltf::Accessor& gltfAccessor{gltfModel.accessors.at(gltfSampler.input)};
            const tinygltf::BufferView& gltfBufferView{gltfModel.bufferViews.at(gltfAccessor.bufferView)};
            std::pair<std::unordered_map<int, std::vector<float>>::iterator, bool>& timelines{animation.timelines_.emplace(gltfSampler.input, gltfAccessor.count)};
            if (timelines.second)
            {
                memcpy(timelines.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
                    gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(FLOAT));
            }
        }
        for (std::vector<tinygltf::AnimationChannel>::const_reference gltfChannel : gltfAnimation.channels)
        {
            Animation::Channel& channel{animation.channels_.emplace_back()};
            channel.sampler_ = gltfChannel.sampler;
            channel.targetNode_ = gltfChannel.target_node;
            channel.targetPath_ = gltfChannel.target_path;

            const tinygltf::AnimationSampler& gltfSampler{gltfAnimation.samplers.at(gltfChannel.sampler)};
            const tinygltf::Accessor& gltfAccessor{gltfModel.accessors.at(gltfSampler.output)};
            const tinygltf::BufferView& gltfBufferView{gltfModel.bufferViews.at(gltfAccessor.bufferView)};
            if (gltfChannel.target_path == "scale")
            {
                std::pair<std::unordered_map<int, std::vector<DirectX::XMFLOAT3>>::iterator, bool>& scales{animation.scales_.emplace(gltfSampler.output, gltfAccessor.count)};
                if (scales.second)
                {
                    memcpy(scales.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT3));
                }
            }
            else if (gltfChannel.target_path == "rotation")
            {
                std::pair<std::unordered_map<int, std::vector<DirectX::XMFLOAT4>>::iterator, bool>& rotations{animation.rotations_.emplace(gltfSampler.output, gltfAccessor.count)};
                if (rotations.second)
                {
                    memcpy(rotations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT4));
                }
            }
            else if (gltfChannel.target_path == "translation")
            {
                std::pair<std::unordered_map<int, std::vector<DirectX::XMFLOAT3>>::iterator, bool>& translations{animation.translations_.emplace(gltfSampler.output, gltfAccessor.count)};
                if (translations.second)
                {
                    memcpy(translations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(DirectX::XMFLOAT3));
                }
            }
            else if (gltfChannel.target_path == "weights")
            {
                //_ASSERT_EXPR(FALSE, L"");
            }
            else
            {
                _ASSERT_EXPR(FALSE, L"");
            }
        }
    }
    //  各チャンネルのタイムラインで、最も長いアニメーション時間を見つける
    for (decltype(animations_)::reference animation : animations_)
    {
        for (decltype(animation.timelines_)::reference timelines : animation.timelines_)
        {
            animation.duration_ = std::max<float>(animation.duration_, timelines.second.back());
        }
    }
}

void GltfModel::CumulateTransforms(std::vector<Node>& nodes)
{
    std::stack<DirectX::XMFLOAT4X4> parentGlobalTransforms;
    std::function<void(int)> traverse{[&](int nodeIndex)->void
        {
            Node& node{ nodes.at(nodeIndex) };
            DirectX::XMMATRIX S{DirectX::XMMatrixScaling(node.scale_.x, node.scale_.y, node.scale_.z)};
            DirectX::XMMATRIX R{DirectX::XMMatrixRotationQuaternion(
                DirectX::XMVectorSet(node.rotation_.x, node.rotation_.y, node.rotation_.z, node.rotation_.w))};
            DirectX::XMMATRIX T{DirectX::XMMatrixTranslation(node.translation_.x, node.translation_.y, node.translation_.z)};
            DirectX::XMStoreFloat4x4(&node.globalTransform_, S * R * T * DirectX::XMLoadFloat4x4(&parentGlobalTransforms.top()));
            
            //  アニメーションの移動値を反映しない
			if (node.isRoot_ && useRootMotionMovement_ == false)
            {
                node.globalTransform_._41 = 0;
                node.globalTransform_._42 = 0;
                node.globalTransform_._43 = 0;
            }

            for (int childIndex : node.children_)
            {
                parentGlobalTransforms.push(node.globalTransform_);
                traverse(childIndex);
                parentGlobalTransforms.pop();
            }
    }};
    for (std::vector<int>::value_type nodeIndex : scenes_.at(0).nodes_)
    {
        parentGlobalTransforms.push({ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 });
        traverse(nodeIndex);
        parentGlobalTransforms.pop();
    }

}

GltfModel::BufferView GltfModel::MakeBufferView(const tinygltf::Accessor& accessor)
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
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            bufferView.format_ = DXGI_FORMAT_R8G8B8A8_UINT;
            bufferView.strideInBytes_ = sizeof(BYTE) * 4;
            break;
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

void GltfModel::PlayAnimation(const int& index, const bool& loop, const float& blendTime, const float& animSpeed, const float& startFrame, const float& endFrame)
{
    Animate(animationClip_, currentAnimationSeconds_, animatedNodes_[0]);
    animationClip_ = index;
    
    Animate(index, startFrame, animatedNodes_[1]);
    currentAnimationSeconds_ = startFrame;
    blendAnimationSeconds_ = 0.0f;
    factor_ = 0.0f;

    isAnimationLoop_ = loop;
    transitionState_ = 1;
    animationEndFlag_ = false;

    transitionTime_ = blendTime;
    factor_ = 0;

    animationSpeed_ = animSpeed;

	animationDuration_ = endFrame == 0.0f ? 
        animations_.at(animationClip_).duration_ : 
		std::min(animations_.at(animationClip_).duration_, endFrame);

    isBlendAnimation_ = true;
}

void GltfModel::UpdateAnimation(const float& elapsedTime)
{
    //  アニメーションを再生していなければreturn
    if (IsPlayAnimation() == false)return;

    if (transitionState_ > 0 && transitionTime_ > 0.0f)
    {
		factor_ = blendAnimationSeconds_ / transitionTime_;
		BlendAnimations(animatedNodes_[0], animatedNodes_[1], factor_, blendedAnimatedNodes_);
        blendAnimationSeconds_ += elapsedTime;
		//  ブレンドアニメーションが終わったら
        if (factor_ > 1.0f)
		{
			transitionState_ = 0;
            blendAnimationSeconds_ = 0.0f;
            isBlendAnimation_ = false;
		}
        nodes_ = blendedAnimatedNodes_;
    }
    else    //  アニメーションの遷移が終わったら
    {
        isBlendAnimation_ = false;

        currentAnimationSeconds_ += elapsedTime * animationSpeed_;

        //  アニメーションの長さ
		float animationDuration = animationDuration_;
		//float animationDuration = animations_.at(animationClip_).duration_;
        
        if (currentAnimationSeconds_ > animationDuration)
        {
            if (isAnimationLoop_)   //  ループフラグがtrueなら巻き戻す
            {
                currentAnimationSeconds_ = 0.0f;
            }
            else
            {
                animationEndFlag_ = true;
                //currentAnimationSeconds_ = animationDuration;
            }
        }
        Animate(animationClip_, currentAnimationSeconds_, nodes_);
        
    }

    //  ルートモーション
    RootMotion(transform_.GetScaleFactor());

}

void GltfModel::Animate(const size_t& animationIndex, const float& time, std::vector<Node>& animatedNodes)
{
    _ASSERT_EXPR(animations_.size() > animationIndex, L"animationSize <= animationIndex.");
    _ASSERT_EXPR(animatedNodes.size() == nodes_.size(), L"animationNodeSize != nodesSize.");
    std::function<size_t(const std::vector<float>&, float, float&)> indexof
    {
        [](const std::vector<float>& timelines, float time, float& interpolationFactor)->size_t
        {
            const size_t keyframeCount{timelines.size()};

            if (time > timelines.at(keyframeCount - 1))
            {
                interpolationFactor = 1.0f;
                return keyframeCount - 2;
            }
            else if (time < timelines.at(0))
            {
                //interpolationFactor = 0.0f;
                interpolationFactor = timelines.at(0);
                return 0;
            }
            size_t keyframeIndex{ 0 };
            for (size_t timeIndex = 1; timeIndex < keyframeCount; ++timeIndex)
            {
                if (time < timelines.at(timeIndex))
                {
                    keyframeIndex = std::max<size_t>(0LL, timeIndex - 1);
                    break;
                }
            }
            interpolationFactor = (time - timelines.at(keyframeIndex + 0)) /
                (timelines.at(keyframeIndex + 1) - timelines.at(keyframeIndex + 0));
            return keyframeIndex;
        }
    };

    if (animations_.size() > 0)
    {
        const Animation& animation{ animations_.at(animationIndex) };
        for (std::vector<Animation::Channel>::const_reference channel : animation.channels_)
        {
            const Animation::Sampler& sampler{ animation.samplers_.at(channel.sampler_) };
            const std::vector<float>& timeline{ animation.timelines_.at(sampler.input_) };
            if (timeline.size() == 0)
            {
                continue;
            }
            float interpolationFactor{};

            size_t keyframeIndex{ indexof(timeline, time, interpolationFactor) };

            if (channel.targetPath_ == "scale")
            {
                const std::vector<DirectX::XMFLOAT3>& scales{ animation.scales_.at(sampler.output_) };
                DirectX::XMStoreFloat3(&animatedNodes.at(channel.targetNode_).scale_,
                    DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&scales.at(keyframeIndex + 0)),
                        DirectX::XMLoadFloat3(&scales.at(keyframeIndex + 1)), interpolationFactor));
            }
            else if (channel.targetPath_ == "rotation")
            {
                const std::vector<DirectX::XMFLOAT4>& rotations{ animation.rotations_.at(sampler.output_) };
                DirectX::XMStoreFloat4(&animatedNodes.at(channel.targetNode_).rotation_,
                    DirectX::XMQuaternionNormalize(DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&rotations.at(keyframeIndex + 0)),
                        DirectX::XMLoadFloat4(&rotations.at(keyframeIndex + 1)), interpolationFactor)));
            }
            else if (channel.targetPath_ == "translation")
            {
                const std::vector<DirectX::XMFLOAT3>& translations{ animation.translations_.at(sampler.output_) };
                DirectX::XMStoreFloat3(&animatedNodes.at(channel.targetNode_).translation_,
                    DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&translations.at(keyframeIndex + 0)),
                        DirectX::XMLoadFloat3(&translations.at(keyframeIndex + 1)), interpolationFactor));
            }
        }
        CumulateTransforms(animatedNodes);
    }
    else
    {
        animatedNodes = nodes_;
    }
}

void GltfModel::AppendAnimation(const std::string& filename)
{
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

    for (std::vector<tinygltf::Scene>::const_reference gltf_scene : gltfModel.scenes)
    {
        Scene& scene{ scenes_.emplace_back() };
        scene.name_ = gltf_scene.name;
        scene.nodes_ = gltf_scene.nodes;
    }
    FetchAnimations(gltfModel);
}

void GltfModel::BlendAnimations(const std::vector<Node>& fromNodes, const std::vector<Node>& toNodes, const float& factor, std::vector<Node>& outNodes)
{
    _ASSERT_EXPR(fromNodes.size() == toNodes.size(), L"The size of the two node arrays must be the same.");

    size_t nodeCount{ fromNodes.size() };
    _ASSERT_EXPR(nodeCount == outNodes.size(), L"The size of output nodes must be input nodes.");
    outNodes.resize(nodeCount);
    
    for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
    {
        DirectX::XMVECTOR S[2]{ DirectX::XMLoadFloat3(&fromNodes.at(nodeIndex).scale_), DirectX::XMLoadFloat3(&toNodes.at(nodeIndex).scale_) };
        DirectX::XMStoreFloat3(&outNodes.at(nodeIndex).scale_, DirectX::XMVectorLerp(S[0], S[1], factor));

        DirectX::XMVECTOR R[2]{ DirectX::XMLoadFloat4(&fromNodes.at(nodeIndex).rotation_), DirectX::XMLoadFloat4(&toNodes.at(nodeIndex).rotation_) };
        DirectX::XMStoreFloat4(&outNodes.at(nodeIndex).rotation_, DirectX::XMQuaternionSlerp(R[0], R[1], factor));

        DirectX::XMVECTOR T[2]{ DirectX::XMLoadFloat3(&fromNodes.at(nodeIndex).translation_), DirectX::XMLoadFloat3(&toNodes.at(nodeIndex).translation_) };
        DirectX::XMStoreFloat3(&outNodes.at(nodeIndex).translation_, DirectX::XMVectorLerp(T[0], T[1], factor));
    }
    CumulateTransforms(outNodes);
}

//  アニメーション再生中かどうか
bool GltfModel::IsPlayAnimation()const
{
    if (animationEndFlag_)return false;

    return true;
}

//  現在再生中のアニメーション番号取得
int GltfModel::GetCurrentAnimNum()
{
    return animationClip_;
}

//  ジョイントポジション取得
DirectX::XMFLOAT3 GltfModel::GetJointPosition(const std::string& nodeName, const DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT3& offsetPos)
{
    DirectX::XMFLOAT3 position = offsetPos;/*world space*/
    DirectX::XMFLOAT3 origin = {};

    for (int index = 0; index < nodes_.size(); index++)
    {
        if (nodes_.at(index).name_ == nodeName)
        {
            const Node& node = nodes_.at(index);
            DirectX::XMFLOAT4X4 globalTransform = node.globalTransform_;
            DirectX::XMMATRIX M = DirectX::XMLoadFloat4x4(&globalTransform) * DirectX::XMLoadFloat4x4(&transform);
            DirectX::XMStoreFloat3(&position, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&position), M));
            DirectX::XMStoreFloat3(&origin, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&origin), M));
            return position;
        }
    }
    _ASSERT_EXPR(false, L"Joint is not found.");
    return {};

}

//  ジョイントポジション取得
DirectX::XMFLOAT3 GltfModel::GetJointPosition(const size_t& nodeIndex, const DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT3& offsetPos)
{
    DirectX::XMFLOAT3 position = offsetPos;

    const Node& node = nodes_.at(nodeIndex);
    DirectX::XMMATRIX M = XMLoadFloat4x4(&node.globalTransform_) * DirectX::XMLoadFloat4x4(&transform);
    DirectX::XMStoreFloat3(&position, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&position), M));
    return position;
}

//  ノードインデックス取得
const int GltfModel::GetNodeIndex(const std::string& nodeName)
{
    for (int nodeIndex = 0; nodeIndex < nodes_.size(); ++nodeIndex)
    {
        if (nodes_.at(nodeIndex).name_ == nodeName)
        {
            return nodeIndex;
        }
    }

    return -1;
}

//  ルートモーション
void GltfModel::RootMotion(const float& scaleFactor)
{
    //  アニメーションが再生されていないためreturn
    if (animationEndFlag_)return;                   

    //  ルートモーションを使用しないためreturn
    if (useRootMotionMovement_ == false)return;

    //  ルートモーション初回なら計算しなおす
    if (isFirstTimeRootMotion_)
    {
        Animate(animationClip_, currentAnimationSeconds_, nodes_);
    }

    Node& node = nodes_.at(rootJointIndex_);

    if (isFirstTimeRootMotion_)
    {
        lastPosition_ = { node.globalTransform_._41,node.globalTransform_._42,node.globalTransform_._43 };
        isFirstTimeRootMotion_ = false;
    }

    DirectX::XMFLOAT3 position  = { node.globalTransform_._41,node.globalTransform_._42,node.globalTransform_._43 };
    DirectX::XMFLOAT3 displacement = { position.x - lastPosition_.x,position.y - lastPosition_.y,position.z - lastPosition_.z };

    DirectX::XMMATRIX C = DirectX::XMLoadFloat4x4(&GetTransform()->GetCoordinateSystemTransform(Transform::CoordinateSystem::cRightYup)) * DirectX::XMMatrixScaling(scaleFactor, scaleFactor, scaleFactor);
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(GetTransform()->GetScale().x, GetTransform()->GetScale().y, GetTransform()->GetScale().z);
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(GetTransform()->GetRotationX(), GetTransform()->GetRotationY(), GetTransform()->GetRotationZ());
    DirectX::XMStoreFloat3(&displacement, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&displacement), C * S * R));

    DirectX::XMFLOAT3 translation = GetTransform()->GetPosition();
    translation = translation + displacement * rootMotionSpeed_;
    GetTransform()->SetPosition(translation);

    node.globalTransform_._41 = initAnimatedNode_.at(rootJointIndex_).globalTransform_._41;
    node.globalTransform_._42 = initAnimatedNode_.at(rootJointIndex_).globalTransform_._42;
    node.globalTransform_._43 = initAnimatedNode_.at(rootJointIndex_).globalTransform_._43;

    std::function<void(int, int)>traverse = [&](int parentIndex, int nodeIndex)
    {
        GltfModel::Node& node = GetNodes()->at(nodeIndex);
        if (parentIndex > -1)
        {
            DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale_.x, node.scale_.y, node.scale_.z);
            DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(node.rotation_.x, node.rotation_.y, node.rotation_.z, node.rotation_.w));
            DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.translation_.x, node.translation_.y, node.translation_.z);
            DirectX::XMStoreFloat4x4(&node.globalTransform_, S * R * T * DirectX::XMLoadFloat4x4(&GetNodes()->at(parentIndex).globalTransform_));
        }
        for (int childIndex : node.children_)
        {
            traverse(nodeIndex, childIndex);
        }
    };
    traverse(-1, rootJointIndex_);

    lastPosition_ = position;
}

void GltfModel::SetUseRootMotion(const bool& useRootMotion)
{
    useRootMotionMovement_ = useRootMotion;
    isFirstTimeRootMotion_ = true;
}

void GltfModel::Render(const DirectX::XMMATRIX& world)
{
    ID3D11DeviceContext* deviceContext = Graphics::Instance().GetDeviceContext();
    deviceContext->PSSetShaderResources(0, 1, materialResourceView_.GetAddressOf());
    deviceContext->VSSetShader(vertexShader_.Get(), nullptr, 0);
    deviceContext->PSSetShader(pixelShader_.Get(), nullptr, 0);
    deviceContext->IASetInputLayout(inputLayout_.Get());
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#if 1
    std::function<void(int)>traverse{[&](int nodeIndex)->void 
        {
        const Node& node{ nodes_.at(nodeIndex) };

        if (node.skin_ > -1)
        {
            const Skin& skin{ skins_.at(node.skin_) };
            PrimitiveJointConstants primitiveJointData{};
            for (size_t jointIndex = 0; jointIndex < skin.joints_.size(); ++jointIndex)
            {
                DirectX::XMStoreFloat4x4(&primitiveJointData.matrices_[jointIndex],
                    DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices_.at(jointIndex)) *
                    // --- GLTF_ANIMATION ---
                    //DirectX::XMLoadFloat4x4(&animatedNodes.at(skin.joints_.at(jointIndex)).globalTransform_) *
                    DirectX::XMLoadFloat4x4(&nodes_.at(skin.joints_.at(jointIndex)).globalTransform_) *
                    DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&node.globalTransform_))
                );
            }
            deviceContext->UpdateSubresource(primitiveJointCbuffer_.Get(), 0, 0, &primitiveJointData, 0, 0);
            deviceContext->VSSetConstantBuffers(2, 1, primitiveJointCbuffer_.GetAddressOf());
        }
        if (node.mesh_ > -1)
        {
            const Mesh& mesh{ meshes_.at(node.mesh_) };
            for (std::vector<Mesh::Primitive>::const_reference primitive : mesh.primitives_)
            {
                ID3D11Buffer* vertexBuffers[]{
                    primitive.vertexBufferViews_.at("POSITION").buffer_.Get(),
                    primitive.vertexBufferViews_.at("NORMAL").buffer_.Get(),
                    primitive.vertexBufferViews_.at("TANGENT").buffer_.Get(),
                    primitive.vertexBufferViews_.at("TEXCOORD_0").buffer_.Get(),
                    primitive.vertexBufferViews_.at("JOINTS_0").buffer_.Get(),
                    primitive.vertexBufferViews_.at("WEIGHTS_0").buffer_.Get(),
                    primitive.vertexBufferViews_.at("JOINTS_1").buffer_.Get(),
                    primitive.vertexBufferViews_.at("WEIGHTS_1").buffer_.Get(),
                };
                UINT strides[]
                {
                    static_cast<UINT>(primitive.vertexBufferViews_.at("POSITION").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("NORMAL").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("TANGENT").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("TEXCOORD_0").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("JOINTS_0").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("WEIGHTS_0").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("JOINTS_1").strideInBytes_),
                    static_cast<UINT>(primitive.vertexBufferViews_.at("WEIGHTS_1").strideInBytes_),
                };
                UINT offsets[_countof(vertexBuffers)]{ 0 };
                deviceContext->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, strides, offsets);
                deviceContext->IASetIndexBuffer(primitive.indexBufferView_.buffer_.Get(), primitive.indexBufferView_.format_, 0);

                PrimitiveConstants primitiveData{};
                primitiveData.material_ = primitive.material_;
                primitiveData.hasTangent_ = primitive.vertexBufferViews_.at("TANGENT").buffer_ != NULL;
                primitiveData.skin_ = node.skin_;
                DirectX::XMStoreFloat4x4(&primitiveData.world_, DirectX::XMLoadFloat4x4(&node.globalTransform_) * world);

                const Material& material{ materials_.at(primitive.material_) };
                const int textureIndices[]
                {
                    material.data_.pbrMetallicRoughness_.baseColorTexture_.index_,
                    material.data_.pbrMetallicRoughness_.metallicRoughnessTexture_.index_,
                    material.data_.normalTexture_.index_,
                    material.data_.emissiveTexture_.index_,
                    material.data_.occlusionTexture_.index_,
                };
                ID3D11ShaderResourceView* nullShaderResourceView{};
                std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
                for (int textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex)
                {
                    shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ?
                        textureResourceViews_.at(textures_.at(textureIndices[textureIndex]).source_).Get() :
                        nullShaderResourceView;
                }
                deviceContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());

                deviceContext->UpdateSubresource(primitiveCbuffer_.Get(), 0, 0, &primitiveData, 0, 0);
                deviceContext->VSSetConstantBuffers(0, 1, primitiveCbuffer_.GetAddressOf());
                deviceContext->PSSetConstantBuffers(0, 1, primitiveCbuffer_.GetAddressOf());

                deviceContext->DrawIndexed(static_cast<UINT>(primitive.indexBufferView_.count()), 0, 0);

            }
        }
        
        for (std::vector<int>::value_type childIndex : node.children_)
        {
            traverse(childIndex);
        }
    }};
#else
    std::function<void(int)>traverse{ [&](int nodeIndex)->void
    {
    const Node& node{ nodes_.at(nodeIndex) };

    //  Animation
    {
        if (node.skin_ > -1)
        {
            const Skin& skin{ skins_.at(node.skin_) };
            _ASSERT_EXPR(skin.joints_.size() <= PRIMITIVE_MAX_JOINTS, L"The size of the joint array is insufficient, please expand it.");
            PrimitiveJointConstants primitiveJointData{};
            for (size_t jointIndex = 0; jointIndex < skin.joints_.size(); ++jointIndex)
            {
                DirectX::XMStoreFloat4x4(&primitiveJointData.matrices_[jointIndex],
                    DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices_.at(jointIndex)) *
                    // --- GLTF_ANIMATION ---
                    //DirectX::XMLoadFloat4x4(&animatedNodes.at(skin.joints_.at(jointIndex)).globalTransform_) *
                    DirectX::XMLoadFloat4x4(&nodes_.at(skin.joints_.at(jointIndex)).globalTransform_) *
                    DirectX::XMMatrixInverse(NULL, DirectX::XMLoadFloat4x4(&node.globalTransform_))
                );
            }
            deviceContext->UpdateSubresource(primitiveJointCbuffer_.Get(), 0, 0, &primitiveJointData, 0, 0);
            deviceContext->VSSetConstantBuffers(2, 1, primitiveJointCbuffer_.GetAddressOf());
        }
    }
    if (node.mesh_ > -1)
    {
        const Mesh& mesh{ meshes_.at(node.mesh_) };
        for (std::vector<Mesh::Primitive>::const_reference primitive : mesh.primitives_)
        {
            ID3D11Buffer* vertexBuffers[]{
                primitive.vertexBufferViews_.at("POSITION").buffer_.Get(),
                primitive.vertexBufferViews_.at("NORMAL").buffer_.Get(),
                primitive.vertexBufferViews_.at("TANGENT").buffer_.Get(),
                primitive.vertexBufferViews_.at("TEXCOORD_0").buffer_.Get(),
                primitive.vertexBufferViews_.at("JOINTS_0").buffer_.Get(),
                primitive.vertexBufferViews_.at("WEIGHTS_0").buffer_.Get(),
                primitive.vertexBufferViews_.at("JOINTS_1").buffer_.Get(),
                primitive.vertexBufferViews_.at("WEIGHTS_1").buffer_.Get(),
            };
            UINT strides[]
            {
                static_cast<UINT>(primitive.vertexBufferViews_.at("POSITION").strideInBytes_),
                static_cast<UINT>(primitive.vertexBufferViews_.at("NORMAL").strideInBytes_),
                static_cast<UINT>(primitive.vertexBufferViews_.at("TANGENT").strideInBytes_),
                static_cast<UINT>(primitive.vertexBufferViews_.at("TEXCOORD_0").strideInBytes_),
                static_cast<UINT>(primitive.vertexBufferViews_.at("JOINTS_0").strideInBytes_),
                static_cast<UINT>(primitive.vertexBufferViews_.at("WEIGHTS_0").strideInBytes_),
                static_cast<UINT>(primitive.vertexBufferViews_.at("JOINTS_1").strideInBytes_),
                static_cast<UINT>(primitive.vertexBufferViews_.at("WEIGHTS_1").strideInBytes_),
            };
            UINT offsets[_countof(vertexBuffers)]{ 0 };
            deviceContext->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, strides, offsets);
            deviceContext->IASetIndexBuffer(primitive.indexBufferView_.buffer_.Get(), primitive.indexBufferView_.format_, 0);

            PrimitiveConstants primitiveData{};
            primitiveData.material_ = primitive.material_;
            primitiveData.hasTangent_ = primitive.vertexBufferViews_.at("TANGENT").buffer_ != NULL;
            primitiveData.skin_ = node.skin_;
            DirectX::XMStoreFloat4x4(&primitiveData.world_, DirectX::XMLoadFloat4x4(&node.globalTransform_) * world);

            deviceContext->UpdateSubresource(primitiveCbuffer_.Get(), 0, 0, &primitiveData, 0, 0);
            deviceContext->VSSetConstantBuffers(0, 1, primitiveCbuffer_.GetAddressOf());
            deviceContext->PSSetConstantBuffers(0, 1, primitiveCbuffer_.GetAddressOf());

            //  Texture
            {
                const Material& material{ materials_.at(primitive.material_) };
                const int textureIndices[]
                {
                    material.data_.pbrMetallicRoughness_.baseColorTexture_.index_,
                    material.data_.pbrMetallicRoughness_.metallicRoughnessTexture_.index_,
                    material.data_.normalTexture_.index_,
                    material.data_.emissiveTexture_.index_,
                    material.data_.occlusionTexture_.index_,
                };
                ID3D11ShaderResourceView* nullShaderResourceView{};
                std::vector<ID3D11ShaderResourceView*> shaderResourceViews(_countof(textureIndices));
                for (int textureIndex = 0; textureIndex < shaderResourceViews.size(); ++textureIndex)
                {
                    shaderResourceViews.at(textureIndex) = textureIndices[textureIndex] > -1 ?
                        textureResourceViews_.at(textures_.at(textureIndices[textureIndex]).source_).Get() :
                        nullShaderResourceView;
                }
                deviceContext->PSSetShaderResources(1, static_cast<UINT>(shaderResourceViews.size()), shaderResourceViews.data());
            }

            deviceContext->DrawIndexed(static_cast<UINT>(primitive.indexBufferView_.count()), 0, 0);

        }
    }

    for (std::vector<int>::value_type childIndex : node.children_)
    {
        traverse(childIndex);
    }
} };
#endif

    for (std::vector<int>::value_type nodeIndex : scenes_.at(0).nodes_)
    {
        traverse(nodeIndex);
    }
}

//  デバッグ描画
void GltfModel::DrawDebug()
{
    if (ImGui::TreeNode(u8"Animation"))
    {
        ImGui::InputInt("CurrentAnimationIndex", &currentAnimationIndex_);      //  現在のアニメーション番号
        ImGui::SliderFloat("TransitionTime", &transitionTime_, 0.0f, 5.0f);
        ImGui::DragInt("AnimationClip", &animationClip_, 0.0f, 5.0f);
        ImGui::DragInt("TransitionState", &transitionState_, 0.0f, 5.0f);
        ImGui::DragFloat("Factor", &factor_, 0.0f, 5.0f);
        ImGui::DragFloat("Time", &currentAnimationSeconds_, 0.0f, 5.0f);
        ImGui::TreePop();
    }
}

bool NullLoadImageData(tinygltf::Image*, const int, std::string*, std::string*,
    int, int, const unsigned char*, int, void*)
{
    return true;
}