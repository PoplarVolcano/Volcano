#pragma once

#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Volcano {

    class Animation;
    class Animator;

    struct ModelNode
    {
        std::string name;
        uint32_t numMeshes;
        std::vector<uint32_t> meshes;
        glm::mat4 transform;
        Ref<ModelNode> parent;
        uint32_t numChildren;
        std::vector<Ref<ModelNode>> children;
    };

    struct MeshNode
    {
        std::string name;
        Ref<Mesh> mesh;
        std::vector<std::pair<ImageType, Ref<Texture>>> textures;
    };

    struct BoneInfo
    {
        /*finalBoneMatrices������*/
        int id;

        // offset���󽫶����ģ�Ϳռ�ת���������ռ�
        glm::mat4 offset;

    };

    class ModelLibrary;

    class Model
    {
    public:
        static Ref<Model> Create(const char* path, bool gamma = false);
        static const Scope<ModelLibrary>& GetModelLibrary();

        Model(const char* path, bool gamma = false);

        std::string& GetPath() { return m_Path; }
        Ref<ModelNode>& GetModelNodeRoot() { return m_ModelNodeRoot; }
        std::map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
        int& GetBoneCount() { return m_BoneCounter; }
        Ref<Animation> GetAnimation() { return m_Animation; }
        std::vector<Ref<MeshNode>>& GetMeshes() { return m_Meshes; }
        
    private:
        std::string m_Path;

        bool gammaCorrection;

        Ref<ModelNode> m_ModelNodeRoot;

        std::vector<Ref<MeshNode>> m_Meshes;

        std::string directory;

        std::map<std::string, BoneInfo> m_BoneInfoMap;
        int m_BoneCounter = 0;

        Ref<Animation> m_Animation;

        void ExtractBoneWeightForVertices(std::vector<MeshVertex>& vertices, aiMesh* mesh, const aiScene* scene);

        void loadModel(std::string path);
        void processNode(aiNode* node, const aiScene* scene, Ref<ModelNode>& modelNode);
        Ref<MeshNode> processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<std::pair<ImageType, Ref<Texture>>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, ImageType imageType);

        static std::once_flag init_flag;
        static Scope<ModelLibrary> m_ModelLibrary;

    };


    class ModelLibrary {
    public:
        void Add(const Ref<Model> model);
        void Add(const std::string& path, const Ref<Model> model);
        Ref<Model> Load(const std::string filepath);
        Ref<Model> Get(const std::string& path);
        bool Exists(const std::string& path);
    private:
        std::unordered_map<std::string, Ref<Model>> m_Models;
    };
}