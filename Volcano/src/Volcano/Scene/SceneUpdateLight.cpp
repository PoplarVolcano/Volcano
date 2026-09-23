#include "volpch.h"
#include "Scene.h"

#include "Volcano/Scene/Entity.h"
#include "Volcano/Renderer/LightRenderer.h"
#include "Volcano/Renderer/UniformBuffer.h"
#include <Volcano/Renderer/RendererItem/Quad.h>

#include "Volcano/Scene/SceneBuffer.h"

namespace Volcano
{
	void Scene::UpdateLight()
	{
		auto& sceneBuffer = SceneBuffer::GetInstance();
		auto& pointLightEntityList = sceneBuffer.pointLightEntityList;
		sceneBuffer.linearAllocator.Reset();
		PointLight* pointLights = (PointLight*)sceneBuffer.linearAllocator.Allocate(pointLightEntityList.size() * sizeof(PointLight));
		for (int i = 0; i != pointLightEntityList.size(); i++)
		{
			auto& entity = pointLightEntityList[i];
			glm::mat4 worldTransform = entity->GetWorldTransform();
			auto& lightComponent = entity->GetComponent<LightComponent>();

			auto& light = pointLights[i];
			light.position      = glm::vec3(worldTransform[3]);
			light.ambient       = lightComponent.ambient;
			light.diffuse       = lightComponent.diffuse;
			light.specular      = lightComponent.specular;
			light.constant      = lightComponent.constant;
			light.linear        = lightComponent.linear;
			light.quadratic     = lightComponent.quadratic;
			light.radius        = lightComponent.radius;
			light.shadowEnabled = lightComponent.shadowEnabled;
		}

		sceneBuffer.lightCount.pointLightCount = pointLightEntityList.size();
		UniformBufferManager::GetUniformBuffer("LightCount")->SetData(&sceneBuffer.lightCount.pointLightCount, sizeof(int), sizeof(int));
		UniformBufferManager::GetUniformBuffer("PointLight")->SetData(pointLights, sceneBuffer.lightCount.pointLightCount * sizeof(PointLight));

		auto& spotEntityList = sceneBuffer.spotLightEntityList;
		sceneBuffer.linearAllocator.Reset();
		SpotLight* spotLights = (SpotLight*)sceneBuffer.linearAllocator.Allocate(spotEntityList.size() * sizeof(SpotLight));
		for (int i = 0; i != spotEntityList.size(); i++)
		{
			auto& entity = spotEntityList[i];
			glm::mat4 worldTransform = entity->GetWorldTransform();
			auto& lightComponent = entity->GetComponent<LightComponent>();

			auto& light = spotLights[i];
			light.position      = glm::vec3(worldTransform[3]);
			light.direction     = glm::rotate(entity->GetWorldRotation(), glm::vec3(0.0f, 0.0f, -1.0f));
			light.ambient       = lightComponent.ambient;
			light.diffuse       = lightComponent.diffuse;
			light.specular      = lightComponent.specular;
			light.constant      = lightComponent.constant;
			light.linear        = lightComponent.linear;
			light.quadratic     = lightComponent.quadratic;
			light.cutoff        = lightComponent.cutoff;
			light.outerCutoff   = lightComponent.outerCutoff;
			light.radius        = lightComponent.radius;
			light.shadowEnabled = lightComponent.shadowEnabled;
		}

		sceneBuffer.lightCount.spotLightCount = spotEntityList.size();
		UniformBufferManager::GetUniformBuffer("LightCount")->SetData(&sceneBuffer.lightCount.spotLightCount, sizeof(int), 2 * sizeof(int));
		UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(spotLights, sceneBuffer.lightCount.spotLightCount * sizeof(SpotLight));

	}
}