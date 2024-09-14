#pragma once
#include <Volcano/Renderer/VertexArray.h>
#include "Volcano/Renderer/Camera.h"

namespace Volcano {


	class Skybox
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void EndScene();

		static void DrawSkybox();
	};
}