#include "volpch.h"
#include "Scene.h"

#include "Volcano/Renderer/UniformBuffer.h"

#include "Volcano/Scene/Entity.h"
#include "Volcano/Scene/SceneBuffer.h"

#include <glm/gtx/matrix_decompose.hpp>

namespace Volcano
{

	void Scene::UpdateScene(Camera& camera, const glm::vec3& cameraWorldPosition, const glm::quat& cameraWorldRotation)
	{

		// view 是视图矩阵，作用是把世界坐标变换到摄像机坐标（也叫视图空间、眼空间）
		// 是 无缩放的摄像机世界变换 的逆矩阵
		glm::mat4 view = glm::inverse(
			glm::translate(glm::mat4(1.0f), cameraWorldPosition) *
			glm::mat4_cast(cameraWorldRotation)
		);

		// 更新CameraData的Uniform
		CameraData cameraData = {
			view,
			camera.GetProjection(),
			cameraWorldPosition,
			0.0f,
			camera.GetNearClip(),
			camera.GetFarClip()
		};
		UniformBufferManager::GetUniformBuffer("CameraData")->SetData(&cameraData, sizeof(CameraData));


		// 注：实例化Instancing和模板Stencil有矛盾，Stencil不能选定特定物体启动模板写入，
		// 所以需要把启动模板的物体独立出来进行二次渲染，

		auto& sceneBuffer = SceneBuffer::GetInstance();
		sceneBuffer.Clear();

		// 重置光源数量
		UniformBufferManager::GetUniformBuffer("LightCount")->SetData(&sceneBuffer.lightCount, 3 * sizeof(int));
		
		// 遍历所有已激活的实体，将对应的实体填充进sceneBuffer中对应的EntityList，并更新entityWorldTransformMap
		TraverseEntity(m_EntityList, [&](Ref<Entity> entity) ->bool{

			sceneBuffer.entityWorldTransformMap[entity.get()] = entity->GetWorldTransform();
			sceneBuffer.entityWorldRotationMap[entity.get()] = entity->GetWorldRotation();

			if (entity->HasComponent<CameraComponent>())
			{
				auto& cameraComponent = entity->GetComponent<CameraComponent>();
				if (cameraComponent.enabled && cameraComponent.primary)
				{
					sceneBuffer.cameraEntity = entity;
				}
			}

			if (entity->HasComponent<MeshComponent>() && entity->HasComponent<MeshRendererComponent>())
			{
				auto& meshComponent = entity->GetComponent<MeshComponent>();
				auto& meshRendererComponent = entity->GetComponent<MeshRendererComponent>();
				if (meshComponent.enabled && meshComponent.meshType != MeshType::None && meshRendererComponent.enabled)
				{
					if (entity->HasComponent<LightComponent>() && entity->GetComponent<LightComponent>().enabled)
					{
						sceneBuffer.forwardShadingEntity.push_back(entity);
					}
					else
					{
						sceneBuffer.GBufferEntityList.push_back(entity);

						bool outlineEnabled = meshRendererComponent.HasFlag(meshRendererComponent.flags, MeshRendererComponent::MeshRendererFlags::Outline);
						if (outlineEnabled)
						{
							sceneBuffer.outlineEntityList.push_back(entity);
						}
						else
						{
							sceneBuffer.normalEntityList.push_back(entity);
						}
					}
				}
			}

			if (entity->HasComponent<LightComponent>())
			{
				auto& lightComponent = entity->GetComponent<LightComponent>();
				if (lightComponent.enabled)
				{
					glm::mat4 worldTransform = entity->GetWorldTransform();

					switch (lightComponent.type)
					{
					case LightComponent::LightType::DirectionalLight:
					{
						DirectionalLight light;
						light.direction     = glm::rotate(glm::quat_cast(worldTransform), glm::vec3(0.0f, 0.0f, -1.0f));
						light.ambient       = lightComponent.ambient;
						light.diffuse       = lightComponent.diffuse;
						light.specular      = lightComponent.specular;
						light.shadowEnabled = lightComponent.shadowEnabled;

						sceneBuffer.lightCount.directionalLightCount = 1;
						UniformBufferManager::GetUniformBuffer("LightCount")->SetData(&sceneBuffer.lightCount.directionalLightCount, sizeof(int));
						UniformBufferManager::GetUniformBuffer("DirectionalLight")->SetData(&light, sizeof(DirectionalLight));
						
						if (lightComponent.shadowEnabled)
						{
							float near_plane = 0.1f, far_plane = 200.0f;
							glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
							glm::vec3 direction = glm::rotate(glm::quat_cast(worldTransform), glm::vec3(0.0f, 0.0f, -1.0f));
							glm::vec3 center = glm::vec3(0.0f);
							glm::vec3 lightPosition = center - direction * 100.0f; // 沿方向后退一定距离
							glm::mat4 lightView = glm::lookAt(lightPosition, center, glm::vec3(0.0f, 1.0f, 0.0f));
							glm::mat4 lightSpaceMatrix = lightProjection * lightView;

							UniformBufferManager::GetUniformBuffer("DirectionalLightShadowData")->SetData(&lightSpaceMatrix, sizeof(glm::mat4));
							sceneBuffer.lightCount.directionalLightShadowCount = 1;
						}

						break;
					}
					case LightComponent::LightType::PointLight:
					{
						glm::vec3 cameraToLight = glm::vec3(worldTransform[3]) - cameraWorldPosition;
						if (glm::dot(cameraToLight, cameraToLight) < lightComponent.radius * lightComponent.radius)
						{
							// 摄像头在光体积球体内，退化成全屏光照
							sceneBuffer.pointLightEntityList.push_back(entity);
						}
						else
						{
							sceneBuffer.lightVolumePointLightEntityList.push_back(entity);
						}

						if (lightComponent.shadowEnabled)
						{
							const uint32_t SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
							const float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
							float nearClip = 0.1f;
							float farClip = lightComponent.radius;
							glm::mat4 shadowProj = glm::perspective(89.535f, aspect, nearClip, farClip);// fov理应是90，可能是float的计算误差会导致立方体贴图视野错位

							PointLightShadowData lightShadowData;
							lightShadowData.position = glm::vec3(worldTransform[3]);
							lightShadowData.radius = lightComponent.radius;
							lightShadowData.lightSpaceMatrices[0] = shadowProj * glm::lookAt(lightShadowData.position, lightShadowData.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
							lightShadowData.lightSpaceMatrices[1] = shadowProj * glm::lookAt(lightShadowData.position, lightShadowData.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
							lightShadowData.lightSpaceMatrices[2] = shadowProj * glm::lookAt(lightShadowData.position, lightShadowData.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
							lightShadowData.lightSpaceMatrices[3] = shadowProj * glm::lookAt(lightShadowData.position, lightShadowData.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
							lightShadowData.lightSpaceMatrices[4] = shadowProj * glm::lookAt(lightShadowData.position, lightShadowData.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
							lightShadowData.lightSpaceMatrices[5] = shadowProj * glm::lookAt(lightShadowData.position, lightShadowData.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
							// 光源视图矩阵，将像素转换到光源视图空间后用于比较深度值。
							// 在阴影渲染中只有对应光源需要通过该矩阵渲染阴影，所以需要给光源加入是否渲染阴影的标志shadowEnable。
							// 注：对于其他设置渲染阴影却因为只有一个阴影位，阴影视图矩阵被覆盖的光源，会因为是否渲染阴影的标志shadowEnabled而错误渲染阴影。
							sceneBuffer.lightCount.pointLightShadowCount = 1;
							UniformBufferManager::GetUniformBuffer("PointLightShadowData")->SetData(&lightShadowData, sizeof(PointLightShadowData));
						}

						break;
					}
					case LightComponent::LightType::SpotLight:
					{
						glm::vec3 lightToCamera = cameraWorldPosition - glm::vec3(worldTransform[3]);
						float distance = glm::dot(lightToCamera, lightToCamera);
						if (distance > lightComponent.radius * lightComponent.radius)
						{
							sceneBuffer.lightVolumeSpotLightEntityList.push_back(entity);
						}
						else
						{
							glm::vec3 direction = glm::rotate(glm::quat_cast(worldTransform), glm::vec3(0.0f, 0.0f, -1.0f));
							glm::vec3 lightToCameraDirection = glm::normalize(lightToCamera);
							float cosAngle = glm::dot(lightToCameraDirection, direction);

							// cos递减函数，角越大，cos越小
							if (cosAngle < lightComponent.outerCutoff)
							{
								sceneBuffer.lightVolumeSpotLightEntityList.push_back(entity);
							}
							else
							{
								// 摄像机在光体积锥体内，退化成全屏光照
								sceneBuffer.spotLightEntityList.push_back(entity);
							}
						}

						if (lightComponent.shadowEnabled)
						{
							float nearClip = 0.1f, farClip = lightComponent.radius;
							float fov = 120.0f;
							glm::mat4 lightProjection = glm::perspective(glm::radians(fov), 1.0f, nearClip, farClip);

							glm::vec3 lightPosition = entity->GetWorldTransform()[3];
							glm::quat lightRotation = entity->GetWorldRotation();
							glm::mat4 lightView = glm::inverse(
								glm::translate(glm::mat4(1.0f), lightPosition) *
								glm::toMat4(lightRotation)
							);

							glm::mat4 lightSpaceMatrix = lightProjection * lightView;
							sceneBuffer.lightCount.spotLightShadowCount = 1;
							UniformBufferManager::GetUniformBuffer("SpotLightShadowData")->SetData(&lightSpaceMatrix, sizeof(glm::mat4));
						}
						break;
					}
					default:
						break;
					}

				}
			}

			if (entity->HasComponent<ParticleSystemComponent>())
			{
				sceneBuffer.particleSystemEntityList.push_back(entity);
				auto& particleSystemComponent = entity->GetComponent<ParticleSystemComponent>();
				if (particleSystemComponent.enabled)
				{
					particleSystemComponent.particleSystem->Update(cameraWorldPosition, glm::eulerAngles(cameraWorldRotation));
				}
			}

			if (entity->HasComponent<SkyboxComponent>())
			{
				auto& skyboxComponent = entity->GetComponent<SkyboxComponent>();
				if (skyboxComponent.enabled && skyboxComponent.primary)
				{
					sceneBuffer.skyboxEntity = entity;
				}
			}

			if (entity->HasComponent<HDRComponent>())
			{
				auto& hdrComponent = entity->GetComponent<HDRComponent>();
				if (hdrComponent.enabled && hdrComponent.primary && !hdrComponent.equirectangularMapKey.key.empty())
				{
					sceneBuffer.hdrEntity = entity;
				}
			}

			return true;
			});

	}

}