#pragma once

#include "glm/glm.hpp"
#include "Volcano/Scene/Scene.h"
#include "Volcano/Renderer/Camera.h"
#include "Volcano/Renderer/Texture.h"
#include "Volcano/Renderer/VertexArray.h"

namespace Volcano {

	class Entity;

	enum class ImageType
	{
		Diffuse,
		Specular,
		Normal,
		Height
	};

	enum class MeshType
	{
		None,
		Quad,
		Circle,
		Line,
		Cube,
		Sphere
	};

	struct MeshTempVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		glm::vec3 Normal;
		glm::vec3 Tangent;	// ÇÐÏß
		glm::vec3 Bitangent;// Ë«ÇÐÏß

		int EntityID;
	};

	class MeshTemp
	{
	public:

		MeshTemp(std::vector<MeshTempVertex> vertices, std::vector<uint32_t> indices);
		
		void Shutdown();
		virtual void BeginScene(const Camera& camera, const glm::mat4& transform, const glm::vec3& position);
		void EndScene();
		void Flush();
		void StartBatch();
		void NextBatch();

		virtual void DrawMesh(int entityID);

		void BindTextures(std::vector<std::pair<ImageType, Ref<Texture>>> textures);
		void BindShader(RenderType type = RenderType::NORMAL);

		void SetEntity(Entity* entity) { m_Entity = entity; }

	protected:
		virtual void SetupMesh();

	protected:
		MeshTemp() {};

		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<Texture2D> m_WhiteTexture;
		std::vector<MeshTempVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		MeshTempVertex* vertexBufferBase = nullptr;
		MeshTempVertex* vertexBufferPtr = nullptr;

		uint32_t m_IndexCount = 0;
		uint32_t m_IndexSize;
		uint32_t MaxMeshes;
		uint32_t MaxVertices;
		uint32_t MaxIndices;

		Entity* m_Entity = nullptr;
	};

}