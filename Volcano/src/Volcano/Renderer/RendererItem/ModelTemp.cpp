#include "volpch.h"
#include "ModelTemp.h"
#include "glad/glad.h"

#include "Volcano/Renderer/RendererItem/AssimpGLMHelpers.h"

#include <stb_image.h>

namespace Volcano {

    std::once_flag ModelTemp::init_flag;
    Scope<ModelLibrary> ModelTemp::m_ModelLibrary;

    Ref<ModelTemp> ModelTemp::Create(const char* path, bool gamma)
    {
        return std::make_shared<ModelTemp>(path, gamma);
    }

    const Scope<ModelLibrary>& ModelTemp::GetModelLibrary()
    {
        std::call_once(init_flag, []() { m_ModelLibrary.reset(new ModelLibrary()); });
        return m_ModelLibrary;
    }

    ModelTemp::ModelTemp(const char* path, bool gamma) : gammaCorrection(gamma)
    {
        loadModel(path);
        m_Path = path;
    }

    // 从文件加载具有支持ASSIMP扩展名的模型，并将生成的网格mesh存储在网格向量meshes中。
    void ModelTemp::loadModel(std::string path)
    {
        Assimp::Importer import;
        const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }
        // 获取obj文件路径
        directory = path.substr(0, path.find_last_of('\\'));

        m_MeshRoot = std::make_shared<MeshData>();
        // 递归处理ASSIMP的根节点
        processNode(scene->mRootNode, scene, m_MeshRoot);
    }

    // 以递归方式处理节点。处理位于节点处的每个单独网格，并在其子节点（如果有的话）上重复此过程。
    // assimp以node分层，node和mesh为一对多关系
    void ModelTemp::processNode(aiNode* node, const aiScene* scene, Ref<MeshData>& meshData)
    {
        meshData->name = node->mName.C_Str();
        meshData->transform = AssimpGLMHelpers::GetGLMMat4(node->mTransformation);
        // 处理节点所有的网格（如果有的话）
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            //节点对象仅包含索引，用于对场景中的实际对象进行索引。 
            //场景包含了所有的数据，节点只是用来组织东西（比如节点之间的关系）。
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshData->children.push_back(processMesh(mesh, scene));
        }

        if (node->mNumChildren > 0)
        {
            Ref<MeshData> meshDataTemp = std::make_shared<MeshData>();
            meshData->children.push_back(meshDataTemp);
            meshDataTemp->parent = meshData;
            // 接下来对它的子节点重复这一过程
            for (uint32_t i = 0; i < node->mNumChildren; i++)
            {
                processNode(node->mChildren[i], scene, meshDataTemp);
            }
        }
    }

    Ref<MeshData> ModelTemp::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<MeshTempVertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<std::pair<ImageType, Ref<Texture>>> textures;

        // 对每一个顶点读取位置，法线，纹理坐标
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            MeshTempVertex vertex;

            // 处理顶点位置、法线和纹理坐标
            vertex.Position = AssimpGLMHelpers::GetGLMVec3(mesh->mVertices[i]);
            vertex.Normal = AssimpGLMHelpers::GetGLMVec3(mesh->mNormals[i]);

            if (mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
            {
                //一个顶点最多可以包含8个不同的纹理坐标。假设不使用顶点可以有多个纹理坐标的模型，总是取第一组（0）。
                vertex.TexCoords = AssimpGLMHelpers::GetGLMVec2(mesh->mTextureCoords[0][i]);
                if (mesh->mTangents)
                    vertex.Tangent = AssimpGLMHelpers::GetGLMVec3(mesh->mTangents[i]);
                if (mesh->mBitangents)
                    vertex.Bitangent = AssimpGLMHelpers::GetGLMVec3(mesh->mBitangents[i]);
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        // 处理索引，mNumFaces：此网格中的图元（三角形、多边形、直线）的数量。
        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // 处理材质
        // 一个Mesh有4种纹理，在读取的时候将所有纹理注入Model的textures_loaded，同时将该纹理注入Mesh的textures
        if (mesh->mMaterialIndex >= 0)
        {
            //我们假设着色器中的采样器名称有一个约定。每个漫反射纹理都应该命名作为'texture_diffuseN'，
            // 其中N是从1到MAX_SAMPLER_NUMBER的序列号。 
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<std::pair<ImageType, Ref<Texture>>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, ImageType::Diffuse);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<std::pair<ImageType, Ref<Texture>>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, ImageType::Specular);
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            std::vector<std::pair<ImageType, Ref<Texture>>> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, ImageType::Normal);
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            std::vector<std::pair<ImageType, Ref<Texture>>> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, ImageType::Height);
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        }
        Ref<MeshData> meshData = std::make_shared<MeshData>();
        meshData->name = mesh->mName.C_Str();
        meshData->mesh = std::make_shared<MeshTemp>(vertices, indices);
        meshData->textures = textures;
        return meshData;
    }

    // 读取材质纹理
    std::vector<std::pair<ImageType, Ref<Texture>>> ModelTemp::loadMaterialTextures(aiMaterial* mat, aiTextureType type, ImageType imageType)
    {
        std::vector<std::pair<ImageType, Ref<Texture>>> textures;
        for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
        {
            // 获取纹理相对obj的路径
            aiString str;
            mat->GetTexture(type, i, &str);
            std::string path = str.data;
            path = directory + '/' + path;
            auto texture = Texture2D::Create(path, false);
            textures.push_back({ imageType, texture });
        }
        return textures;
    }


    void ModelLibrary::Add(const Ref<ModelTemp> model)
    {
        auto& path = model->GetPath();
        Add(path, model);
    }

    void ModelLibrary::Add(const std::string& path, const Ref<ModelTemp> Model)
    {

        VOL_CORE_ASSERT(!Exists(path), "ModelLibrary:Model已经存在了");
        m_Models[path] = Model;
    }

    Ref<ModelTemp> ModelLibrary::Load(const std::string filepath)
    {
        auto Model = ModelTemp::Create(filepath.c_str());
        Add(Model);
        return Model;
    }


    Ref<ModelTemp> ModelLibrary::Get(const std::string& path)
    {
        VOL_CORE_ASSERT(Exists(path), "ModelLibrary:未找到Model");
        return m_Models[path];
    }

    bool ModelLibrary::Exists(const std::string& path)
    {
        return m_Models.find(path) != m_Models.end();
    }
}