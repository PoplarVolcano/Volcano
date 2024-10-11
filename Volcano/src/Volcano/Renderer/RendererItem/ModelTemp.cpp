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

    // ���ļ����ؾ���֧��ASSIMP��չ����ģ�ͣ��������ɵ�����mesh�洢����������meshes�С�
    void ModelTemp::loadModel(std::string path)
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

        m_MeshRoot = std::make_shared<MeshData>();
        // �ݹ鴦��ASSIMP�ĸ��ڵ�
        processNode(scene->mRootNode, scene, m_MeshRoot);
    }

    // �Եݹ鷽ʽ����ڵ㡣����λ�ڽڵ㴦��ÿ���������񣬲������ӽڵ㣨����еĻ������ظ��˹��̡�
    // assimp��node�ֲ㣬node��meshΪһ�Զ��ϵ
    void ModelTemp::processNode(aiNode* node, const aiScene* scene, Ref<MeshData>& meshData)
    {
        meshData->name = node->mName.C_Str();
        meshData->transform = AssimpGLMHelpers::GetGLMMat4(node->mTransformation);
        // ����ڵ����е���������еĻ���
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            //�ڵ������������������ڶԳ����е�ʵ�ʶ������������ 
            //�������������е����ݣ��ڵ�ֻ��������֯����������ڵ�֮��Ĺ�ϵ����
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshData->children.push_back(processMesh(mesh, scene));
        }

        if (node->mNumChildren > 0)
        {
            Ref<MeshData> meshDataTemp = std::make_shared<MeshData>();
            meshData->children.push_back(meshDataTemp);
            meshDataTemp->parent = meshData;
            // �������������ӽڵ��ظ���һ����
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

        // ��ÿһ�������ȡλ�ã����ߣ���������
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            MeshTempVertex vertex;

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
        Ref<MeshData> meshData = std::make_shared<MeshData>();
        meshData->name = mesh->mName.C_Str();
        meshData->mesh = std::make_shared<MeshTemp>(vertices, indices);
        meshData->textures = textures;
        return meshData;
    }

    // ��ȡ��������
    std::vector<std::pair<ImageType, Ref<Texture>>> ModelTemp::loadMaterialTextures(aiMaterial* mat, aiTextureType type, ImageType imageType)
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


    void ModelLibrary::Add(const Ref<ModelTemp> model)
    {
        auto& path = model->GetPath();
        Add(path, model);
    }

    void ModelLibrary::Add(const std::string& path, const Ref<ModelTemp> Model)
    {

        VOL_CORE_ASSERT(!Exists(path), "ModelLibrary:Model�Ѿ�������");
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
        VOL_CORE_ASSERT(Exists(path), "ModelLibrary:δ�ҵ�Model");
        return m_Models[path];
    }

    bool ModelLibrary::Exists(const std::string& path)
    {
        return m_Models.find(path) != m_Models.end();
    }
}