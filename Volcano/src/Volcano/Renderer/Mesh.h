#pragma once

#include "glm/glm.hpp"
#include "Shader.h"

#include "VertexArray.h"
#include "Texture.h"

namespace Volcano {

	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;	// ����
		glm::vec3 Bitangent;// ˫����

		int EntityID;
	};

	struct MeshTexture
	{
		Ref<Texture2D> texture;
		std::string type;
		std::string path;
	};

	class Mesh
	{
	public:
		Ref<VertexArray> va;
		Ref<VertexBuffer> vb;

		Mesh(std::vector<MeshVertex> vertices, std::vector<uint32_t> indices, std::vector<MeshTexture> textures);
		void Draw(Shader& shader, const glm::mat4& transform, const glm::mat3& normalTransform, int entityID);
	private:
		// ��������
		std::vector<MeshVertex> vertices;
		// ��������
		std::vector<uint32_t> indices;
		std::vector<MeshTexture> textures;

		MeshVertex* vertexBufferBase = nullptr;
		MeshVertex* vertexBufferPtr  = nullptr;

		void setupMesh();

	};
}