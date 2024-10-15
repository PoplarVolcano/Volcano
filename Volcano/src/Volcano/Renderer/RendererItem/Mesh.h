#pragma once

#include "glm/glm.hpp"
#include "Volcano/Scene/Scene.h"
#include "Volcano/Renderer/Camera.h"
#include "Volcano/Renderer/Texture.h"
#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/RendererItem/Bone.h"

namespace Volcano {

	class Entity;

	enum class ImageType
	{
		Albedo,
		Diffuse = Albedo,
		Metallic,
		Specular = Metallic,
		Normal,
		Height,
		Roughness,
		AO
	};

	enum class MeshType
	{
		None,
		Quad,
		Circle,
		Line,
		Cube,
		Sphere,
		Model
	};

#define MAX_BONE_INFLUENCE 4

	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		glm::vec3 Normal;
		glm::vec3 Tangent;	// ÇÐÏß
		glm::vec3 Bitangent;// Ë«ÇÐÏß
		//bone indexes which will influence this vertex
		int BoneIDs[MAX_BONE_INFLUENCE];
		//weights from each bone
		float Weights[MAX_BONE_INFLUENCE];

		int EntityID;
		MeshVertex()
		{
			for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; i++)
				BoneIDs[i] = -1;
		}
	};

	class Mesh
	{
	public:
		static void Init();
		
		void Shutdown();
		virtual void BeginScene(const Camera& camera, const glm::mat4& transform, const glm::vec3& position);
		void EndScene();
		void Flush();
		void StartBatch();
		void NextBatch();
		virtual void Draw();

		virtual void DrawMesh(int entityID, std::vector<glm::mat4>& finalBoneMatrices);

		void BindTextures(std::vector<std::pair<ImageType, Ref<Texture>>> textures);
		void BindShader(RenderType type = RenderType::NORMAL);

		void ResetVertexBufferBase();
		void SetEntity(Entity* entity) { m_Entity = entity; }
		void SetBoneID(int vertexIndex1, int vertexIndex2, int boneIndex, float weight);

		uint32_t GetVertexSize() { return m_VertexSize; }

		void SetVertexBoneDataToDefault();

		static void SetVertexBoneDataToDefault(MeshVertex& vertex);
		static void SetVertexBoneData(MeshVertex& vertex, int boneID, float weight);
	protected:
		Mesh() = default;
		virtual void SetupMesh();

	protected:

		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		std::vector<MeshVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		//Ref<Bone> m_Bones[MAX_BONE_INFLUENCE];

		uint32_t m_IndexCount = 0;
		uint32_t m_VertexSize;
		uint32_t m_IndexSize;
		uint32_t MaxMeshes;
		uint32_t MaxVertices;
		uint32_t MaxIndices;

		MeshVertex* vertexBufferBase = nullptr;
		MeshVertex* vertexBufferPtr = nullptr;

		Entity* m_Entity = nullptr;

		static Ref<Texture2D> m_WhiteTextures[2];
		static Ref<Texture2D> m_BlackTextures[5];
	};

}