#pragma once
#include "Volcano/Renderer/Camera.h"
#include "Volcano/Renderer/Framebuffer.h"

namespace Volcano {


	class Shadow
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene();
		static void EndScene();

		static void DrawShadow();
	private:
		static uint32_t m_DepthMapFBO;
		static uint32_t m_DepthCubemap;
	};
}