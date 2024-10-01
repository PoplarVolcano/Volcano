#include "volpch.h"
#include "Model.h"
#include "glad/glad.h"

#include "Volcano/Renderer/RendererItem/AssimpGLMHelpers.h"

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

    void Model::Draw(Shader& shader, const glm::mat4& transform, const glm::mat3& normalTransform, int entityID, std::vector<glm::mat4>& finalBoneMatrices)
    {
        for (uint32_t i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader, transform, normalTransform, entityID, finalBoneMatrices);
    }

    void Model::DrawIndexed()
    {
        // Bind textures
        for (uint32_t i = 0; i < textures_loaded.size(); i++)
            textures_loaded[i].texture->Bind(i);

        for (uint32_t i = 0; i < meshes.size(); i++)
            meshes[i].DrawIndexed();
    }

    void Model::SetVertexBoneDataToDefault(MeshVertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.BoneIDs[i] = -1;
            vertex.Weights[i] = 0.0f;
        }
    }

    void Model::SetVertexBoneData(MeshVertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.BoneIDs[i] < 0)
            {
                vertex.Weights[i] = weight;
                vertex.BoneIDs[i] = boneID;
                break;
            }
        }
    }

    void Model::ExtractBoneWeightForVertices(std::vector<MeshVertex>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = m_BoneCounter;
                newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                m_BoneInfoMap[boneName] = newBoneInfo;
                boneID = m_BoneCounter;
                m_BoneCounter++;
            }
            else
            {
                boneID = m_BoneInfoMap[boneName].id;
            }
            VOL_CORE_ASSERT(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                VOL_CORE_ASSERT(vertexId <= vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
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
            SetVertexBoneDataToDefault(vertex);

            // ������λ�á����ߺ���������
            vertex.Position = AssimpGLMHelpers::GetGLMVec3(mesh->mVertices[i]);
            vertex.Normal = AssimpGLMHelpers::GetGLMVec3(mesh->mNormals[i]);

            if (mesh->mTextureCoords[0]) // �����Ƿ����������ꣿ
            {
                //һ�����������԰���8����ͬ���������ꡣ�������ǲ�ʹ�ö�������ж�����������ģ�ͣ���������ȡ��һ�飨0����
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

        ExtractBoneWeightForVertices(vertices, mesh, scene);

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