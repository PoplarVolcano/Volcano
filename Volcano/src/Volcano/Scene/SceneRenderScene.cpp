#include "volpch.h"
#include "Scene.h"

#include "Volcano/Scene/Components.h"
#include "Volcano/Scene/Entity.h"

#include "Volcano/Renderer/RendererItem/Skybox.h"
#include "Volcano/Renderer/RendererItem/Quad.h"
#include "Volcano/Renderer/RendererItem/Plane.h"
#include "Volcano/Renderer/RendererItem/Cube.h"
#include "Volcano/Renderer/RendererItem/Sphere.h"
#include "Volcano/Renderer/RendererItem/Cylinder.h"
#include "Volcano/Renderer/RendererItem/Capsule.h"
#include "Volcano/Renderer/RendererItem/Cone.h"

#include "Volcano/Renderer/InstanceData.h"
#include "Volcano/Renderer/LightRenderer.h"
#include "Volcano/Renderer/UniformBuffer.h"
#include "Volcano/Renderer/Renderer.h"

#include "Volcano/Renderer/SceneRenderer.h"
#include <glad/glad.h>

#include "Volcano/Scene/SceneBuffer.h"

namespace Volcano
{
	namespace Utils
	{
		static void AddInstance(MeshType meshType, InstanceDataTotal instanceDataTotal)
		{

			switch (meshType)
			{
			case MeshType::None:
				break;
			case MeshType::Quad:
			{
				auto quadMesh = Mesh::GetMeshLibrary()->Get<Quad>(MeshType::Quad)->mesh;
				quadMesh->AddInstance(instanceDataTotal);
				break;
			}
			case MeshType::Circle:
				//Renderer2D::DrawCircle(instanceDataTotal.instanceData.transform, instanceDataTotal.instanceData.color, 1.0f, 0.2f, instanceDataTotal.instanceData.entityID);
				break;
			case MeshType::Line:
				break;
			case MeshType::Plane:
			{
				auto planeMesh = Mesh::GetMeshLibrary()->Get<Plane>(MeshType::Plane)->mesh;
				planeMesh->AddInstance(instanceDataTotal);
				break;
			}
			case MeshType::Cube:
			{
				auto cubeMesh = Mesh::GetMeshLibrary()->Get<Cube>(MeshType::Cube)->mesh;
				cubeMesh->AddInstance(instanceDataTotal);
				break;
			}
			case MeshType::Sphere:
			{
				auto sphereMesh = Mesh::GetMeshLibrary()->Get<Sphere>(MeshType::Sphere)->mesh;
				sphereMesh->AddInstance(instanceDataTotal);
				break;
			}
			case MeshType::Cylinder:
			{
				auto cylinderMesh = Mesh::GetMeshLibrary()->Get<Cylinder>(MeshType::Cylinder)->mesh;
				cylinderMesh->AddInstance(instanceDataTotal);
				break;
			}
			case MeshType::Capsule:
			{
				auto capsuleMesh = Mesh::GetMeshLibrary()->Get<Capsule>(MeshType::Capsule)->mesh;
				capsuleMesh->AddInstance(instanceDataTotal);
				break;
			}
			case MeshType::Cone:
			{
				auto coneMesh = Mesh::GetMeshLibrary()->Get<Cone>(MeshType::Cone)->mesh;
				coneMesh->AddInstance(instanceDataTotal);
				break;
			}
			case MeshType::Model:
				break;
			default:
				VOL_TRACE("AddInstance: 错误MeshType");
				break;
			}

		}

	}

