#pragma once

#include <glm/glm.hpp>

#include "Volcano/Renderer/Texture.h"
#include "Volcano/Renderer/Camera.h"
#include "Volcano/Renderer/EditorCamera.h"
#include "Volcano/Scene/Components.h"

namespace Volcano {

	class VOL_API Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void EndScene();

		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f, float fade = 0.005f, int entityID = -1);

		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int entityID = -1);
		static float GetLineWidth();
		static void SetLineWidth(float width);
		// 画矩形
		static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID = -1);
		static void DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);

		struct Statistics
		{
			uint32_t DrawCalls = 0;

			uint32_t QuadCount = 0;
			uint32_t CircleCount = 0;
			uint32_t LineCount = 0;

			uint32_t GetTotalVartexCount() { return (QuadCount + CircleCount) * 4 + LineCount * 2; }
			uint32_t GetTotalIndexCount() { return (QuadCount + CircleCount) * 6; }
		};
		static void ResetStats();
		static Statistics GetStats();
	private:
		static void StartBatch();
		static void NextBatch();
		static void Flush();

	};

}