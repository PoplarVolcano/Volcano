#pragma once

#include "MeshTemp.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Volcano {

    struct MeshData
    {
        std::string name;
        glm::mat4 transform;
        Ref<MeshTemp> mesh;
        std::vector<std::pair<ImageType, Ref<Texture>>> textures;
        Ref<MeshData> parent;
        std::vector<Ref<MeshData>> children;
    };

    struct BoneInfo
    {
        /*finalBoneMatrices的索引*/
        int id;

        // offset矩阵将顶点从模型空间转换到骨骼空间
        glm::mat4 offset;

    };

    class ModelLibrary;

    class ModelTemp
    {
    public:
        static Ref<ModelTemp> Create(const char* path, bool gamma = false);
        static const Scope<ModelLibrary>& GetModelLibrary();
        ModelTemp(const char* path, bool gamma = false);
        std::string& GetPath() { return m_Path; }
        Ref<MeshData>& GetMeshRoot() { return m_MeshRoot; }
    private:
        std::string m_Path;

        bool gammaCorrection;

        Ref<MeshData> m_MeshRoot;

        Ref<Texture2D> m_BlackTexture;

        std::string directory;

        void loadModel(std::string path);
        void processNode(aiNode* node, const aiScene* scene, Ref<MeshData>& meshData);
        Ref<MeshData> processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<std::pair<ImageType, Ref<Texture>>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, ImageType imageType);
        
        static std::once_flag init_flag;
        static Scope<ModelLibrary> m_ModelLibrary;

    };


    class ModelLibrary {
    public:
        void Add(const Ref<ModelTemp> model);
        void Add(const std::string& path, const Ref<ModelTemp> model);
        Ref<ModelTemp> Load(const std::string filepath);
        Ref<ModelTemp> Get(const std::string& path);
        bool Exists(const std::string& path);
    private:
        std::unordered_map<std::string, Ref<ModelTemp>> m_Models;
    };
}