#pragma once

#include "glm/glm.hpp"
#include "Volcano/Renderer/VertexArray.h"

namespace Volcano {

	struct FullQuadVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
	};

	class FullQuad
	{
	public:
		static void Init();
		static void DrawIndexed();
	private:
		FullQuad();
		static Ref<VertexArray> m_VertexArray;

	};
}