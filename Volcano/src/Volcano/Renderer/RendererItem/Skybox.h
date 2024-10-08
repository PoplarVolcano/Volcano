#pragma once
#include "Volcano/Renderer/Camera.h"
#include "Volcano/Renderer/VertexArray.h"
#include "Volcano/Renderer/Texture.h"
#include "Volcano/Renderer/Shader.h"

namespace Volcano {

	struct SkyboxData
	{
		Ref<VertexArray> VertexArray;
		Ref<Shader> Shader;
		Ref<TextureCube> Texture;
	};

	class Skybox
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void EndScene();

		static void DrawSkybox();
		static void DrawIndexed();

		static void SetTexture(Ref<TextureCube> texture);
	private:
		static Ref<SkyboxData> m_SkyboxData;
	};
}