#pragma once

#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Volcano {

    class Model
    {
    public:
        static Ref<Model> Create(const char* path, bool gamma = false);
        Model(const char* path, bool gamma = false);
        void Draw(Shader& shader, const glm::mat4& transform, const glm::mat3& normalTransform, int entityID);
        void DrawIndexed();
        std::string GetPath() { return m_Path; }
    private:
        std::string m_Path;

        bool gammaCorrection;

        std::vector<Mesh> meshes;
        std::vector<MeshTexture> textures_loaded; // 存储到目前为止加载的所有纹理，优化以确保纹理不会加载多次。

        Ref<Texture2D> m_BlackTexture;

        std::string directory;

        void loadModel(std::string path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
    };
}