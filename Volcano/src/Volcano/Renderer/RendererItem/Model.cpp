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

    // ���ļ����ؾ���֧��ASSIMP��չ����ģ�ͣ��������ɵ�����mesh�洢����������meshes�С�
    void Model::loadModel(std::string path)
    {
        Assimp::Importer import;
        const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }
        // ��ȡobj�ļ�·��
        directory = path.substr(0, path.find_last_of('/'));

        // �ݹ鴦��ASSIMP�ĸ��ڵ�
        processNode(scene->mRootNode, scene);
    }

    // �Եݹ鷽ʽ����ڵ㡣����λ�ڽڵ㴦��ÿ���������񣬲������ӽڵ㣨����еĻ������ظ��˹��̡�
    void Model::processNode(aiNode* node, const aiScene* scene)
    {
        // ����ڵ����е���������еĻ���
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            //�ڵ������������������ڶԳ����е�ʵ�ʶ������������ 
            //�������������е����ݣ��ڵ�ֻ��������֯����������ڵ�֮��Ĺ�ϵ����
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // �������������ӽڵ��ظ���һ����
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

        // ��ÿһ�������ȡλ�ã����ߣ���������
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            MeshVertex vertex;
            // ������λ�á����ߺ���������
            //��Ϊassimpʹ�����Լ��������࣬���಻ֱ��ת��Ϊglm��vec3�࣬�����������Ƚ����ݴ��䵽���glm:��vec3��
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;

            if (mesh->mTextureCoords[0]) // �����Ƿ����������ꣿ
            {
                glm::vec2 vec;
                //һ�����������԰���8����ͬ���������ꡣ�������ǲ�ʹ�ö�������ж�����������ģ�ͣ���������ȡ��һ�飨0����
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

        // ����������mNumFaces���������е�ͼԪ�������Ρ�����Ρ�ֱ�ߣ���������
        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // �������
        // һ��Mesh��4�������ڶ�ȡ��ʱ����������ע��Model��textures_loaded��ͬʱ��������ע��Mesh��textures
        if (mesh->mMaterialIndex >= 0)
        {
            //���Ǽ�����ɫ���еĲ�����������һ��Լ����ÿ������������Ӧ��������Ϊ'texture_diffuseN'��
            // ����N�Ǵ�1��MAX_SAMPLER_NUMBER�����кš� 
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

    // ��ȡ��������
    std::vector<MeshTexture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<MeshTexture> textures;
        for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
        {
            // ��ȡ�������obj��·��
            aiString str;
            mat->GetTexture(type, i, &str);
            //���֮ǰ�Ƿ��������������ǣ�������һ�ε�������������������
            bool skip = false;
            for (uint32_t j = 0; j < textures_loaded.size(); j++)
            {
                // ����Ѽ��أ����Ѽ��ص�����ע��textures
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);// push_back��vector��������Ϊ��vectorβ������һ�����ݣ�
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