	void Scene::RenderScene()
	{
		auto& sceneBuffer = SceneBuffer::GetInstance();
		if (m_RenderType == RenderType::SKYBOX)
		{
			auto& entity = sceneBuffer.skyboxEntity;
			if (entity != nullptr)
			{
				SkyboxComponent& skyboxComp = entity->GetComponent<SkyboxComponent>();

				if (sceneBuffer.hdrEntity != nullptr)
				{
					HDRComponent& hdrComp = sceneBuffer.hdrEntity->GetComponent<HDRComponent>();
					hdrComp.envCubeMap->Bind();
					//hdrComp.prefilterMap->Bind();
				}
				else if (skyboxComp.textureType == 0)
				{
					skyboxComp.textureCubeMap->Bind();
				}
				else
				{
					skyboxComp.textureCubeSixSided->Bind();
				}

				//glBindTextureUnit(0, SceneRenderer::GetPointLightDepthMapFramebuffer()->GetDepthAttachmentRendererID());

				Ref<Skybox> skybox = Mesh::GetMeshLibrary()->Get<Skybox>(MeshType::Skybox)->mesh;
				skybox->DrawSkybox();
			}
			return;
		}

		if (m_RenderType == RenderType::ENTITYID)
		{
			for (Ref<Entity>& entity : sceneBuffer.forwardShadingEntity)
			{
				auto& transformComp = entity->GetComponent<TransformComponent>();
				auto& meshComp = entity->GetComponent<MeshComponent>();
				auto& meshRendererComp = entity->GetComponent<MeshRendererComponent>();

				glm::mat4 worldTransform = entity->GetWorldTransform();
				glm::mat4 normalTransform = transpose(inverse(glm::mat3(worldTransform)));

				InstanceDataTotal instanceDataTotal;

				instanceDataTotal.instanceData = { worldTransform };

				instanceDataTotal.material = {
					normalTransform,
					meshRendererComp.color,
					meshRendererComp.uvRect,
					meshRendererComp.parallaxScale,
					meshRendererComp.tilingFactor,
					m_MaterialLibrary.GetIndex(meshRendererComp.materialLibraryKey)
				};

				if (meshRendererComp.HasFlag(meshRendererComp.flags, MeshRendererComponent::MeshRendererFlags::Explosion))
					instanceDataTotal.explosion.offset = meshRendererComp.explosionOffset;

				instanceDataTotal.entityID.entityID = (int)entity->GetEntityHandle();

				Utils::AddInstance(meshComp.meshType, instanceDataTotal);
			}

			for (Ref<Entity>& entity : sceneBuffer.GBufferEntityList)
			{
				auto& transformComp = entity->GetComponent<TransformComponent>();
				auto& meshComp = entity->GetComponent<MeshComponent>();
				auto& meshRendererComp = entity->GetComponent<MeshRendererComponent>();

				glm::mat4 worldTransform = entity->GetWorldTransform();
				glm::mat4 normalTransform = transpose(inverse(glm::mat3(worldTransform)));

				InstanceDataTotal instanceDataTotal;

				instanceDataTotal.instanceData = { worldTransform };

				instanceDataTotal.material = {
					normalTransform,
					meshRendererComp.color,
					meshRendererComp.uvRect,
					meshRendererComp.parallaxScale,
					meshRendererComp.tilingFactor,
					m_MaterialLibrary.GetIndex(meshRendererComp.materialLibraryKey)
				};

				instanceDataTotal.entityID.entityID = (int)entity->GetEntityHandle();

				Utils::AddInstance(meshComp.meshType, instanceDataTotal);
			}

			for (auto& entity : sceneBuffer.particleSystemEntityList)
			{
				auto& particleSystemComponent = entity->GetComponent<ParticleSystemComponent>();
				particleSystemComponent.particleSystem->Render();
			}

			Mesh::EndScene();

		}

		if (m_RenderType == RenderType::LIGHTING_MODE)
		{
			Mesh::BeginScene();
			{
				for (Ref<Entity>& entity : sceneBuffer.GBufferEntityList)
				{
					auto& transformComp = entity->GetComponent<TransformComponent>();
					auto& meshComp = entity->GetComponent<MeshComponent>();
					auto& meshRendererComp = entity->GetComponent<MeshRendererComponent>();

					glm::mat4 worldTransform = entity->GetWorldTransform();
					glm::mat4 normalTransform = transpose(inverse(glm::mat3(worldTransform)));

					InstanceDataTotal instanceDataTotal;

					instanceDataTotal.instanceData = { worldTransform };

					instanceDataTotal.material = {
						normalTransform,
						meshRendererComp.color,
						meshRendererComp.uvRect,
						meshRendererComp.parallaxScale,
						meshRendererComp.tilingFactor,
						m_MaterialLibrary.GetIndex(meshRendererComp.materialLibraryKey)
					};

					auto* materialPtr = m_MaterialLibrary.GetMaterial(meshRendererComp.materialLibraryKey);
					if (materialPtr != nullptr)
						instanceDataTotal.lightingMode.lightingMode = (int)materialPtr->lightingMode;

					Utils::AddInstance(meshComp.meshType, instanceDataTotal);
				}

				for (auto& entity : sceneBuffer.particleSystemEntityList)
				{
					auto& particleSystemComponent = entity->GetComponent<ParticleSystemComponent>();
					particleSystemComponent.particleSystem->Render();
				}

				Mesh::EndScene();
			}
		}

		if (m_RenderType == RenderType::SHADOW)
		{
			Mesh::BeginScene();
			{
				for (Ref<Entity>& entity : sceneBuffer.GBufferEntityList)
				{
					auto& transformComp = entity->GetComponent<TransformComponent>();
					auto& meshComp = entity->GetComponent<MeshComponent>();
					auto& meshRendererComp = entity->GetComponent<MeshRendererComponent>();

					glm::mat4 worldTransform = entity->GetWorldTransform();

					InstanceDataTotal instanceDataTotal;

					instanceDataTotal.instanceData = { worldTransform };

					Utils::AddInstance(meshComp.meshType, instanceDataTotal);

				}

				Mesh::EndScene();
			}

		}

		if (m_RenderType == RenderType::G_BUFFER)
		{
			Mesh::BeginScene();
			{
				for (Ref<Entity>& entity : sceneBuffer.normalEntityList)
				{
					auto& transformComp = entity->GetComponent<TransformComponent>();
					auto& meshComp = entity->GetComponent<MeshComponent>();
					auto& meshRendererComp = entity->GetComponent<MeshRendererComponent>();

					glm::mat4 worldTransform = entity->GetWorldTransform();
					glm::mat4 normalTransform = transpose(inverse(glm::mat3(worldTransform)));

					InstanceDataTotal instanceDataTotal;

					instanceDataTotal.instanceData = { worldTransform };

					instanceDataTotal.material = {
						normalTransform,
						meshRendererComp.color,
						meshRendererComp.uvRect,
						meshRendererComp.parallaxScale,
						meshRendererComp.tilingFactor,
						m_MaterialLibrary.GetIndex(meshRendererComp.materialLibraryKey),
					};

					if (meshRendererComp.HasFlag(meshRendererComp.flags, MeshRendererComponent::MeshRendererFlags::Explosion))
						instanceDataTotal.explosion.offset = meshRendererComp.explosionOffset;

					Utils::AddInstance(meshComp.meshType, instanceDataTotal);

				}

				Mesh::EndScene();
			}

			// 注：两段BeginScene完全一样，只是开关Stencil写入的区别
			Mesh::BeginScene();
			{
				for (Ref<Entity>& entity : sceneBuffer.outlineEntityList)
				{
					auto& transformComp = entity->GetComponent<TransformComponent>();
					auto& meshComp = entity->GetComponent<MeshComponent>();
					auto& meshRendererComp = entity->GetComponent<MeshRendererComponent>();

					glm::mat4 worldTransform = entity->GetWorldTransform();
					glm::mat4 normalTransform = transpose(inverse(glm::mat3(worldTransform)));

					InstanceDataTotal instanceDataTotal;

					instanceDataTotal.instanceData = { worldTransform };

					instanceDataTotal.material = {
						normalTransform,
						meshRendererComp.color,
						meshRendererComp.uvRect,
						meshRendererComp.parallaxScale,
						meshRendererComp.tilingFactor,
						m_MaterialLibrary.GetIndex(meshRendererComp.materialLibraryKey),
					};

					if (meshRendererComp.HasFlag(meshRendererComp.flags, MeshRendererComponent::MeshRendererFlags::Explosion))
						instanceDataTotal.explosion.offset = meshRendererComp.explosionOffset;

					Utils::AddInstance(meshComp.meshType, instanceDataTotal);

				}

				RendererAPI::SetStencilMask(0xFF);
				Mesh::EndScene();
				RendererAPI::SetStencilMask(0x00);
			}
		}

		if (m_RenderType == RenderType::POINT_LIGHT)
		{
			Mesh::BeginScene();
			{
				auto& entityList = sceneBuffer.lightVolumePointLightEntityList;
				sceneBuffer.linearAllocator.Reset();
				PointLight* lights = (PointLight*)sceneBuffer.linearAllocator.Allocate(entityList.size() * sizeof(PointLight));
				for (int i = 0; i != entityList.size(); i++)
				{
					auto& entity = entityList[i];
					glm::mat4 worldTransform = entity->GetWorldTransform();
					auto& lightComponent = entity->GetComponent<LightComponent>();

					auto& light = lights[i];
					light.position      = glm::vec3(worldTransform[3]);
					light.ambient       = lightComponent.ambient;
					light.diffuse       = lightComponent.diffuse;
					light.specular      = lightComponent.specular;
					light.constant      = lightComponent.constant;
					light.linear        = lightComponent.linear;
					light.quadratic     = lightComponent.quadratic;
					light.radius        = lightComponent.radius;
					light.shadowEnabled = lightComponent.shadowEnabled;

					InstanceDataTotal instanceDataTotal;

					// 体积光的世界坐标转换
					glm::mat4 translation = glm::translate(glm::mat4(1.0f), light.position);
					glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(light.radius));

					instanceDataTotal.instanceData = { translation * scale };

					auto& sphereMesh = Mesh::GetMeshLibrary()->Get<Sphere>(MeshType::Sphere)->mesh;
					sphereMesh->AddInstance(instanceDataTotal);

				}

				sceneBuffer.lightCount.pointLightCount = entityList.size();
				UniformBufferManager::GetUniformBuffer("LightCount")->SetData(&sceneBuffer.lightCount.pointLightCount, sizeof(int), sizeof(int));
				UniformBufferManager::GetUniformBuffer("PointLight")->SetData(lights, sceneBuffer.lightCount.pointLightCount * sizeof(PointLight));
				
				Mesh::EndScene();
			}
		}

