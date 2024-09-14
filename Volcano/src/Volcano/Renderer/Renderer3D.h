#pragma once

#include <glm/glm.hpp>

#include "Volcano/Renderer/Camera.h"

#include "Volcano/Scene/Components.h"

namespace Volcano {

	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();


		static void BeginScene(const Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction);
		static void EndScene(bool shadow = false);
		static void Flush(bool shadow = false);

		static void DrawCube(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawCube(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);


		static void DrawCube(const glm::mat4& transform, const glm::mat3& normalTransform, const glm::vec4& color, int entityID = -1);
		static void DrawCube(const glm::mat4& transform, const glm::mat3& normalTransform, const Ref<Texture2D>& diffuse, const Ref<Texture2D>& specular, const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);

		static void DrawCube(const glm::mat4& transform, const glm::mat3& normalTransform, CubeRendererComponent& crc, int entityID);

	private:
		static void StartBatch();
		static void NextBatch();


	};
}