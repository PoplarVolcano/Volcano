#include "volpch.h"
#include "Model.h"
#include "glad/glad.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

namespace Volcano {

    Ref<Model> Model::Create(const char* path, bool gamma)
    {
        return std::make_shared<Model>(path, gamma);
    }

    Model::Model(const char* path, bool gamma) : gammaCorrection(gamma)
    {
        m_BlackTexture = Texture2D::Create(1, 1);
        uint32_t blackTextureData = 0x00000000;
        m_BlackTexture->SetData(&blackTextureData, sizeof(uint32_t));

        MeshTexture texture;
        texture.texture = m_BlackTexture;
        texture.type = "BlackTexture";
        texture.path = "";
        texture.textureIndex = 0.0f;
        textures_loaded.push_back(texture);

        loadModel(path);
        m_Path = path;
    }

    void Model::Draw(Shader& shader, const glm::mat4& transform, const glm::mat3& normalTransform, int entityID)
    {
        for (uint32_t i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader, transform, normalTransform, entityID);
    }

    void Model::DrawIndexed()
    {
        // Bind textures
        for (uint32_t i = 0; i < textures_loaded.size(); i++)
            textures_loaded[i].texture->Bind(i);

        for (uint32_t i = 0; i < meshes.size(); i++)
            meshes[i].DrawIndexed();
    }

    // 从文件加载具有支持ASSIMP扩展名的模型，并将生成的网格mesh存储在网格向量meshes中。
    void Model::loadModel(std::string path)
    {
        Assimp::Importer import;
        const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }
        // 获取obj文件路径
        directory = path.substr(0, path.find_last_of('/'));

        // 递归处理ASSIMP的根节点
        processNode(scene->mRootNode, scene);
    }

    // 以递归方式处理节点。处理位于节点处的每个单独网格，并在其子节点（如果有的话）上重复此过程。
    void Model::processNode(aiNode* node, const aiScene* scene)
    {
        // 处理节点所有的网格（如果有的话）
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            //节点对象仅包含索引，用于对场景中的实际对象进行索引。 
            //场景包含了所有的数据，节点只是用来组织东西（比如节点之间的关系）。
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // 接下来对它的子节点重复这一过程
        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<MeshVertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<MeshTexture> textures;

        // 对每一个顶点读取位置，法线，纹理坐标
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            MeshVertex vertex;
            // 处理顶点位置、法线和纹理坐标
            //因为assimp使用它自己的向量类，该类不直接转换为glm的vec3类，所以我们首先将数据传输到这个glm:：vec3。
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;

            if (mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
            {
                glm::vec2 vec;
                //一个顶点最多可以包含8个不同的纹理坐标。假设我们不使用顶点可以有多个纹理坐标的模型，我们总是取第一组（0）。
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                if (mesh->mTangents)
                {
                    // tangent
                    vector.x = mesh->mTangents[i].x;
                    vector.y = mesh->mTangents[i].y;
                    vector.z = mesh->mTangents[i].z;
                    vertex.Tangent = vector;
                }
                if (mesh->mBitangents)
                {
                    // bitangent
                    vector.x = mesh->mBitangents[i].x;
                    vector.y = mesh->mBitangents[i].y;
                    vector.z = mesh->mBitangents[i].z;
                    vertex.Bitangent = vector;
                }
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
            std::vector<MeshTexture> diffuseMaps  = loadMaterialTextures(material, aiTextureType_DIFFUSE,  "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(),  diffuseMaps.end());
            std::vector<MeshTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            std::vector<MeshTexture> normalMaps   = loadMaterialTextures(material, aiTextureType_HEIGHT,   "texture_normal");
            textures.insert(textures.end(), normalMaps.begin(),   normalMaps.end());
            std::vector<MeshTexture> heightMaps   = loadMaterialTextures(material, aiTextureType_AMBIENT,  "texture_height");
        }

        return Mesh(vertices, indices, textures);
    }

    // 读取材质纹理
    std::vector<MeshTexture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<MeshTexture> textures;
        for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
        {
            // 获取纹理相对obj的路径
            aiString str;
            mat->GetTexture(type, i, &str);
            //检查之前是否加载了纹理，如果是，继续下一次迭代：跳过加载新纹理
            bool skip = false;
            for (uint32_t j = 0; j < textures_loaded.size(); j++)
            {
                // 如果已加载，将已加载的纹理注入textures
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);// push_back在vector类中作用为在vector尾部加入一个数据；
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                std::string path = str.data;
                path = directory + '/' + path;
                MeshTexture texture;
                texture.texture = Texture2D::Create(path, false);
                texture.type = typeName;
                texture.path = str.C_Str();
                texture.textureIndex = textures_loaded.size();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }

        }
        return textures;
    }
}