#pragma once

#include "glm/glm.hpp"
#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Buffer.h"
#include "Volcano/Renderer/Shader.h"
#include "Volcano/Renderer/Texture.h"

namespace Volcano {

	struct CubeVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		// Editor-only
		int EntityID;
	};

	struct CubeData
	{
		static const uint32_t VertexSize = 36;
		static const uint32_t IndexSize = 36;
		static const uint32_t MaxCubes = 20000;
		static const uint32_t MaxVertices = MaxCubes * VertexSize;
		static const uint32_t MaxIndices = MaxCubes * IndexSize;
		static const uint32_t MaxTextureSlots = 32;// RenderCaps

		Ref<VertexArray>  vertexArray;
		Ref<VertexBuffer> vertexBuffer;
		Ref<Shader>       shader;
		Ref<Texture2D>    whiteTexture;
		Ref<Texture2D>    shadowMap;

		uint32_t indexCount = 0;
		CubeVertex* vertexBufferBase = nullptr;
		CubeVertex* vertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> textureSlots;
		uint32_t textureSlotIndex = 1;// 0 = white texture

		glm::vec3 vertexPosition[36];
		glm::vec2 texCoords[36];
		glm::vec3 normal[36];
		glm::vec3 tangent[36];
		glm::vec3 bitangent[36];
	};

	class Cube
	{
	public:
		static void Init();
		static Ref<CubeData> GetCubeData() { return m_CubeData; }
		static void DrawIndexed();
	private:
		Cube();
		static Ref<CubeData> m_CubeData;

	};
}