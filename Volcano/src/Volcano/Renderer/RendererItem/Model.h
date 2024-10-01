#pragma once

#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Volcano {
    
    struct BoneInfo
    {
        /*finalBoneMatrices的索引*/
        int id;

        // offset矩阵将顶点从模型空间转换到骨骼空间
        glm::mat4 offset;

    };

    class Model
    {
    public:
        static Ref<Model> Create(const char* path, bool gamma = false);
        Model(const char* path, bool gamma = false);
        void Draw(Shader& shader, const glm::mat4& transform, const glm::mat3& normalTransform, int entityID, std::vector<glm::mat4>& finalBoneMatrices);
        void DrawIndexed();
        std::string GetPath() { return m_Path; }
        auto& GetBoneInfoMap() { return m_BoneInfoMap; }
        int& GetBoneCount() { return m_BoneCounter; }

    private:
        std::string m_Path;

        bool gammaCorrection;

        std::vector<Mesh> meshes;
        std::vector<MeshTexture> textures_loaded; // 存储到目前为止加载的所有纹理，优化以确保纹理不会加载多次。

        Ref<Texture2D> m_BlackTexture;

        std::string directory;

        std::map<std::string, BoneInfo> m_BoneInfoMap;
        int m_BoneCounter = 0;

        void SetVertexBoneDataToDefault(MeshVertex& vertex);
        void SetVertexBoneData(MeshVertex& vertex, int boneID, float weight);
        void ExtractBoneWeightForVertices(std::vector<MeshVertex>& vertices, aiMesh* mesh, const aiScene* scene);

        void loadModel(std::string path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
    };
}