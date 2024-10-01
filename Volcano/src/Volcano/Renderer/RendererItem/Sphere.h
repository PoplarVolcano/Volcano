#pragma once
#include <Volcano/Renderer/Camera.h>
#include <Volcano/Scene/Scene.h>

#include "Volcano/Scene/Components.h"

namespace Volcano {

	class Sphere
	{

	public:
        static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction);
		static void EndScene(RenderType type = RenderType::NORMAL);
		static void Flush(RenderType type = RenderType::NORMAL);

		static void DrawSphere(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawSphere(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);


		static void DrawSphere(const glm::mat4& transform, const glm::mat3& normalTransform, const glm::vec4& color, int entityID = -1);
		static void DrawSphere(const glm::mat4& transform, const glm::mat3& normalTransform, 
			const Ref<Texture2D>& albedo, const Ref<Texture2D>& normal, const Ref<Texture2D>& metallic,
			const Ref<Texture2D>& roughness, const Ref<Texture2D>& AO, const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);

		static void DrawSphere(const glm::mat4& transform, const glm::mat3& normalTransform, SphereRendererComponent& crc, int entityID);

		static void SetIrradianceMap(const Ref<TextureCube> irradianceMap);
		static void SetPrefilterMap(const Ref<TextureCube> prefilterMap);
		static void SetBRDFLUT(const Ref<Texture2D> BRDFLUT);

	private:
		static void StartBatch();
		static void NextBatch();


	};
}