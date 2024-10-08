#pragma once

#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Buffer.h"
#include "Volcano/Renderer/Shader.h"
#include "Volcano/Renderer/Texture.h"

namespace Volcano {

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoords;
		float TextureIndex;
		float TilingFactor;

		// Editor-only
		int EntityID;
	};


	struct QuadData
	{
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> vertexArray;
		Ref<VertexBuffer> vertexBuffer;
		Ref<Shader> shader;
		Ref<Texture2D> whiteTexture;

		uint32_t indexCount = 0;
		QuadVertex* vertexBufferBase = nullptr;
		QuadVertex* vertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> textureSlots;
		uint32_t textureSlotIndex = 1;// 0 = white texture

		glm::vec4 vertexPosition[4];
	};

	class Quad
	{
	public:
		static void Init();
		static Ref<QuadData> GetQuadData() { return m_QuadData; }
		static void DrawIndexed();
	private:
		Quad();
		static Ref<QuadData> m_QuadData;

	};
}