		if (m_RenderType == RenderType::SPOT_LIGHT)
		{
			Mesh::BeginScene();
			{
				auto& entityList = sceneBuffer.lightVolumeSpotLightEntityList;
				sceneBuffer.linearAllocator.Reset();
				SpotLight* lights = (SpotLight*)sceneBuffer.linearAllocator.Allocate(entityList.size() * sizeof(SpotLight));
				for (int i = 0; i != entityList.size(); i++)
				{
					auto& entity = entityList[i];
					glm::mat4 worldTransform = entity->GetWorldTransform();
					auto& lightComponent = entity->GetComponent<LightComponent>();

					auto& light = lights[i];
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

					InstanceDataTotal instanceDataTotal;

					float length = light.radius;
					// 底面半径（angle为半角）
					float radius = tan(glm::radians(lightComponent.outerCutoffAngle)) * length;
					glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(radius, radius, length));

					glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
					if (glm::abs(glm::dot(light.direction, up)) > 0.999f)
					{
						up = glm::vec3(1.0f, 0.0f, 0.0f);
					}
					glm::mat4 rotation = glm::toMat4(glm::quatLookAt(light.direction, up));

					// 将锥尖 (0,0,1) 移到 light.position：
					// 锥尖局部坐标为 (0,0,1)，缩放后为 (0,0,length)
					// 要将锥尖移动到原点，需要让锥尖坐标沿光源方向移动length，即light.direction * length
					// 平移量 = light.position + light.direction * length
					glm::mat4 translation = glm::translate(glm::mat4(1.0f), light.position + light.direction * length);

					instanceDataTotal.instanceData = { translation * rotation * scale };

					auto& coneMesh = Mesh::GetMeshLibrary()->Get<Cone>(MeshType::Cone)->mesh;
					coneMesh->AddInstance(instanceDataTotal);
				}
				sceneBuffer.lightCount.spotLightCount = entityList.size();
				UniformBufferManager::GetUniformBuffer("LightCount")->SetData(&sceneBuffer.lightCount.spotLightCount, sizeof(int), 2 * sizeof(int));
				UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(lights, sceneBuffer.lightCount.spotLightCount * sizeof(SpotLight));

				Mesh::EndScene();
			}
		}

		if (m_RenderType == RenderType::FORWARD_SHADING)
		{
			Mesh::BeginScene();
			{
				for (Ref<Entity>& entity : sceneBuffer.forwardShadingEntity)
				{
					auto& transformComp = entity->GetComponent<TransformComponent>();
					auto& meshComp = entity->GetComponent<MeshComponent>();
					auto& meshRendererComp = entity->GetComponent<MeshRendererComponent>();

					glm::mat4 worldTransform = entity->GetWorldTransform();
					glm::mat4 normalTransform = transpose(inverse(glm::mat3(worldTransform)));

					InstanceDataTotal instanceDataTotal;

					instanceDataTotal.instanceData = { worldTransform };

					instanceDataTotal.material = {
						normalTransform,
						meshRendererComp.color,
						meshRendererComp.uvRect,
						meshRendererComp.parallaxScale,
						meshRendererComp.tilingFactor,
						m_MaterialLibrary.GetIndex(meshRendererComp.materialLibraryKey),
					};

					if (meshRendererComp.HasFlag(meshRendererComp.flags, MeshRendererComponent::MeshRendererFlags::Explosion))
						instanceDataTotal.explosion.offset = meshRendererComp.explosionOffset;

					Utils::AddInstance(meshComp.meshType, instanceDataTotal);

				}

				Mesh::EndScene();
			}

		}

		if (m_RenderType == RenderType::OUTLINE)
		{
			Mesh::BeginScene();
			{
				for (Ref<Entity>& entity : sceneBuffer.outlineEntityList)
				{
					auto& transformComp = entity->GetComponent<TransformComponent>();
					auto& meshComp = entity->GetComponent<MeshComponent>();
					auto& meshRendererComp = entity->GetComponent<MeshRendererComponent>();

					glm::mat4 worldTransform = entity->GetWorldTransform();
					glm::mat4 normalTransform = transpose(inverse(glm::mat3(worldTransform)));

					// 计算放大后的模型矩阵
					glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(meshRendererComp.outlineScale));
					worldTransform = worldTransform * scaleMat;

					InstanceDataTotal instanceDataTotal;

					instanceDataTotal.instanceData = { worldTransform };

					instanceDataTotal.outline.color = meshRendererComp.outlineColor;

					Utils::AddInstance(meshComp.meshType, instanceDataTotal);
				}

				Mesh::EndScene();
			}

		}

		if (m_RenderType == RenderType::NORMAL_VISUALIZATION)
		{
			Mesh::BeginScene();
			{
				for (Ref<Entity>& entity : sceneBuffer.GBufferEntityList)
				{
					auto& transformComp = entity->GetComponent<TransformComponent>();
					auto& meshComp = entity->GetComponent<MeshComponent>();
					auto& meshRendererComp = entity->GetComponent<MeshRendererComponent>();

					if (!meshRendererComp.HasFlag(meshRendererComp.flags, MeshRendererComponent::MeshRendererFlags::NormalVisualization))
					{
						return;
					}

					glm::mat4 worldTransform = entity->GetWorldTransform();
					glm::mat4 normalTransform = transpose(inverse(glm::mat3(worldTransform)));

					InstanceDataTotal instanceDataTotal;

					instanceDataTotal.instanceData = { worldTransform };

					instanceDataTotal.material = {
						normalTransform,
						meshRendererComp.color,
						meshRendererComp.uvRect,
						meshRendererComp.parallaxScale,
						meshRendererComp.tilingFactor,
						m_MaterialLibrary.GetIndex(meshRendererComp.materialLibraryKey),
					};

					instanceDataTotal.normalVisualization = {
					   meshRendererComp.normalVisualizationColor,
					   meshRendererComp.normalVisualizationLength,
					   meshRendererComp.normalVisualizationIndex1,
					   meshRendererComp.normalVisualizationIndex2
					};

					Utils::AddInstance(meshComp.meshType, instanceDataTotal);
				}

				Mesh::EndScene();
			}
		}

		if (m_RenderType == RenderType::PARTICLE)
		{
			Mesh::BeginScene();
			{
				for (auto& entity : sceneBuffer.particleSystemEntityList)
				{
					auto& particleSystemComponent = entity->GetComponent<ParticleSystemComponent>();
					particleSystemComponent.particleSystem->Render();
				}

				Mesh::EndScene();
			}
		}
	}

}