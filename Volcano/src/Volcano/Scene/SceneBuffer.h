#pragma once

#include "Volcano/Core/Core.h"
#include "Volcano/Renderer/LightRenderer.h"
#include "Volcano/Scene/LinearAllocator.h"

namespace Volcano
{
	struct LightCount
	{
		int directionalLightCount;
		int pointLightCount;
		int spotLightCount;
		int directionalLightShadowCount;
		int pointLightShadowCount;
		int spotLightShadowCount;
	};

	class Entity;

	struct SceneBuffer
	{
		static SceneBuffer& GetInstance()
		{
			static SceneBuffer instance;
			return instance;
		}

		Ref<Entity> cameraEntity;

		std::vector<Ref<Entity>> forwardShadingEntity;
		std::vector<Ref<Entity>> GBufferEntityList;
		std::vector<Ref<Entity>> normalEntityList;
		std::vector<Ref<Entity>> outlineEntityList;

		std::vector<Ref<Entity>> lightVolumePointLightEntityList;
		std::vector<Ref<Entity>> pointLightEntityList;
		std::vector<Ref<Entity>> lightVolumeSpotLightEntityList;
		std::vector<Ref<Entity>> spotLightEntityList;

		std::vector<Ref<Entity>> scriptEntityList;

		LightCount lightCount;

		std::vector<Ref<Entity>> particleSystemEntityList;
		Ref<Entity> skyboxEntity;
		Ref<Entity> hdrEntity;

		std::unordered_map<Entity*, glm::mat4> entityWorldTransformMap;
		std::unordered_map<Entity*, glm::quat> entityWorldRotationMap;

		LinearAllocator linearAllocator{ 16 * 1024 * 1026 };

		void Clear()
		{
			cameraEntity = nullptr;

			forwardShadingEntity.clear();
			GBufferEntityList.clear();
			normalEntityList.clear();
			outlineEntityList.clear();

			lightVolumePointLightEntityList.clear();
			pointLightEntityList.clear();
			lightVolumeSpotLightEntityList.clear();
			spotLightEntityList.clear();
			
			scriptEntityList.clear();

			lightCount = { 0, 0, 0, 0, 0, 0 };

			particleSystemEntityList.clear();

			skyboxEntity = nullptr;
			hdrEntity = nullptr;

			entityWorldTransformMap.clear();
			entityWorldRotationMap.clear();
		}
	};
	
}