#include "volpch.h"
#include "Scene.h"

#include "Components.h"
#include "ScriptableEntity.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Renderer/Renderer2D.h"
#include "Volcano/Renderer/Renderer3D.h"
#include "Volcano/Renderer/RendererModel.h"
#include "Volcano/Physics/Physics2D.h"

#include "glm/glm.hpp"
#include "Entity.h"

// Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

namespace Volcano {

	Scene::Scene()
	{
		delete m_PhysicsWorld;
	}

	Scene::~Scene()
	{
	}

	// 将源注册表下实体复制到目标注册表，Map以UUID作为标记获取新实体
	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{ 
		// 这个lambda会递归调用
		// [&]表示引用传递方式捕捉所有父作用域的变量（包括this）
		// 隐式引用捕获dst、src或者"解开的Component包引用"，下面的Component是指具体的单个组件
		([&]()
		{
			auto view = src.view<Component>();
			for (auto srcEntity : view)
			{
				entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);
				auto& srcComponent = src.get<Component>(srcEntity);
				dst.emplace_or_replace<Component>(dstEntity, srcComponent);
			}

		}(), ...);// 这三个点应该是解Component包
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	// 如果源实体存在则把对应组件复制到目标实体
	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
		{
			if (src.HasComponent<Component>())
				dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		// 创建新场景，为新场景创建和旧场景同名和uuid的实体，并用map存入（旧实体的uuid对应新实体）的关系
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// 遍历旧场景所有uuid组件的旧实体，用旧实体的uuid和name创建新实体
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// 获取旧实体的所有组件，然后用API，复制旧实体的所有组件给新实体，复制组件会包括复制组件的属性值
		// Copy components (except IDComponent and TagComponent)
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string & name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<IDComponent>(uuid); // 使用实参uuid，不创建新的
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		m_EntityMap[uuid] = entity;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		OnPhysics2DStart();

		// 脚本初始化Scripting
		{
			ScriptEngine::OnRuntimeStart(this);
			// Instantiate all script entities
			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				ScriptEngine::OnCreateEntity(entity);
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
		// 脚本影响pyhsic然后渲染，当前帧得到结果
		// 迭代速度：使用更少的迭代可以提高性能，但准确性会受到影响。使用更多迭代会降低性能但会提高模拟质量
		// Physics
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

		// Retrieve transform from Box2D
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			// 获取物理模拟计算后的主体
			b2Body* body = (b2Body*)rb2d.RuntimeBody;

			// 将计算后的值赋予实体
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
					Entity entity = { e, this };
					ScriptEngine::OnUpdateEntity(entity, ts);
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
			// Script - Physic - Render顺序
			Physics(ts);
		}

		// 获取主摄像头
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		glm::vec3 cameraPosition;
		glm::vec3 cameraDirection;
		{
			Entity mainCameraEntity = GetPrimaryCameraEntity();
			if (mainCameraEntity)
			{
				auto& transform = mainCameraEntity.GetComponent<TransformComponent>();
				auto& camera = mainCameraEntity.GetComponent<CameraComponent>();
				mainCamera = &camera.Camera;
				cameraTransform = transform.GetTransform();
				cameraPosition = transform.Translation;
				cameraDirection = glm::rotate(glm::quat(transform.Rotation), glm::vec3(0.0f, 0.0f, -1.0f));
			}
		}

		// 主摄像头渲染场景
		if (mainCamera)
			RenderScene(*mainCamera, cameraTransform, cameraPosition, cameraDirection);
	}

	void Scene::OnUpdateSimulation(Timestep ts, EditorCamera& camera)
	{
		if (!m_IsPaused || m_StepFrames-- > 0)
			Physics(ts);
		// Render
		RenderScene(camera, camera.GetViewMatrix(), camera.GetPosition(), camera.GetForwardDirection());
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
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

	Entity Scene::DuplicateEntity(Entity entity)
	{
		// Copy name because we're going to modify component data structure
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const TagComponent& tc = view.get<TagComponent>(entity);
			if (tc.Tag == name)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		if (m_EntityMap.find(uuid) != m_EntityMap.end())
			return { m_EntityMap.at(uuid), this };

		return {};
	}

	void Scene::OnPhysics2DStart()
	{
		// 创建一个物体世界/环境
		// 重力加速度向下
		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });

		// 为当前场景所有具有物理组件的实体创建b2Body
		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			// 主体定义用来指定动态类型和参数
			b2BodyDef bodyDef;
			bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			// 绕着z轴旋转
			bodyDef.angle = transform.Rotation.z;

			// 由b2BodyDef创建主体
			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			// 是否固定旋转
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
				// 定义盒子包围盒
				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

				// 定义fixture，fixture包含定义的包围盒
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
				// 定义主体的fixture
				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

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

	// 摄像头渲染场景，摄像头，摄像头TRS，摄像头位置（translation），摄像头方向
	void Scene::RenderScene(Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& direction)
	{
		Renderer2D::BeginScene(camera, transform);
		Renderer3D::BeginScene(camera, transform, position, direction);
		RendererModel::BeginScene(camera, transform, position, direction);

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

		// Draw cube
		{
			auto view = m_Registry.view<TransformComponent, CubeRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, cube] = view.get<TransformComponent, CubeRendererComponent>(entity);
				Renderer3D::DrawCube(transform.GetTransform(), transform.GetNormalTransform(), cube, (int)entity);
			}
		}

		// Draw model
		{
			auto view = m_Registry.view<TransformComponent,ModelRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, model] = view.get<TransformComponent, ModelRendererComponent>(entity);
				RendererModel::DrawModel(transform.GetTransform(), transform.GetNormalTransform(), (int)entity);
			}
		}

		RendererModel::EndScene();

		Renderer2D::EndScene();
		Renderer3D::EndScene();
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		// 静态断言
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CubeRendererComponent>(Entity entity, CubeRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<ModelRendererComponent>(Entity entity, ModelRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{

	}

	template<>
	void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
	{
	}

}