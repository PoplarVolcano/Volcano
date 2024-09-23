#pragma once

#include "glm/glm.hpp"
#include "Volcano/Renderer/Shader.h"
#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Texture.h"

namespace Volcano {

	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;	// 切线
		glm::vec3 Bitangent;// 双切线
		float DiffuseIndex;
		float SpecularIndex;
		float NormalIndex;
		float ParallaxIndex;

		int EntityID;
	};

	struct MeshTexture
	{
		Ref<Texture2D> texture;
		std::string type;
		std::string path;
		float textureIndex;
	};

	class Mesh
	{
	public:
		Ref<VertexArray> va;
		Ref<VertexBuffer> vb;

		Mesh(std::vector<MeshVertex> vertices, std::vector<uint32_t> indices, std::vector<MeshTexture> textures);
		void Draw(Shader& shader, const glm::mat4& transform, const glm::mat3& normalTransform, int entityID);
		void DrawIndexed();
	private:
		// 顶点数据
		std::vector<MeshVertex> vertices;
		// 索引数据
		std::vector<uint32_t> indices;
		std::vector<MeshTexture> textures;

		MeshVertex* vertexBufferBase = nullptr;
		MeshVertex* vertexBufferPtr  = nullptr;

		void setupMesh();

	};
}