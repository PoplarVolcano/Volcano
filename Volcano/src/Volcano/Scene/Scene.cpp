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
#include "Volcano/Math/Math.h"
#include "Volcano/Scene/Prefab.h"

#include "glm/glm.hpp"
#include "Entity.h"

// Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"


// Box3D
#include "Volcano/Physics/Physic/b3_World.h"
#include "Volcano/Physics/Physic/b3_Body.h"
#include "Volcano/Physics/Physic/b3_Fixture.h"
#include "Volcano/Physics/Physic/b3_Contact.h"
#include "Volcano/Physics/Physic/Collision/b3_SphereShape.h"
#include "Volcano/Physics/Physic/Collision/b3_BoxShape.h"
#include "Volcano/Physics/Physic/Physic3D.h"
#include "Volcano/Physics/Physic/b3_Math.h"

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
				UUID& id = src.get<IDComponent>(srcEntity).ID;
				VOL_CORE_ASSERT(entityIDMap.find(id) != entityIDMap.end());
				entt::entity dstEntity = entityIDMap.at(id)->GetEntityHandle();
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


	/*
	void CopyEntityChildren(Ref<Entity>& srcEntity, Ref<Entity>& dstEntity, std::unordered_map<UUID, Ref<Entity>>& entityIDMap, std::unordered_map<entt::entity, Ref<Entity>>& entityEnttMap)
	{
		if (!srcEntity->GetEntityChildren().empty())
		{
			auto& dstEntityChildren = dstEntity->GetEntityChildren();
			for (auto& entityChild : srcEntity->GetEntityChildren())
			{
				Ref<Entity> entity = Entity::Create(*dstEntity->GetScene(), entityChild->GetUUID(), entityChild->GetName());
				entity->SetActive(entityChild->GetActive());
				entity->SetEntityParent(srcEntity.get());
				entityIDMap[entity->GetUUID()] = entity;
				entityEnttMap[entity->GetEntityHandle()] = entity;
				dstEntityChildren.push_back(entity);

				CopyEntityChildren(entityChild, entity, entityIDMap, entityEnttMap);
			}
		}
	}
	*/

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
		auto& entityList = newScene->GetEntityList();

		for (auto& child : other->GetEntityList())
		{
			/*
			Ref<Entity> newEntity = Entity::Create(*newScene.get(), entityChild->GetUUID(), entityChild->GetName());
			newEntity->SetActive(entityChild->GetActive());
			entityIDMap[newEntity->GetUUID()] = newEntity;
			entityEnttMap[newEntity->GetEntityHandle()] = newEntity;
			entityList.push_back(newEntity);
			CopyEntityChildren(entityChild, newEntity, entityIDMap, entityEnttMap);
			*/
			newScene->DuplicateEntity(child, nullptr, child->GetUUID());
		}

		// ��ȡ��ʵ������������Ȼ����API�����ƾ�ʵ��������������ʵ�壬�������������������������ֵ
		// Copy components (except IDComponent and TagComponent)
		//CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, entityIDMap);

		// MeshComponent��Meshָ�븴�ƺ��ǰ󶨾�mesh����mesh�󶨾�Entity����Ҫ��������һ������������mesh������Entity
		/*
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
		*/
		return newScene;
	}

	Ref<Entity> Scene::CreateEntity(const std::string& name, Entity* parent)
	{
		return CreateEntityWithUUID(UUID(), name, parent);
	}

	std::string Scene::NewName(std::vector<Ref<Entity>>& entityList, std::string name)
	{
		std::string newName = name;
		int i = 0;
		while (FindIteratorInList(newName, entityList) != entityList.end())
		{
			newName = name + "(" + std::to_string(i) + ")";
			i++;
		}

		return newName;
	}

	Ref<Entity> Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, Entity* parent)
	{
		Ref<Entity> entityTemp;

		if (parent != nullptr)
		{
			entityTemp = parent->AddEntityChild(uuid, name);
			m_EntityIDMap[entityTemp->GetUUID()] = entityTemp;
			m_EntityEnttMap[entityTemp->GetEntityHandle()] = entityTemp;
			return entityTemp;
		}

		std::string newName = NewName(m_EntityList, name);

		entityTemp = Entity::Create(*this, uuid, newName);
		m_EntityIDMap[entityTemp->GetUUID()] = entityTemp;
		m_EntityEnttMap[entityTemp->GetEntityHandle()] = entityTemp;
		m_EntityList.push_back(entityTemp);

		return entityTemp;
	}


	void Scene::DestroyEntityChild(Ref<Entity> entity)
	{
		auto& entityChildren = entity->GetEntityChildren();
		while (!entityChildren.empty())
		{
			auto it = entityChildren.end() - 1;
			DestroyEntityChild(*it);
			m_EntityIDMap.erase((*it)->GetUUID());
			m_EntityEnttMap.erase((*it)->GetEntityHandle());
			entityChildren.erase(it);
			m_Registry.destroy((*it)->GetEntityHandle());
		}
	}

	// ����ʵ��
	void Scene::DestroyEntity(Ref<Entity> entity)
	{
		if (m_Name == "Prefab")
			Prefab::RemovePrefab(entity);
		else
		    Prefab::RemovePrefabTarget(entity);

		// ���������ӽڵ�
		DestroyEntityChild(entity);

		m_EntityIDMap.erase(entity->GetUUID());
		m_EntityEnttMap.erase(entity->GetEntityHandle());
		// �Ӹ��ڵ����Ƴ��Լ�
		Entity* entityParent = entity->GetEntityParent();
		if (entityParent == nullptr)
		{
			Scene::RemoveEntityFromList(entity, m_EntityList);
		}
		else
		{
			Scene::RemoveEntityFromList(entity, entityParent->GetEntityChildren());
		}
		m_Registry.destroy(entity->GetEntityHandle());
	}

	// ����Active��entity��Scene.Destroy�������entity�ĸ��ڵ���ӽڵ��б������ TraverseEntity��������ݹ�
	void TraverseEntity(std::vector<Ref<Entity>> entityList, const std::function<void(Ref<Entity>)>& function)
	{

		std::stack<Ref<Entity>> entityStack;
		for (auto& entity : entityList)
		{
			entityStack.push(entity);
		}

		while (!entityStack.empty())
		{
			Ref<Entity> entity = entityStack.top();
			entityStack.pop();

			if (entity->GetActive())
			{
				function(entity);

				for (auto& child : entity->GetEntityChildren())
				{
					entityStack.push(child);
				}
			}
		}
	}

	void Scene::OnRuntimeStart()
	{
		OnPhysics2DStart();
		OnPhysics3DStart();

		// �ű���ʼ��Scripting
		{
			ScriptEngine::OnRuntimeStart(this);
			// Instantiate all script entities

			TraverseEntity(m_EntityList, [](Ref<Entity> entity) { if (entity->HasComponent<ScriptComponent>() && !entity->GetComponent<ScriptComponent>().ClassName.empty()) ScriptEngine::CreateEntity(*entity.get());   });
			TraverseEntity(m_EntityList, [](Ref<Entity> entity) { if (entity->HasComponent<ScriptComponent>() && !entity->GetComponent<ScriptComponent>().ClassName.empty()) ScriptEngine::AwakeEntity(*entity.get());    });
			TraverseEntity(m_EntityList, [](Ref<Entity> entity) { if (entity->HasComponent<ScriptComponent>() && !entity->GetComponent<ScriptComponent>().ClassName.empty()) ScriptEngine::OnEnableEntity(*entity.get()); });
			TraverseEntity(m_EntityList, [](Ref<Entity> entity) { if (entity->HasComponent<ScriptComponent>() && !entity->GetComponent<ScriptComponent>().ClassName.empty()) ScriptEngine::StartEntity(*entity.get());    });
		}
	}

	void Scene::OnRuntimeStop()
	{
		OnPhysics2DStop();
		//OnPhysics3DStop();

		TraverseEntity(m_EntityList, [](Ref<Entity> entity) { if (entity->HasComponent<ScriptComponent>() && !entity->GetComponent<ScriptComponent>().ClassName.empty()) ScriptEngine::OnDisableEntity(*entity.get()); });
		ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnSimulationStart()
	{
		OnPhysics2DStart();
		OnPhysics3DStart();
	}

	void Scene::OnSimulationStop()
	{
		OnPhysics2DStop();
		//OnPhysics3DStop();

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

	void Scene::OnPhysics3DStart()
	{
		// ����һ����������/����
		// �������ٶ�����
		m_Physics3DWorld = new b3_World({ 0.0f, -9.8f, 0.0f });
		//m_physic3DListener = Listener(this);
		//m_Physics3DWorld->SetContactListener(&m_physic3DListener);
		// Ϊ��ǰ�������о������������ʵ�崴��b3_Body
		auto view = m_Registry.view<RigidbodyComponent>();
		TraverseEntity(m_EntityList, [&](Ref<Entity> entity)
			{
				if (entity->HasComponent<RigidbodyComponent>())
				{
					auto& transform = entity->GetComponent<TransformComponent>();
					auto& rb = entity->GetComponent<RigidbodyComponent>();

					b3_BodyDef bodyDef;
					bodyDef.type = Utils::RigidbodyTypeToBody(rb.Type);
					bodyDef.position = entity->GetParentTransform() * glm::vec4(transform.Translation, 1.0f);
					bodyDef.rotation = entity->GetParentTransform() * glm::vec4(transform.Rotation, 1.0f);
					bodyDef.userData.pointer = entity->GetUUID();

					b3_Body* body = m_Physics3DWorld->CreateBody(&bodyDef);
					// �Ƿ�̶���ת
					body->SetFixedRotation(rb.FixedRotation);
					rb.RuntimeBody = body;


					if (entity->HasComponent<BoxColliderComponent>() && entity->GetComponent<BoxColliderComponent>().enabled)
					{
						auto& bc = entity->GetComponent<BoxColliderComponent>();
						// ������Ӱ�Χ��

						b3_BoxShape boxShape;
						glm::vec3 size = bc.size / 2.0f * transform.Scale;
						boxShape.Set(&size);

						// ����fixture��fixture��������İ�Χ��
						b3_FixtureDef fixtureDef;
						fixtureDef.shape = &boxShape;
						fixtureDef.density = bc.material.density;
						fixtureDef.friction = bc.material.staticFriction;
						fixtureDef.restitution = bc.material.bounciness;
						fixtureDef.restitutionThreshold = bc.material.restitutionThreshold;
						// ���������fixture
						body->CreateFixture(&fixtureDef);
					}

					if (entity->HasComponent<SphereColliderComponent>() && entity->GetComponent<SphereColliderComponent>().enabled)
					{
						auto& ccd = entity->GetComponent<SphereColliderComponent>();

						b3_SphereShape sphereShape;
						sphereShape.m_position = { ccd.center.x, ccd.center.y, ccd.center.z };
						sphereShape.m_radius = transform.Scale.x * ccd.radius;

						b3_FixtureDef fixtureDef;
						fixtureDef.shape = &sphereShape;
						fixtureDef.density = ccd.material.density;
						fixtureDef.friction = ccd.material.staticFriction;
						fixtureDef.restitution = ccd.material.bounciness;
						fixtureDef.restitutionThreshold = ccd.material.restitutionThreshold;
						fixtureDef.userData.pointer = entity->GetUUID();
						body->CreateFixture(&fixtureDef);
					}
				}
			});
	}
	void Scene::OnPhysics3DStop()
	{
		delete m_Physics3DWorld;
		m_Physics3DWorld = nullptr;
	}



	void Scene::Physics(Timestep ts)
	{
		// Physic2D
		{
			// �ű�Ӱ��pyhsicȻ����Ⱦ����ǰ֡�õ����
			// �����ٶȣ�ʹ�ø��ٵĵ�������������ܣ���׼ȷ�Ի��ܵ�Ӱ�졣ʹ�ø�������ή�����ܵ������ģ������
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

				// ��ȡ����ģ�������body
				b2Body* body = (b2Body*)rb2d.RuntimeBody;

				// ��������ֵ����ʵ��
				const auto& position = body->GetPosition();
				transform.Translation.x = position.x;
				transform.Translation.y = position.y;
				transform.Rotation.z = body->GetAngle();
			}
		}

		// Physic3D
		{
			
			TraverseEntity(m_EntityList, [&](Ref<Entity> entity)
				{
					if (entity->HasComponent<RigidbodyComponent>())
					{
						auto& transform = entity->GetComponent<TransformComponent>();
						b3_Body* body = (b3_Body*)entity->GetComponent<RigidbodyComponent>().RuntimeBody;
						body->SetTransform(transform.Translation, transform.Rotation);
					}
				});
				
			const int velocityIterations = 1;
			const int positionIterations = 2;
			// ��Box3D�����任(Retrieve transform)
			m_Physics3DWorld->Step(ts, velocityIterations, positionIterations);

			TraverseEntity(m_EntityList, [&](Ref<Entity> entity)
				{
					if (entity->HasComponent<RigidbodyComponent>())
					{
						b3_Body* body = (b3_Body*)entity->GetComponent<RigidbodyComponent>().RuntimeBody;
						entity->SetPosition(body->GetPosition(), false);
						entity->SetRotation(body->GetRotation(), false);
					}
				});
		}
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		if (!m_IsPaused || m_StepFrames-- > 0)
		{
			//Update scripts
			{
				// MonoBehaviour.Update(ts)
				TraverseEntity(m_EntityList, [ts](Ref<Entity> entity) { if (entity->HasComponent<ScriptComponent>() && !entity->GetComponent<ScriptComponent>().ClassName.empty()) ScriptEngine::UpdateEntity(*entity.get(), ts); });

				// InvokeDelayed, InvokeDelayed��ȡ�󣬼�ʹ�ر�Entity��ScriptComponent��InvokeDelayedҲ��ִ�С�
				auto& invokeListBuffer = ScriptEngine::GetEntityInvokeDelayedListBuffer();
				auto& invokeList = ScriptEngine::GetEntityInvokeDelayedList();
				for (auto& invoke : invokeListBuffer)
					invokeList.push_back(invoke);
				invokeListBuffer.clear();

				for (auto it = invokeList.begin(); it != invokeList.end(); )
				{
					float time = it->timer.Elapsed();
					auto scriptClass = it->instance->GetScriptClass();
					if (it->repeatRate == 0.0f)
					{
						if (time >= it->time)
						{
							scriptClass->InvokeMethod(it->instance->GetManagedObject(), it->method.ClassMethod);
							it = invokeList.erase(it);
						}
						else
							it++;
					}
					else
					{
						if (it->firstInvoke)
						{
							if (time >= it->time)
							{
								scriptClass->InvokeMethod(it->instance->GetManagedObject(), it->method.ClassMethod);
								it->timer.Reset();
								it->firstInvoke = false;
							}
						}
						else
							if (time >= it->repeatRate)
							{
								scriptClass->InvokeMethod(it->instance->GetManagedObject(), it->method.ClassMethod);
								it->timer.Reset();
							}
						it++;
					}
				}

			}


			// Destroy
			auto& updateList = ScriptEngine::GetEntityUpdateList();
			while (!updateList.empty())
			{
				EntityUpdateBuffer& buffer = updateList.front();
				updateList.pop();

				Ref<Entity> srcEntity = this->GetEntityByUUID(buffer.srcID);
				switch (buffer.type)
				{
				case EntityUpdateType::ADD:
					break;
				case EntityUpdateType::DESTROY:
					if (srcEntity != nullptr)
					{
						ScriptEngine::OnDisableEntity(*srcEntity.get());
						ScriptEngine::OnDestroyEntity(*srcEntity.get());
						ScriptEngine::RemoveEntityInvokeDelayed(buffer.srcID);
						srcEntity->GetScene()->DestroyEntity(srcEntity);
					}
					break;
				case EntityUpdateType::MOVE:
					Scene::UpdateEntityParent(srcEntity, this->GetEntityByUUID(buffer.disID).get());
					break;
				}
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
				cameraTransform = mainCameraEntity->GetParentTransform() * mainCameraEntity->GetTransform();
				glm::vec3 parentPosition, parentRotation, parentScale;
				Math::DecomposeTransform(mainCameraEntity->GetParentTransform(), parentPosition, parentRotation, parentScale);
				cameraPosition = parentPosition + transform.Translation;
				cameraDirection = glm::rotate(glm::quat(parentRotation + transform.Rotation), glm::vec3(0.0f, 0.0f, -1.0f));
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

	// parentΪ��ʱ����SceneĿ¼�¿�¡entity
	Ref<Entity> Scene::DuplicateEntity(Ref<Entity> entity, Entity* parent, UUID id)
	{
		// Copy name because we're going to modify component data structure
		std::string name = entity->GetName();
		Ref<Entity> newEntity = CreateEntityWithUUID(id, name, parent);
		newEntity->SetActive(entity->GetActive());
		CopyComponentIfExists(AllComponents{}, newEntity, entity);

		// MeshComponent��Meshָ�븴�ƺ��ǰ󶨾�mesh����mesh�󶨾�Entity����Ҫ��������һ������������mesh������Entity
		if (newEntity->HasComponent<MeshComponent>())
		{
			auto& mc = newEntity->GetComponent<MeshComponent>();
			auto modelPath = mc.modelPath;
			auto vertexBone = mc.vertexBone;
			mc.SetMesh(mc.meshType, newEntity.get(), mc.mesh);
			mc.modelPath = modelPath;
			mc.vertexBone = vertexBone;
		}

		// AnimatorComponent���ƺ�animatorָ����Ҫ����
		if (newEntity->HasComponent<AnimatorComponent>())
		{
			auto& ac = newEntity->GetComponent<AnimatorComponent>();
			ac.Reset();
		}
		if (id == 0)
		{
			for (auto& child : entity->GetEntityChildren())
				DuplicateEntity(child, newEntity.get());
		}
		else
		{
			for (auto& child : entity->GetEntityChildren())
				DuplicateEntity(child, newEntity.get(), child->GetUUID());
		}

		return newEntity;
	}

	Ref<Entity> Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			if (m_EntityEnttMap[entity]->GetActive())
			{
				auto camera = view.get<CameraComponent>(entity);
				if (camera.Primary)
					return m_EntityEnttMap[entity];
			}
		}
		return {};
	}
	
	Ref<Entity> Scene::GetDirectionalLightEntity()
	{
		// ��ȡ��Դ
		auto view = m_Registry.view<LightComponent>();
		for (auto entity : view)
		{
			if (m_EntityEnttMap[entity]->GetActive())
			{
				auto& lightComponent = view.get<LightComponent>(entity);
				if (lightComponent.enabled && lightComponent.Type == LightComponent::LightType::DirectionalLight)
					return m_EntityEnttMap[entity];
			}
		}
		return {};
	}

	std::vector<Ref<Entity>> Scene::GetPointLightEntities()
	{
		std::vector<Ref<Entity>> pointLights;
		auto view = m_Registry.view<LightComponent>();
		for (auto entity : view)
		{
			if (m_EntityEnttMap[entity]->GetActive())
			{
				auto& lightComponent = view.get<LightComponent>(entity);
				if (lightComponent.enabled && lightComponent.Type == LightComponent::LightType::PointLight)
					pointLights.push_back(m_EntityEnttMap[entity]);// entt�ǴӺ���ǰ�������õ���vector�ǵ����
			}
		}
		return pointLights;

	}
	std::vector<Ref<Entity>> Scene::GetSpotLightEntities()
	{
		std::vector<Ref<Entity>> spotLights;
		auto view = m_Registry.view<LightComponent>();
		for (auto entity : view)
		{
			if (m_EntityEnttMap[entity]->GetActive())
			{
				auto& lightComponent = view.get<LightComponent>(entity);
				if (lightComponent.enabled && lightComponent.Type == LightComponent::LightType::SpotLight)
					spotLights.push_back(m_EntityEnttMap[entity]);
			}
		}
		return spotLights;
	}

	Ref<Entity> Scene::GetPrimarySkyboxEntity()
	{
		auto view = m_Registry.view<SkyboxComponent>();
		for (auto entity : view)
		{
			if (m_EntityEnttMap[entity]->GetActive())
			{
				auto& skyboxComponent = view.get<SkyboxComponent>(entity);
				if (skyboxComponent.enabled && skyboxComponent.Primary)
					return m_EntityEnttMap[entity];
			}
		}
		return {};
	}

	Ref<Entity> Scene::Find(std::string name, std::vector<Ref<Entity>>& entityList)
	{
		// ��name��·���ָ�ɸ���·������ʵ��
		std::vector<std::string> result;
		std::filesystem::path fs_path(name);
		for (auto& piece : fs_path)
			result.push_back(piece.string());

		auto currentName = result.begin();
		std::stack<Ref<Entity>> entityStack;
		for (auto& entity : entityList)
		{
			entityStack.push(entity);
		}

		while (!entityStack.empty())
		{
			Ref<Entity> entity = entityStack.top();
			entityStack.pop();

			if (entity->GetName() == *currentName)
				if (currentName == result.end() - 1)
					return entity;
				else
					currentName++;

			for (auto& child : entity->GetEntityChildren())
			{
				entityStack.push(child);
			}
		}

		return nullptr;
	}

	void Scene::RemoveEntityFromList(Ref<Entity> entity, std::vector<Ref<Entity>>& entityList)
	{
		auto it = std::find_if(entityList.begin(), entityList.end(), [&](Ref<Entity>& entityTemp) { return entityTemp->GetUUID() == entity->GetUUID(); });
		if (it != entityList.end())
			entityList.erase(it);
	}

	// parentΪ�գ�targetScene�ǿ� => ��entity�ƶ���targetScene��scene·����
	// parentΪ�գ�entity�и��ڵ�  => ��entity�ƶ����Լ���scene·����
	// parent�ǿ�                  => ��entity�ƶ���parent��
	void Scene::UpdateEntityParent(Ref<Entity> entity, Entity* parent, Scene* targetScene)
	{
		if (entity == nullptr)
			return;

		if (parent == nullptr && targetScene != nullptr)
		{
			if (entity->GetScene() == targetScene)
				return;
			targetScene->DuplicateEntity(entity);
			entity->GetScene()->DestroyEntity(entity);
			return;
		}

		if (parent == nullptr && targetScene == nullptr && entity->GetEntityParent() != nullptr)
		{
			Scene::RemoveEntityFromList(entity, entity->GetEntityParent()->GetEntityChildren());
			entity->SetName(Scene::NewName(entity->GetScene()->GetEntityList(), entity->GetName()));
			entity->GetScene()->GetEntityList().push_back(entity);
			entity->SetEntityParent(nullptr);
			return;
		}

		if (parent != nullptr)
		{
			if (entity->GetScene() == parent->GetScene())
			{
				if (entity->GetEntityParent() != nullptr)
					Scene::RemoveEntityFromList(entity, entity->GetEntityParent()->GetEntityChildren());
				else
				{
					Scene::RemoveEntityFromList(entity, entity->GetScene()->GetEntityList());
				}
				entity->SetName(Scene::NewName(parent->GetEntityChildren(), entity->GetName()));
				parent->GetEntityChildren().push_back(entity);
				entity->SetEntityParent(parent);
			}
			else
			{
				parent->GetScene()->DuplicateEntity(entity, parent);
				entity->GetScene()->DestroyEntity(entity);
			}
			return;
		}
	}

	std::vector<Ref<Entity>>::iterator Scene::FindIteratorInList(std::string name, std::vector<Ref<Entity>>& entityList)
	{
		for (auto it = entityList.begin(); it != entityList.end(); it++)
			if (it->get()->GetName() == name)
				return it;
		return entityList.end();
	}

	Ref<Entity> Scene::GetEntityByUUID(UUID uuid)
	{
		// ���������Ҳ��������Դ�Prefab������
		if (m_EntityIDMap.find(uuid) != m_EntityIDMap.end())
			return m_EntityIDMap[uuid];
		else if (Prefab::GetScene()->GetEntityIDMap().find(uuid) != Prefab::GetScene()->GetEntityIDMap().end())
			return Prefab::GetScene()->GetEntityIDMap()[uuid];
		return {};
	}

	void Scene::UpdateScene(Timestep ts)
	{
		// Update Skybox
		auto skyboxEntity = GetPrimarySkyboxEntity();
		if (skyboxEntity != nullptr)
			Skybox::SetTexture(skyboxEntity->GetComponent<SkyboxComponent>().texture);

		// Update Animator
		TraverseEntity(m_EntityList, [ts](Ref<Entity> entity) {

			if (entity->HasComponent<AnimatorComponent>() && entity->HasComponent<AnimationComponent>() && entity->GetComponent<AnimatorComponent>().enabled && entity->GetComponent<AnimationComponent>().enabled)
			{
				auto& animatorComponent = entity->GetComponent<AnimatorComponent>();
				auto& animationComponent = entity->GetComponent<AnimationComponent>();
				if (animationComponent.animation != nullptr)
				{
					animatorComponent.animator->SetAnimation(animationComponent.animation.get());
					animatorComponent.animator->UpdateAnimation(ts);
				}
			}

			});


		// Update Mesh
		{
			auto view = m_Registry.view<TransformComponent, MeshComponent, MeshRendererComponent>();
			std::vector<glm::mat4> finalBoneMatrices;

			for (auto entity : view)
			{
				if (m_EntityEnttMap[entity]->GetActive())
				{
					auto [meshTransform, mesh, renderer] = view.get<TransformComponent, MeshComponent, MeshRendererComponent>(entity);
					Entity* entityNode;
					switch (mesh.meshType)
					{
					case MeshType::None:
						break;
					case MeshType::Plane:
					case MeshType::Cube:
					case MeshType::Sphere:
					case MeshType::Cylinder:
					case MeshType::Capsule:
						mesh.mesh->StartBatch();

						if (!mesh.vertexBone.empty())
						{
							mesh.mesh->SetVertexBoneDataToDefault();
							for (auto& vertexBone : mesh.vertexBone)
								mesh.mesh->SetBoneID(vertexBone.vertexIndex1, vertexBone.vertexIndex2, vertexBone.boneIndex, vertexBone.weight);

							/*
							entityNode = m_EntityEnttMap[entity].get();
							do
							{
								if (entityNode->HasComponent<AnimationComponent>() && entityNode->HasComponent<AnimatorComponent>())
								{
									auto& animator = entityNode->GetComponent<AnimatorComponent>().animator;
									mesh.mesh->DrawMesh((int)entity, animator->GetFinalBoneMatrices());
									break;
								}
								else
									entityNode = entityNode->GetEntityParent();
							} while (entityNode != nullptr);

							if (entityNode == nullptr)
								mesh.mesh->DrawMesh((int)entity, finalBoneMatrices);
							*/
							mesh.mesh->DrawMesh((int)entity, finalBoneMatrices);
						}
						else
							mesh.mesh->DrawMesh((int)entity, finalBoneMatrices);
						break;
					case MeshType::Model:
						if (mesh.mesh != nullptr)
						{
							mesh.mesh->StartBatch();
							/*
							entityNode = m_EntityEnttMap[entity].get();
							do
							{
								if (entityNode->HasComponent<AnimationComponent>() && entityNode->HasComponent<AnimatorComponent>())
								{
									auto& animatorComponent = entityNode->GetComponent<AnimatorComponent>();
									auto& animationComponent = entityNode->GetComponent<AnimationComponent>();
									if (animatorComponent.enabled && animationComponent.enabled)
									{
										mesh.mesh->DrawMesh((int)entity, animatorComponent.animator->GetFinalBoneMatrices());
										break;
									}
									else
										entityNode = entityNode->GetEntityParent();
								}
								else
									entityNode = entityNode->GetEntityParent();
							} while (entityNode != nullptr);

							if (entityNode == nullptr)
							{
								mesh.mesh->DrawMesh((int)entity, finalBoneMatrices);
							}
							*/
							mesh.mesh->DrawMesh((int)entity, finalBoneMatrices);
						}
						break;
					default:
						VOL_TRACE("����MeshType");
						break;
					}
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

		if (m_RenderType == RenderType::COLLIDER)
		{
			RendererAPI::SetPolygonMode(true);
			RendererAPI::SetDepthTest(false);
			TraverseEntity(m_EntityList, [&](Ref<Entity> entity) {

				if (!entity->HasComponent<RigidbodyComponent>())
					return;
				std::vector<glm::mat4> finalBoneMatrices;
				if (entity->HasComponent<SphereColliderComponent>())
				{
					auto& sphereColliderComponent = entity->GetComponent<SphereColliderComponent>();
					if (sphereColliderComponent.enabled && sphereColliderComponent.showCollider)
					{
						auto& transformComponent = entity->GetComponent<TransformComponent>();
						SphereMesh::GetInstance()->BeginScene(camera, transform, position);
						SphereMesh::GetInstance()->StartBatch();
						SphereMesh::GetInstance()->DrawMesh(entity->GetUUID(), finalBoneMatrices);
						SphereMesh::GetInstance()->BindShader(m_RenderType);
						b3_Body* body = (b3_Body*)entity->GetComponent<RigidbodyComponent>().RuntimeBody;
						if (body != nullptr)
						{
							glm::mat4 transform = body->GetTransform().Transform() * 
								glm::mat4(transformComponent.Scale.x * sphereColliderComponent.radius);
							SphereMesh::GetInstance()->DrawSphere(transform);
						}
						else
						{
							glm::mat4 transform = glm::translate(glm::mat4(1.0f), transformComponent.Translation) * 
								glm::toMat4(glm::quat(transformComponent.Rotation)) * 
								glm::mat4(transformComponent.Scale.x * sphereColliderComponent.radius);
							SphereMesh::GetInstance()->DrawSphere(transform);
						}
					}

				}
				if (entity->HasComponent<BoxColliderComponent>())
				{
					auto& boxColliderComponent = entity->GetComponent<BoxColliderComponent>();
					if (boxColliderComponent.enabled && boxColliderComponent.showCollider)
					{
						auto& transformComponent = entity->GetComponent<TransformComponent>();
						CubeMesh::GetInstance()->BeginScene(camera, transform, position);
						CubeMesh::GetInstance()->StartBatch();
						CubeMesh::GetInstance()->DrawMesh(entity->GetUUID(), finalBoneMatrices);
						CubeMesh::GetInstance()->BindShader(m_RenderType);
						b3_Body* body = (b3_Body*)entity->GetComponent<RigidbodyComponent>().RuntimeBody;
						if (body != nullptr)
						{
							glm::mat4 transform = body->GetTransform().Transform() *
								glm::scale(glm::mat4(1.0f), transformComponent.Scale * boxColliderComponent.size);
							CubeMesh::GetInstance()->DrawCube(transform);
						}
						else
						{
							glm::mat4 transform = glm::translate(glm::mat4(1.0f), transformComponent.Translation) * 
								glm::toMat4(glm::quat(transformComponent.Rotation)) * 
								glm::scale(glm::mat4(1.0f), transformComponent.Scale * boxColliderComponent.size);
							CubeMesh::GetInstance()->DrawCube(transform);
						}
					}
				}
				});
			RendererAPI::SetPolygonMode(false);
			RendererAPI::SetDepthTest(true);
			return;

		}

		Renderer2D::BeginScene(camera, transform);

		// Draw sprites
		{
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				if (m_EntityEnttMap[entity]->GetActive())
				{
					auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
					Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
				}
			}
		}

		// Draw circles
		{
			auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : view)
			{
				if (m_EntityEnttMap[entity]->GetActive())
				{
					auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
					Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
				}
			}
		}
		Renderer2D::EndScene();

		//Draw Mesh
		TraverseEntity(m_EntityList, [&](Ref<Entity> entity) {

			if (entity->HasComponent<MeshComponent>() && entity->HasComponent<MeshRendererComponent>() && entity->GetComponent<MeshRendererComponent>().enabled)
			{
				auto& transformComponent    = entity->GetComponent<TransformComponent>();
				auto& meshComponent         = entity->GetComponent<MeshComponent>();
				auto& meshRendererComponent = entity->GetComponent<MeshRendererComponent>();

				switch (meshComponent.meshType)
				{
				case MeshType::None:
					break;
				case MeshType::Plane:
				case MeshType::Cube:
				case MeshType::Sphere:
				case MeshType::Cylinder:
				case MeshType::Capsule:
					meshComponent.mesh->BeginScene(camera, transform, position);
					meshComponent.mesh->BindTextures(meshRendererComponent.Textures);
					meshComponent.mesh->BindShader(m_RenderType);
					meshComponent.mesh->EndScene();
					break;
				case MeshType::Model:
					if (meshComponent.mesh != nullptr)
					{
						meshComponent.mesh->BeginScene(camera, transform, position);
						meshComponent.mesh->BindTextures(meshRendererComponent.Textures);
						meshComponent.mesh->BindShader(m_RenderType);
						meshComponent.mesh->EndScene();
					}
					break;
				default:
					VOL_TRACE("����MeshType");
					break;
				}
			}

			});
		/*
		{
			auto view = m_Registry.view<TransformComponent, MeshComponent, MeshRendererComponent>();

			for (auto entity : view)
			{
				if (m_EntityEnttMap[entity]->GetActive())
				{
					auto [meshTransform, mesh, renderer] = view.get<TransformComponent, MeshComponent, MeshRendererComponent>(entity);

					switch (mesh.meshType)
					{
					case MeshType::None:
						break;
					case MeshType::Plane:
					case MeshType::Cube:
					case MeshType::Sphere:
					case MeshType::Cylinder:
					case MeshType::Capsule:
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
		}
		*/

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
	void Scene::OnComponentAdded<RigidbodyComponent>(Entity& entity, RigidbodyComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxColliderComponent>(Entity& entity, BoxColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SphereColliderComponent>(Entity& entity, SphereColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SkyboxComponent>(Entity& entity, SkyboxComponent& component)
	{
	}

	void Listener::BeginContact(b3_Contact* contact)
	{
		VOL_TRACE("Listener1");
		if (m_scene != nullptr)
		{
			VOL_TRACE("Listener1");
			UUID uuid = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
			Ref<Entity> entity = m_scene->GetEntityByUUID(uuid);
			if (entity && entity->HasComponent<ScriptComponent>() && entity->GetComponent<ScriptComponent>().enabled)
			{
				VOL_TRACE("Listener2");
				auto instance = ScriptEngine::GetEntityScriptInstance(uuid);
				if (instance)
				{
					VOL_TRACE("Listener3");
					instance->InvokeOnTriggerEnter(ScriptEngine::CreateInstance(ScriptEngine::GetColliderClass()->GetClass(), uuid));
				}
			}
		}
	}
}