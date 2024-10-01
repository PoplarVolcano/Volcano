#pragma once
#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Camera.h"
#include "Volcano/Renderer/Texture.h"

namespace Volcano {


	class Skybox
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void EndScene();

		static void DrawSkybox();

		static void SetTexture(Ref<TextureCube> texture);
	};
}