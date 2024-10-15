#include "volpch.h"
#include "Model.h"
#include "glad/glad.h"

#include "Volcano/Renderer/RendererItem/AssimpGLMHelpers.h"
#include "Volcano/Renderer/RendererItem/Animation.h"
#include "Volcano/Renderer/RendererItem/Animator.h"

#include <stb_image.h>
#include "ModelMesh.h"

namespace Volcano {

    std::once_flag Model::init_flag;
    Scope<ModelLibrary> Model::m_ModelLibrary;

    Ref<Model> Model::Create(const char* path, bool gamma)
    {
        return std::make_shared<Model>(path, gamma);
    }

    const Scope<ModelLibrary>& Model::GetModelLibrary()
    {
        std::call_once(init_flag, []() { m_ModelLibrary.reset(new ModelLibrary()); });
        return m_ModelLibrary;
    }

    Model::Model(const char* path, bool gamma) : gammaCorrection(gamma)
    {
        loadModel(path);

        m_Animation = std::make_shared<Animation>(path, this);

        m_Path = path;
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
                Mesh::SetVertexBoneData(vertices[vertexId], boneID, weight);
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
        directory = path.substr(0, path.find_last_of('\\'));

        m_ModelNodeRoot = std::make_shared<ModelNode>();
        // �ݹ鴦��ASSIMP�ĸ��ڵ�
        processNode(scene->mRootNode, scene, m_ModelNodeRoot);
    }

    // �Եݹ鷽ʽ����ڵ㡣����λ�ڽڵ㴦��ÿ���������񣬲������ӽڵ㣨����еĻ������ظ��˹��̡�
    // assimp��node�ֲ㣬node��meshΪһ�Զ��ϵ
    void Model::processNode(aiNode* node, const aiScene* scene, Ref<ModelNode>& modelNode)
    {
        modelNode->name = node->mName.data;
        modelNode->transform = AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation);
        // ����ڵ����е���������еĻ���
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            //�ڵ������������������ڶԳ����е�ʵ�ʶ������������ 
            //�������������е����ݣ��ڵ�ֻ��������֯����������ڵ�֮��Ĺ�ϵ����
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            modelNode->numMeshes++;
            modelNode->meshes.push_back(m_Meshes.size());
            processMesh(mesh, scene);
        }

        if (node->mNumChildren > 0)
        {
            modelNode->numChildren++;
            Ref<ModelNode> modelNodeTemp = std::make_shared<ModelNode>();
            modelNode->children.push_back(modelNodeTemp);
            modelNodeTemp->parent = modelNode;
            // �������������ӽڵ��ظ���һ����
            for (uint32_t i = 0; i < node->mNumChildren; i++)
            {
                processNode(node->mChildren[i], scene, modelNodeTemp);
            }
        }
    }

    Ref<MeshNode> Model::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<MeshVertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<std::pair<ImageType, Ref<Texture>>> textures;

        // ��ÿһ�������ȡλ�ã����ߣ���������
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            MeshVertex vertex;
            Mesh::SetVertexBoneDataToDefault(vertex);

            // ������λ�á����ߺ���������
            vertex.Position = AssimpGLMHelpers::GetGLMVec3(mesh->mVertices[i]);
            vertex.Normal = AssimpGLMHelpers::GetGLMVec3(mesh->mNormals[i]);

            if (mesh->mTextureCoords[0]) // �����Ƿ����������ꣿ
            {
                //һ�����������԰���8����ͬ���������ꡣ���費ʹ�ö�������ж�����������ģ�ͣ�����ȡ��һ�飨0����
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
            std::vector<std::pair<ImageType, Ref<Texture>>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, ImageType::Diffuse);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<std::pair<ImageType, Ref<Texture>>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, ImageType::Specular);
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            std::vector<std::pair<ImageType, Ref<Texture>>> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, ImageType::Normal);
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            std::vector<std::pair<ImageType, Ref<Texture>>> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, ImageType::Height);
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        }

        ExtractBoneWeightForVertices(vertices, mesh, scene);

        Ref<MeshNode> meshNode = std::make_shared<MeshNode>();
        meshNode->name = mesh->mName.data;
        meshNode->mesh = std::make_shared<ModelMesh>(vertices, indices);
        meshNode->textures = textures;
        m_Meshes.push_back(meshNode);
        return meshNode;
    }

    // ��ȡ��������
    std::vector<std::pair<ImageType, Ref<Texture>>> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, ImageType imageType)
    {
        std::vector<std::pair<ImageType, Ref<Texture>>> textures;
        for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
        {
            // ��ȡ�������obj��·��
            aiString str;
            mat->GetTexture(type, i, &str);
            std::string path = str.data;
            path = directory + '/' + path;
            auto texture = Texture2D::Create(path, false);
            textures.push_back({ imageType, texture });
        }
        return textures;
    }


    void ModelLibrary::Add(const Ref<Model> model)
    {
        auto& path = model->GetPath();
        Add(path, model);
    }

    void ModelLibrary::Add(const std::string& path, const Ref<Model> Model)
    {

        VOL_CORE_ASSERT(!Exists(path), "ModelLibrary:Model�Ѿ�������");
        m_Models[path] = Model;
    }

    Ref<Model> ModelLibrary::Load(const std::string filepath)
    {
        auto Model = Model::Create(filepath.c_str());
        Add(Model);
        return Model;
    }


    Ref<Model> ModelLibrary::Get(const std::string& path)
    {
        if (!Exists(path))
        {
            VOL_CORE_TRACE("ModelLibrary:δ�ҵ�Model");
            return nullptr;
        }
        return m_Models[path];
    }

    bool ModelLibrary::Exists(const std::string& path)
    {
        return m_Models.find(path) != m_Models.end();
    }
}