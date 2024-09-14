#pragma once

#include "Camera.h"

namespace Volcano {

	class RendererModel {

	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction);
		static void EndScene(bool shadow = false);
		static void Flush(bool shadow = false);

		static void DrawModel(const glm::mat4& transform, const glm::mat3& normalTransform, std::string& modelPath, int entityID);
	private:
		static void StartBatch();
		static void NextBatch();

	};
}