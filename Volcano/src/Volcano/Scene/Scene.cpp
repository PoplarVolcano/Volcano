#include "volpch.h"
#include "Scene.h"

#include "Volcano/Scene/Entity.h"
#include "Volcano/Scene/SceneBuffer.h"
#include "Volcano/Scripting/ScriptEngine.h"

// Box2D
#include "box2d/b2_world.h"

namespace Volcano
{
	Scene::Scene()
	{
		m_HDREnabled = false;
		m_Exposure = 1.0f;
		m_BloomEnabled = false;
	}

	Scene::~Scene()
	{
		delete m_PhysicsWorld;
	}

	void Scene::OnRuntimeStart()
	{
		OnPhysics2DStart();


		// 脚本初始化Scripting
		ScriptEngine::OnRuntimeStart(this);

		// 将EntityScriptFieldMap中存储的字段传给脚本实例
		auto view = m_Registry.view<ScriptComponent>();
		for (auto entity : view)
		{
			Ref<Entity>& entityPtr = m_EntityEnttMap[entity];

			const auto& scriptComponent = entityPtr->GetComponent<ScriptComponent>();
			if (scriptComponent.ClassName.empty())
				continue;

			UUID entityID = entityPtr->GetUUID();
			
			Ref<ScriptInstanceMonoBehaviour> monoBehaviour = ScriptEngine::CreateMonoBehaviourScriptInstanceByEntity(entityPtr, true);

			if (monoBehaviour == nullptr)
				continue;

			Ref<ScriptClass> scriptClass = monoBehaviour->GetScriptClass();

			EntityScriptFieldMap& entityScriptFieldMap = ScriptEngine::GetEntityScriptFieldMap();

			for (auto& [name, scriptField] : scriptClass->GetFields())
			{
				ScriptFieldInstance* fieldInstance = entityScriptFieldMap.TryGetField(entityID, scriptClass->GetFullNameHash(), scriptField.nameHash);
				if (fieldInstance != nullptr)
					monoBehaviour->SetFieldValueInternal(scriptField.nameHash, fieldInstance->GetBufferPtr());
			}
		}

		// 依序执行实体脚本的Awake函数
		for (auto& [entityID, instance] : ScriptEngine::GetEntityScriptInstanceMap())
		{
			ScriptEngine::EntityAwake(entityID);
		}

		// 依序执行实体脚本的Start函数
		for (auto entity : view)
		{
			Ref<Entity>& entityPtr = m_EntityEnttMap[entity];
			if (entityPtr->GetActive()
				&& entityPtr->GetComponent<ScriptComponent>().enabled
				&& !entityPtr->GetComponent<ScriptComponent>().ClassName.empty())
			{
				ScriptEngine::EntityStart(entityPtr->GetUUID());
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

	void Scene::OnUpdateRuntime()
	{
		if (!m_IsPaused || m_StepFrames-- > 0)
		{
			// scriptEntityList在SceneBuffer初始化时是空数组，在UpdateScene中Clear，所以这里不需要Clear
			auto& scriptEntityList = SceneBuffer::GetInstance().scriptEntityList;
			TraverseEntity(m_EntityList, [&](Ref<Entity> entity) ->bool {
				if (entity->HasComponent<ScriptComponent>()
					&& entity->GetComponent<ScriptComponent>().enabled
					&& !entity->GetComponent<ScriptComponent>().ClassName.empty())
				{
					scriptEntityList.push_back(entity);
				}
				return true;
				});

			//更新脚本
			for (auto& entity : scriptEntityList)
			{
				ScriptEngine::EntityUpdate(entity->GetUUID());
			}
			for (auto& entity : scriptEntityList)
			{
				ScriptEngine::EntityFixedUpdate(entity->GetUUID());
			}
			for (auto& entity : scriptEntityList)
			{
				ScriptEngine::EntityLateUpdate(entity->GetUUID());
			}

			// 结算 延迟调用
			auto& invokeListBuffer = ScriptEngine::GetEntityInvokeDelayedListBuffer();
			auto& invokeList = ScriptEngine::GetEntityInvokeDelayedList();
			for (auto& invoke : invokeListBuffer)
				invokeList.push_back(invoke);
			invokeListBuffer.clear();

			for (auto it = invokeList.begin(); it != invokeList.end(); )
			{
				float time = it->timer.GetSeconds();
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


			// 结算实体的增添、移动、销毁
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
						ScriptEngine::EntityOnDisable(srcEntity->GetUUID());
						ScriptEngine::EntityOnDestroy(srcEntity->GetUUID());
						ScriptEngine::RemoveEntityInvokeDelayed(buffer.srcID);
						srcEntity->GetScene()->DestroyEntity(srcEntity);
					}
					break;
				case EntityUpdateType::MOVE:
					Scene::MoveEntityToEntity(srcEntity, this->GetEntityByUUID(buffer.disID).get());
					break;
				}
			}

			// Script - Physic - Render顺序
			OnPhysics();
		}


		// 获取主摄像头
		Ref<Entity> mainCameraEntity = GetPrimaryCameraEntity();
		if (mainCameraEntity != nullptr)
		{
			auto& cameraComponent = mainCameraEntity->GetComponent<CameraComponent>();
			auto& mainCamera = cameraComponent.Camera;

			UpdateScene(mainCamera, glm::vec3(mainCameraEntity->GetWorldTransform()[3]), mainCameraEntity->GetWorldRotation());
		}

	}

	void Scene::OnRenderRuntime()
	{
		RenderScene();
	}

	void Scene::OnUpdateSimulation(EditorCamera& camera)
	{
		if (!m_IsPaused || m_StepFrames-- > 0)
			OnPhysics();

		UpdateScene(camera, camera.GetPosition(), camera.GetOrientation());
	}

	void Scene::OnRenderSimulation()
	{
		RenderScene();
	}

	void Scene::OnUpdateEditor(EditorCamera& camera)
	{
		UpdateScene(camera, camera.GetPosition(), camera.GetOrientation());
	}

	void Scene::OnRenderEditor()
	{
		RenderScene();
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (cameraComponent.fixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

	std::pair<uint32_t, uint32_t> Scene::GetViewportSize()
	{
		return std::pair<uint32_t, uint32_t>(m_ViewportWidth, m_ViewportHeight);
	}

	Ref<Entity> Scene::GetPrimaryCameraEntity()
	{
		Ref<Entity> cameraEntity = nullptr;
		TraverseEntity(m_EntityList, [&](Ref<Entity> entity) -> bool {

			if (entity->HasComponent<CameraComponent>()
				&& entity->GetComponent<CameraComponent>().enabled
				&& entity->GetComponent<CameraComponent>().primary)
			{
				cameraEntity = entity;
				return false;
			}
			return true;

			});
		return cameraEntity;
	}

	void Scene::Step(int frames)
	{
		m_StepFrames = frames;
	}

}