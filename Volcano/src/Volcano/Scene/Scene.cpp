#include "volpch.h"
#include "Scene.h"

#include "Components.h"
#include "ScriptableEntity.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Renderer/Renderer.h"
#include "Volcano/Renderer/RendererAPI.h"
#include "Volcano/Renderer/Renderer2D.h"
#include "Volcano/Renderer/RendererItem/Skybox.h"
#include "Volcano/Renderer/RendererItem/Model.h"
#include "Volcano/Renderer/RendererItem/Animator.h"
#include "Volcano/Physics/Physics2D.h"
#include "Volcano/Renderer/Light.h"

#include "glm/glm.hpp"
#include "Entity.h"

// Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"
#include <Volcano/Renderer/Light.h>
#include <glm/gtc/type_ptr.hpp>

namespace Volcano {

	Scene::Scene()
	{
		InitializeUniform();
	}

	Scene::~Scene()
	{
		delete m_PhysicsWorld;
	}

	void Scene::InitializeUniform()
	{
	}

	// ��Դע�����ʵ�帴�Ƶ�Ŀ��ע���Map��UUID��Ϊ��ǻ�ȡ��ʵ��
	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, Ref<Entity>>& entityIDMap)
	{ 
		// ���lambda��ݹ����
		// [&]��ʾ���ô��ݷ�ʽ��׽���и�������ı���������this��
		// ��ʽ���ò���dst��src����"�⿪��Component������"�������Component��ָ����ĵ������
		([&]()
		{
			auto view = src.view<Component>();
			for (auto srcEntity : view)
			{
				entt::entity dstEntity = entityIDMap.at(src.get<IDComponent>(srcEntity).ID)->GetEntityHandle();
				auto& srcComponent = src.get<Component>(srcEntity);
				dst.emplace_or_replace<Component>(dstEntity, srcComponent);
			}

		}(), ...);// ��������Ӧ���ǽ�Component��
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, Ref<Entity>>& entityIDMap)
	{
		CopyComponent<Component...>(dst, src, entityIDMap);
	}

	// ���Դʵ�������Ѷ�Ӧ������Ƶ�Ŀ��ʵ��
	template<typename... Component>
	static void CopyComponentIfExists(Ref<Entity> dst, Ref<Entity> src)
	{
		([&]()
		{
			if (src->HasComponent<Component>())
				dst->AddOrReplaceComponent<Component>(src->GetComponent<Component>());
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Ref<Entity> dst, Ref<Entity> src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	void CopyEntityChildren(Ref<Entity>& srcEntity, Ref<Entity>& dstEntity, std::unordered_map<UUID, Ref<Entity>>& entityIDMap, std::unordered_map<entt::entity, Ref<Entity>>& entityEnttMap)
	{
		if (!srcEntity->GetEntityChildren().empty())
		{
			auto& dstEntityChildren = dstEntity->GetEntityChildren();
			for (auto& [name, entityChild] : srcEntity->GetEntityChildren())
			{
				Ref<Entity> entity = Entity::Create(*dstEntity->GetScene(), entityChild->GetUUID(), entityChild->GetName());
				entity->SetEntityParent(srcEntity.get());
				entityIDMap[entity->GetUUID()] = entity;
				entityEnttMap[entity->GetEntityHandle()] = entity;
				dstEntityChildren[entityChild->GetName()] = entity;
				CopyEntityChildren(entityChild, entity, entityIDMap, entityEnttMap);
			}
		}
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		// �����³�����Ϊ�³��������;ɳ���ͬ����uuid��ʵ�壬����map���루��ʵ���uuid��Ӧ��ʵ�壩�Ĺ�ϵ
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_ViewportWidth  = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		auto& entityIDMap   = newScene->GetEntityIDMap();
		auto& entityEnttMap = newScene->GetEntityEnttMap();
		auto& entityNameMap = newScene->GetEntityNameMap();

		for (auto& [name, entityChild] : other->GetEntityNameMap())
		{
			Ref<Entity> entity = Entity::Create(*newScene.get(), entityChild->GetUUID(), entityChild->GetName());
			entityIDMap[entity->GetUUID()] = entity;
			entityEnttMap[entity->GetEntityHandle()] = entity;
			entityNameMap[entity->GetName()] = entity;
			CopyEntityChildren(entityChild, entity, entityIDMap, entityEnttMap);
		}

		// ��ȡ��ʵ������������Ȼ����API�����ƾ�ʵ��������������ʵ�壬�������������������������ֵ
		// Copy components (except IDComponent and TagComponent)
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, entityIDMap);

		// MeshComponent��Meshָ�븴�ƺ��ǰ󶨾�mesh����mesh�󶨾�Entity����Ҫ��������һ������������mesh������Entity
		auto view = dstSceneRegistry.view<MeshComponent>();
		for (auto entity : view)
		{
			auto& mc = dstSceneRegistry.get<MeshComponent>(entity);
			auto modelPath = mc.modelPath;
			auto vertexBone = mc.vertexBone;
			mc.SetMesh(mc.meshType, entityEnttMap[entity].get(), mc.mesh);
			mc.modelPath = modelPath;
			mc.vertexBone = vertexBone;
		}

		return newScene;
	}

	Ref<Entity> Scene::CreateEntity(const std::string& name, Ref<Entity> entity)
	{
		return CreateEntityWithUUID(UUID(), name, entity);
	}

	std::string Scene::NewName(std::map<std::string, Ref<Entity>> entityNameMap, std::string name)
	{
		std::string newName = name;
		if (entityNameMap.find(newName) != entityNameMap.end())
		{
			int i = 0;
			do
			{
				newName = name + "(" + std::to_string(i) + ")";
				i++;
			} while (entityNameMap.find(newName) != entityNameMap.end());
		}

		return newName;
	}

	Ref<Entity> Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, Ref<Entity> entity)
	{
		Ref<Entity> entityTemp;

		if (entity != nullptr)
		{
			entityTemp = entity->AddEntityChild(uuid, name);
			m_EntityIDMap[entityTemp->GetUUID()] = entityTemp;
			m_EntityEnttMap[entityTemp->GetEntityHandle()] = entityTemp;
			return entityTemp;
		}

		std::string newName = NewName(m_EntityNameMap, name);

		entityTemp = Entity::Create(*this, uuid, newName);
		m_EntityIDMap[entityTemp->GetUUID()] = entityTemp;
		m_EntityEnttMap[entityTemp->GetEntityHandle()] = entityTemp;
		m_EntityNameMap[entityTemp->GetName()] = entityTemp;

		return entityTemp;
	}


	void Scene::DestroyEntityChild(Ref<Entity> entity)
	{
		auto entityChildren = entity->GetEntityChildren();
		for (auto& [name, entityChild] : entityChildren)
		{
			auto& entityChildrenRef = entity->GetEntityChildren();
			entityChildrenRef.erase(entityChild->GetName());
			DestroyEntityChild(entityChild);
		}
		m_EntityIDMap.erase(entity->GetUUID());
		m_EntityEnttMap.erase(entity->GetEntityHandle());
		entity->GetEntityParent()->GetEntityChildren().erase(entity->GetName());
		m_Registry.destroy(*entity.get());
	}
	void Scene::DestroyEntity(Ref<Entity> entity)
	{
		auto entityChildren = entity->GetEntityChildren();
		for (auto& [name, entityChild] : entityChildren)
		{
			auto& entityChildrenRef = entity->GetEntityChildren();
			entityChildrenRef.erase(entityChild->GetName());
			DestroyEntityChild(entityChild);
		}
		m_EntityIDMap.erase(entity->GetUUID());
		m_EntityEnttMap.erase(entity->GetEntityHandle());
		m_EntityNameMap.erase(entity->GetName());
		m_Registry.destroy(*entity.get());
	}

	void Scene::OnRuntimeStart()
	{
		OnPhysics2DStart();

		// �ű���ʼ��Scripting
		{
			ScriptEngine::OnRuntimeStart(this);
			// Instantiate all script entities
			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				//Entity entity = { e, this };
				ScriptEngine::OnCreateEntity(*m_EntityEnttMap[e].get());
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		OnPhysics2DStop();
		ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnSimulationStart()
	{
		OnPhysics2DStart();
	}

	void Scene::OnSimulationStop()
	{
		OnPhysics2DStop();
	}

	void Scene::Physics(Timestep ts)
	{
		// �ű�Ӱ��pyhsicȻ����Ⱦ����ǰ֡�õ����
		// �����ٶȣ�ʹ�ø��ٵĵ�������������ܣ���׼ȷ�Ի��ܵ�Ӱ�졣ʹ�ø�������ή�����ܵ������ģ������
		// Physics
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

		// Retrieve transform from Box2D
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			//Entity entity = { e, this };
			auto& transform = m_EntityEnttMap[e]->GetComponent<TransformComponent>();
			auto& rb2d = m_EntityEnttMap[e]->GetComponent<Rigidbody2DComponent>();

			// ��ȡ����ģ�����������
			b2Body* body = (b2Body*)rb2d.RuntimeBody;

			// ��������ֵ����ʵ��
			const auto& position = body->GetPosition();
			transform.Translation.x = position.x;
			transform.Translation.y = position.y;
			transform.Rotation.z = body->GetAngle();
		}
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		if (!m_IsPaused || m_StepFrames-- > 0)
		{
			//Update scripts
			{
				// C# Entity OnUpdate
				auto view = m_Registry.view<ScriptComponent>();
				for (auto e : view)
				{
					//Entity entity = { e, this };
					ScriptEngine::OnUpdateEntity(*m_EntityEnttMap[e].get(), ts);
				}

				m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
					{
						if (!nsc.Instance)
						{
							nsc.Instance = nsc.InstantiateScript();
							nsc.Instance->m_Entity = Entity{ entity, this };
							nsc.Instance->OnCreate();
						}
						nsc.Instance->OnUpdate(ts);
					});
			}
			// Script - Physic - Render˳��
			Physics(ts);
		}

		UpdateScene(ts);
	}

	void Scene::OnRenderRuntime(Timestep ts)
	{
		// ��ȡ������ͷ
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		glm::vec3 cameraPosition;
		glm::vec3 cameraDirection;
		{
			Ref<Entity> mainCameraEntity = GetPrimaryCameraEntity();
			if (mainCameraEntity)
			{
				auto& transform = mainCameraEntity->GetComponent<TransformComponent>();
				auto& camera = mainCameraEntity->GetComponent<CameraComponent>();
				mainCamera = &camera.Camera;
				cameraTransform = transform.GetTransform();
				cameraPosition = transform.Translation;
				cameraDirection = glm::rotate(glm::quat(transform.Rotation), glm::vec3(0.0f, 0.0f, -1.0f));
			}
		}

		// ������ͷ��Ⱦ����
		if (mainCamera)
			RenderScene(*mainCamera, cameraTransform, cameraPosition, cameraDirection);
	}

	void Scene::OnUpdateSimulation(Timestep ts, EditorCamera& camera)
	{
		if (!m_IsPaused || m_StepFrames-- > 0)
			Physics(ts);

		UpdateScene(ts);
	}

	void Scene::OnRenderSimulation(Timestep ts, EditorCamera& camera)
	{
		// Render
		RenderScene(camera, camera.GetViewMatrix(), camera.GetPosition(), camera.GetForwardDirection());
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{

		{
			auto view = m_Registry.view<TransformComponent>();
			for (auto entity : view)
			{
				auto transform = view.get<TransformComponent>(entity);
				m_EntityEnttMap[entity]->SetTransform(transform.GetTransform());
			}
		}

		UpdateScene(ts);
	}

	void Scene::OnRenderEditor(Timestep ts, EditorCamera& camera)
	{
		// Render
		RenderScene(camera, camera.GetViewMatrix(), camera.GetPosition(), camera.GetForwardDirection());
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		//Resize our non-FixedAspectRatio camera

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

	void Scene::Step(int frames)
	{
		m_StepFrames = frames;
	}

	Ref<Entity> Scene::DuplicateEntity(Ref<Entity> entity)
	{
		// Copy name because we're going to modify component data structure
		std::string name = entity->GetName();
		Ref<Entity> newEntity = CreateEntity(name);
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}

	Ref<Entity> Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return m_EntityEnttMap[entity];
		}
		return {};
	}
	
	Ref<Entity> Scene::GetDirectionalLightEntity()
	{
		// ��ȡ��Դ
		auto view = m_Registry.view<LightComponent>();
		for (auto entity : view)
		{
			auto light = view.get<LightComponent>(entity);
			if (light.Type == LightComponent::LightType::DirectionalLight)
				return m_EntityEnttMap[entity];
		}
		return {};
	}

	Ref<Entity> Scene::GetPrimarySkyboxEntity()
	{
		auto view = m_Registry.view<SkyboxComponent>();
		for (auto entity : view)
		{
			auto skybox = view.get<SkyboxComponent>(entity);
			if (skybox.Primary)
				return m_EntityEnttMap[entity];
		}
		return {};
	}

	std::vector<Ref<Entity>> Scene::GetPointLightEntities()
	{
		std::vector<Ref<Entity>> pointLights;
		auto view = m_Registry.view<LightComponent>();
		for (auto entity : view)
		{
			auto light = view.get<LightComponent>(entity);
			if (light.Type == LightComponent::LightType::PointLight)
				pointLights.push_back(m_EntityEnttMap[entity]);// entt�ǴӺ���ǰ�������õ���vector�ǵ����
		}
		return pointLights;

	}
	std::vector<Ref<Entity>> Scene::GetSpotLightEntities()
	{
		std::vector<Ref<Entity>> spotLights;
		auto view = m_Registry.view<LightComponent>();
		for (auto entity : view)
		{
			auto light = view.get<LightComponent>(entity);
			if (light.Type == LightComponent::LightType::SpotLight)
				spotLights.push_back(m_EntityEnttMap[entity]);
		}
		return spotLights;
	}

	Ref<Entity> Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const TagComponent& tc = view.get<TagComponent>(entity);
			if (tc.Tag == name)
				return m_EntityEnttMap[entity];
		}
		return {};
	}

	Ref<Entity> Scene::GetEntityByUUID(UUID uuid)
	{
		if (m_EntityIDMap.find(uuid) != m_EntityIDMap.end())
			return m_EntityIDMap[uuid];

		return {};
	}

	void Scene::OnPhysics2DStart()
	{
		// ����һ����������/����
		// �������ٶ�����
		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });

		// Ϊ��ǰ�������о������������ʵ�崴��b2Body
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Ref<Entity> entity = m_EntityEnttMap[e];
			auto& transform = entity->GetComponent<TransformComponent>();
			auto& rb2d = entity->GetComponent<Rigidbody2DComponent>();

			// ���嶨������ָ����̬���ͺͲ���
			b2BodyDef bodyDef;
			bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			// ����z����ת
			bodyDef.angle = transform.Rotation.z;

			// ��b2BodyDef��������
			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			// �Ƿ�̶���ת
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;

			if (entity->HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity->GetComponent<BoxCollider2DComponent>();
				// ������Ӱ�Χ��
				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

				// ����fixture��fixture��������İ�Χ��
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
				// ���������fixture
				body->CreateFixture(&fixtureDef);
			}

			if (entity->HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity->GetComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
				circleShape.m_radius = transform.Scale.x * cc2d.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2d.Density;
				fixtureDef.friction = cc2d.Friction;
				fixtureDef.restitution = cc2d.Restitution;
				fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::OnPhysics2DStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}


	void Scene::UpdateScene(Timestep ts)
	{
		// Update Skybox
		auto skyboxEntity = GetPrimarySkyboxEntity();
		if (skyboxEntity != nullptr)
			Skybox::SetTexture(skyboxEntity->GetComponent<SkyboxComponent>().texture);

		// Update Animator
		{
			auto view = m_Registry.view<AnimatorComponent, AnimationComponent>();
			for (auto entity : view)
			{
				auto animator = view.get<AnimatorComponent>(entity).animator;
				auto animation = view.get<AnimationComponent>(entity).animation;
				if (animation != nullptr)
				{
					animator->SetAnimation(animation.get());
					animator->UpdateAnimation(ts);
				}
			}
		}

		// Update Transform
		{
			auto view = m_Registry.view<TransformComponent>();
			for (auto entity : view)
			{
				auto transform = view.get<TransformComponent>(entity);
				m_EntityEnttMap[entity]->SetTransform(transform.GetTransform());
			}
		}

		// Update Mesh
		{
			auto view = m_Registry.view<TransformComponent, MeshComponent, MeshRendererComponent>();
			std::vector<glm::mat4> finalBoneMatrices;

			for (auto entity : view)
			{
				auto [meshTransform, mesh, renderer] = view.get<TransformComponent, MeshComponent, MeshRendererComponent>(entity);
				Entity* entityNode;
				switch (mesh.meshType)
				{
				case MeshType::None:
					break;
				case MeshType::Cube:
					mesh.mesh->SetVertexBoneDataToDefault();
					for(auto& vertexBone : mesh.vertexBone)
					    mesh.mesh->SetBoneID(vertexBone.vertexIndex1, vertexBone.vertexIndex2, vertexBone.boneIndex, vertexBone.weight);
					mesh.mesh->StartBatch();

					entityNode = m_EntityEnttMap[entity].get();
					do
					{
						if (entityNode->HasComponent<AnimationComponent>() && entityNode->HasComponent<AnimatorComponent>())
						{
							auto animator = entityNode->GetComponent<AnimatorComponent>().animator;
							mesh.mesh->DrawMesh((int)entity, animator->GetFinalBoneMatrices());
							break;
						}
						else
							entityNode = entityNode->GetEntityParent();
					} while (entityNode != nullptr);

					if (entityNode == nullptr)
					{
						mesh.mesh->DrawMesh((int)entity, finalBoneMatrices);
					}
					break;
				case MeshType::Sphere:
					for (auto& vertexBone : mesh.vertexBone)
						mesh.mesh->SetBoneID(vertexBone.vertexIndex1, vertexBone.vertexIndex2, vertexBone.boneIndex, vertexBone.weight);
					mesh.mesh->StartBatch();

					entityNode = m_EntityEnttMap[entity].get();
					do
					{
						if (entityNode->HasComponent<AnimationComponent>() && entityNode->HasComponent<AnimatorComponent>())
						{
							auto animator = entityNode->GetComponent<AnimatorComponent>().animator;
							mesh.mesh->DrawMesh((int)entity, animator->GetFinalBoneMatrices());
							break;
						}
						else
							entityNode = entityNode->GetEntityParent();
					} while (entityNode != nullptr);

					if (entityNode == nullptr)
					{
						mesh.mesh->DrawMesh((int)entity, finalBoneMatrices);
					}
					break;
				case MeshType::Model:
					if (mesh.mesh != nullptr)
					{
						mesh.mesh->StartBatch();
						entityNode = m_EntityEnttMap[entity].get();
						do
						{
							if (entityNode->HasComponent<AnimationComponent>() && entityNode->HasComponent<AnimatorComponent>())
							{
								auto animator = entityNode->GetComponent<AnimatorComponent>().animator;
								mesh.mesh->DrawMesh((int)entity, animator->GetFinalBoneMatrices());
								break;
							}
							else
								entityNode = entityNode->GetEntityParent();
						} while (entityNode != nullptr);

						if (entityNode == nullptr)
						{
							mesh.mesh->DrawMesh((int)entity, finalBoneMatrices);
						}
					}
					break;
				default:
					VOL_TRACE("����MeshType");
					break;
				}
			}
		}
	}

	void RenderBone(const AssimpNodeData* node, glm::mat4 parentTransform, Animation* animation)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		//��animation��m_Bones�����в��ҹ��������ù����Ƿ����ö�����
		Bone* Bone = animation->FindBone(nodeName);

		if (Bone)
		{
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform; // �����ռ�ת�������ڵ�����ռ䣬�ݹ�ֱ��ת�������ڵ�����ռ�

		auto boneInfoMap = animation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			glm::mat4 transform = globalTransformation* offset;
			glm::vec4 p0 = transform * glm::vec4(0.0f,  0.0f, 0.0f, 1.0f);
			glm::vec4 p1 = transform * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			Renderer2D::DrawLine(p0, p1, glm::vec4(1.0f), -1);
		}

		for (int i = 0; i < node->childrenCount; i++)
			RenderBone(&node->children[i], globalTransformation, animation);
	}

	// ����ͷ��Ⱦ����������ͷ������ͷTRS������ͷλ�ã�translation��������ͷ����
	void Scene::RenderScene(Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction)
	{
		UpdateCameraData(camera, transform, position);

		if (m_RenderType == RenderType::SKYBOX)
		{
			auto entity = GetPrimarySkyboxEntity();
			if (entity != nullptr)
			{
				RendererAPI::SetDepthFunc(DepthFunc::LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
				Skybox::BeginScene(camera, transform);
				Skybox::DrawSkybox();
				Skybox::EndScene();
				RendererAPI::SetDepthFunc(DepthFunc::LESS); // set depth function back to default
			}
			return;
		}

		if (m_RenderType == RenderType::NORMAL)
		{

			// Render Bone
			if(m_ShowBone)
			{
				Renderer2D::BeginScene(camera, transform);
				auto view = m_Registry.view<TransformComponent, AnimationComponent>();
				for (auto entity : view)
				{
					auto parentTransform = view.get<TransformComponent>(entity).GetTransform();
					auto animation = view.get<AnimationComponent>(entity).animation;
					if (animation != nullptr)
					{
						auto node = animation->GetRootNode();
						RenderBone(&node, parentTransform, animation.get());
					}
				}
				Renderer2D::EndScene();
			}
			return;

		}

		Renderer2D::BeginScene(camera, transform);

		// Draw sprites
		{
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}
		}

		// Draw circles
		{
			auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
				Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
			}
		}
		Renderer2D::EndScene();


		//Draw Mesh
		{
			auto view = m_Registry.view<TransformComponent, MeshComponent, MeshRendererComponent>();

			for (auto entity : view)
			{
				auto [meshTransform, mesh, renderer] = view.get<TransformComponent, MeshComponent, MeshRendererComponent>(entity);

				switch (mesh.meshType)
				{
				case MeshType::None:
					break;
				case MeshType::Cube:
					mesh.mesh->BeginScene(camera, transform, position);
					mesh.mesh->BindTextures(renderer.Textures);
					mesh.mesh->BindShader(m_RenderType);
					mesh.mesh->EndScene();
					break;
				case MeshType::Sphere:
					mesh.mesh->BeginScene(camera, transform, position);
					mesh.mesh->BindTextures(renderer.Textures);
					mesh.mesh->BindShader(m_RenderType);
					mesh.mesh->EndScene();
					break;
				case MeshType::Model:
					if (mesh.mesh != nullptr)
					{
						mesh.mesh->BeginScene(camera, transform, position);
						mesh.mesh->BindTextures(renderer.Textures);
						mesh.mesh->BindShader(m_RenderType);
						mesh.mesh->EndScene();
					}
					break;
				default:
					VOL_TRACE("����MeshType");
					break;
				}
			}
		}
		

		// draw skybox as last
		

	}


	void Scene::UpdateCameraData(Camera& camera, glm::mat4 transform, glm::vec3 position)
	{
		glm::mat4 viewProjection = camera.GetProjection() * glm::inverse(transform);
		UniformBufferManager::GetUniformBuffer("CameraViewProjection")->SetData(&viewProjection, sizeof(glm::mat4));

		UniformBufferManager::GetUniformBuffer("CameraPosition")->SetData(&position, sizeof(glm::vec3));

	}

	void Scene::UpdateLight(uint32_t index)
	{
			Ref<Entity> directionalLightEntity = GetDirectionalLightEntity();
			if (directionalLightEntity != nullptr && index < 1)
			{
				auto& transform = directionalLightEntity->GetComponent<TransformComponent>();
				auto& light = directionalLightEntity->GetComponent<LightComponent>();
				glm::vec3 direction = glm::rotate(glm::quat(transform.Rotation), glm::vec3(0.0f, 0.0f, -1.0f));
				s_DirectionalLightBuffer.position  = transform.Translation;
				s_DirectionalLightBuffer.direction = direction;
				s_DirectionalLightBuffer.ambient   = light.Ambient;
				s_DirectionalLightBuffer.diffuse   = light.Diffuse;
				s_DirectionalLightBuffer.specular  = light.Specular;
				UniformBufferManager::GetUniformBuffer("DirectionalLight")->SetData(&s_DirectionalLightBuffer.position,  sizeof(glm::vec3));
				UniformBufferManager::GetUniformBuffer("DirectionalLight")->SetData(&s_DirectionalLightBuffer.direction, sizeof(glm::vec3), 4 * sizeof(float));
				UniformBufferManager::GetUniformBuffer("DirectionalLight")->SetData(&s_DirectionalLightBuffer.ambient,   sizeof(glm::vec3), (4 + 4) * sizeof(float));
				UniformBufferManager::GetUniformBuffer("DirectionalLight")->SetData(&s_DirectionalLightBuffer.diffuse,   sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
				UniformBufferManager::GetUniformBuffer("DirectionalLight")->SetData(&s_DirectionalLightBuffer.specular,  sizeof(glm::vec3), (4 + 4 + 4 + 4) * sizeof(float));

				
				glm::mat4 lightProjection, lightView;
				glm::mat4 lightSpaceMatrix;
				float near_plane = -1.0f, far_plane = 200.0f;
				//lightProjection = glm::perspective(glm::radians(68.0f), 1.0f, 0.001f, 1000.0f);
				lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
				lightView = glm::inverse(transform.GetTransform());

				lightSpaceMatrix = lightProjection * lightView;
				UniformBufferManager::GetUniformBuffer("DirectionalLightSpaceMatrix")->SetData(&lightSpaceMatrix, sizeof(glm::mat4));
			}
			else
			{
				float zero[20] = { 0.0f };
				UniformBufferManager::GetUniformBuffer("DirectionalLight")->SetData(&zero, sizeof(zero));

				float zero1[16] = { 0.0f };
				UniformBufferManager::GetUniformBuffer("DirectionalLightSpaceMatrix")->SetData(&zero1, sizeof(zero1));
				
			}


			std::vector<Ref<Entity>> pointLightEntities = GetPointLightEntities();
			if (pointLightEntities.size() > index)
			{
				const uint32_t SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
				float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
				float m_Near = 0.1f;//1.0f;
				float m_Far = 25.0f;
				glm::mat4 shadowProj = glm::perspective(89.535f, aspect, m_Near, m_Far);// fov��Ӧ��90��������float�ļ���������������ͼ��Ұ��λ
				std::vector<glm::mat4> shadowTransforms;

				auto pointLightEntity = pointLightEntities[index];
				{
					auto& transform = pointLightEntity->GetComponent<TransformComponent>();
					auto& light = pointLightEntity->GetComponent<LightComponent>();
					s_PointLightBuffer.position  = transform.Translation;
					s_PointLightBuffer.ambient   = light.Ambient;
					s_PointLightBuffer.diffuse   = light.Diffuse;
					s_PointLightBuffer.specular  = light.Specular;
					s_PointLightBuffer.constant  = light.Constant;
					s_PointLightBuffer.linear    = light.Linear;
					s_PointLightBuffer.quadratic = light.Quadratic;
					UniformBufferManager::GetUniformBuffer("PointLight")->SetData(&s_PointLightBuffer.position,  sizeof(glm::vec3));
					UniformBufferManager::GetUniformBuffer("PointLight")->SetData(&s_PointLightBuffer.ambient,   sizeof(glm::vec3), 4 * sizeof(float));
					UniformBufferManager::GetUniformBuffer("PointLight")->SetData(&s_PointLightBuffer.diffuse,   sizeof(glm::vec3), (4 + 4) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("PointLight")->SetData(&s_PointLightBuffer.specular,  sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("PointLight")->SetData(&s_PointLightBuffer.constant,  sizeof(float),     (4 + 4 + 4 + 3) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("PointLight")->SetData(&s_PointLightBuffer.linear,    sizeof(float),     (4 + 4 + 4 + 3 + 1) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("PointLight")->SetData(&s_PointLightBuffer.quadratic, sizeof(float),     (4 + 4 + 4 + 3 + 1 + 1) * sizeof(float));
					
				    glm::vec3 lightPos = s_PointLightBuffer.position;
				    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
				    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
				    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0)));
				    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0)));
				    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0)));
				    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0, -1.0,  0.0)));

				}

				UniformBufferManager::GetUniformBuffer("PointLightSpaceMatrix")->SetData(&shadowTransforms[0], 6 * 4 * 4 * sizeof(float));
				UniformBufferManager::GetUniformBuffer("PointLightSpaceMatrix")->SetData(&m_Far, sizeof(float), 6 * 4 * 4 * sizeof(float));
			}
			else
			{
				float zero[20] = { 0.0f };
				UniformBufferManager::GetUniformBuffer("PointLight")->SetData(&zero,  sizeof(zero));

				float zero1[97] = { 0.0f };
				UniformBufferManager::GetUniformBuffer("PointLightSpaceMatrix")->SetData(&zero1, sizeof(zero1));

			}


			std::vector<Ref<Entity>> spotLightEntities = GetSpotLightEntities();
			if (spotLightEntities.size() > index)
			{
				glm::mat4 lightProjection, lightView;
				glm::mat4 lightSpaceMatrix;
				float m_Near = 0.1f, m_Far = 100.0f;
				lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, m_Near, m_Far);

				auto spotLightEntity = spotLightEntities[index];
				{
					auto& transform = spotLightEntity->GetComponent<TransformComponent>();
					auto& light = spotLightEntity->GetComponent<LightComponent>();
					s_SpotLightBuffer.position = transform.Translation;
					glm::vec3 direction = glm::rotate(glm::quat(transform.Rotation), glm::vec3(0.0f, 0.0f, -1.0f));
					s_SpotLightBuffer.direction   = direction;
					s_SpotLightBuffer.ambient     = light.Ambient;
					s_SpotLightBuffer.diffuse     = light.Diffuse;
					s_SpotLightBuffer.specular    = light.Specular;
					s_SpotLightBuffer.constant    = light.Constant;
					s_SpotLightBuffer.linear      = light.Linear;
					s_SpotLightBuffer.quadratic   = light.Quadratic;
					s_SpotLightBuffer.cutOff      = light.CutOff;//glm::cos(glm::radians(12.5f));
					s_SpotLightBuffer.outerCutOff = light.OuterCutOff;//glm::cos(glm::radians(17.5f));
					UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&s_SpotLightBuffer.position,    sizeof(glm::vec3));
					UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&s_SpotLightBuffer.direction,   sizeof(glm::vec3), 4 * sizeof(float));
					UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&s_SpotLightBuffer.ambient,     sizeof(glm::vec3), (4 + 4) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&s_SpotLightBuffer.diffuse,     sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&s_SpotLightBuffer.specular,    sizeof(glm::vec3), (4 + 4 + 4 + 4) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&s_SpotLightBuffer.constant,    sizeof(float),     (4 + 4 + 4 + 4 + 3) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&s_SpotLightBuffer.linear,      sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&s_SpotLightBuffer.quadratic,   sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&s_SpotLightBuffer.cutOff,      sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1) * sizeof(float));
					UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&s_SpotLightBuffer.outerCutOff, sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1 + 1) * sizeof(float));

					lightView = glm::inverse(transform.GetTransform());
					lightSpaceMatrix = lightProjection * lightView;
					UniformBufferManager::GetUniformBuffer("SpotLightSpaceMatrix")->SetData(&lightSpaceMatrix, sizeof(glm::mat4));
					UniformBufferManager::GetUniformBuffer("SpotLightSpaceMatrix")->SetData(&m_Far, sizeof(float), 4 * 4 * sizeof(float));
				}

			}
			else
			{
				float zero[24] = { 0.0f };
		        UniformBufferManager::GetUniformBuffer("SpotLight")->SetData(&zero, sizeof(zero));

				float zero1[17] = { 0.0f };
				UniformBufferManager::GetUniformBuffer("SpotLightSpaceMatrix")->SetData(&zero1, sizeof(zero1));

			}
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity& entity, T& component)
	{
		// ��̬����
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity& entity, IDComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity& entity, TagComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity& entity, TransformComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<AnimatorComponent>(Entity& entity, AnimatorComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<AnimationComponent>(Entity& entity, AnimationComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<LightComponent>(Entity& entity, LightComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity& entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity& entity, ScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MeshComponent>(Entity& entity, MeshComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MeshRendererComponent>(Entity& entity, MeshRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CircleRendererComponent>(Entity& entity, CircleRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity& entity, SpriteRendererComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity& entity, NativeScriptComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity& entity, Rigidbody2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity& entity, BoxCollider2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity& entity, CircleCollider2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SkyboxComponent>(Entity& entity, SkyboxComponent& component)
	{
	}
}