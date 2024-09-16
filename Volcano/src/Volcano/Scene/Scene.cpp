#include "volpch.h"
#include "Scene.h"

#include "Components.h"
#include "ScriptableEntity.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Renderer/RendererAPI.h"
#include "Volcano/Renderer/Renderer2D.h"
#include "Volcano/Renderer/Renderer3D.h"
#include "Volcano/Renderer/RendererModel.h"
#include "Volcano/Renderer/RendererItem/Skybox.h"
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

	static Ref<UniformBuffer> s_DirectionalLightSpaceMatrixUniformBuffer;
	static Ref<UniformBuffer> s_PointLightSpaceMatrixUniformBuffer;
	static Ref<UniformBuffer> s_SpotLightSpaceMatrixUniformBuffer;

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
		s_DirectionalLightUniformBuffer            = UniformBuffer::Create((4 + 4 + 4 + 4) * sizeof(float), 2);
		s_PointLightUniformBuffer                  = UniformBuffer::Create((4 + 4 + 4 + 3 + 1 + 1 + 1) * sizeof(float), 3);
		s_SpotLightUniformBuffer                   = UniformBuffer::Create((4 + 4 + 4 + 4 + 3 + 1 + 1 + 1 + 1 + 1) * sizeof(float), 4);
		s_DirectionalLightSpaceMatrixUniformBuffer = UniformBuffer::Create(4 * 4 * sizeof(float), 8);
		s_PointLightSpaceMatrixUniformBuffer       = UniformBuffer::Create((4 * 4 * 6 + 1) * sizeof(float), 9);
		s_SpotLightSpaceMatrixUniformBuffer        = UniformBuffer::Create((4 * 4 + 1) * sizeof(float), 10);
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
	}

	void Scene::OnRenderRuntime(Timestep ts)
	{
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
	}

	void Scene::OnRenderSimulation(Timestep ts, EditorCamera& camera)
	{
		// Render
		RenderScene(camera, camera.GetViewMatrix(), camera.GetPosition(), camera.GetForwardDirection());
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
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
	
	Entity Scene::GetDirectionalLightEntity()
	{
		// 获取光源
		auto view = m_Registry.view<LightComponent>();
		for (auto entity : view)
		{
			auto light = view.get<LightComponent>(entity);
			if (light.Type == LightComponent::LightType::DirectionalLight)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::GetPointLightEntity()
	{
		// 获取光源
		auto view = m_Registry.view<LightComponent>();
		for (auto entity : view)
		{
			auto light = view.get<LightComponent>(entity);
			if (light.Type == LightComponent::LightType::PointLight)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::GetSpotLightEntity()
	{
		// 获取光源
		auto view = m_Registry.view<LightComponent>();
		for (auto entity : view)
		{
			auto light = view.get<LightComponent>(entity);
			if (light.Type == LightComponent::LightType::SpotLight)
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


		UpdateLight();

		Renderer3D::BeginScene(camera, transform, position, direction);
		// Draw cube
		{
			auto view = m_Registry.view<TransformComponent, CubeRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, cube] = view.get<TransformComponent, CubeRendererComponent>(entity);
				Renderer3D::DrawCube(transform.GetTransform(), transform.GetNormalTransform(), cube, (int)entity);
			}
		}
		Renderer3D::EndScene(m_RenderType);
		

		RendererModel::BeginScene(camera, transform, position, direction);
		// Draw model
		{
			auto view = m_Registry.view<TransformComponent,ModelRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, model] = view.get<TransformComponent, ModelRendererComponent>(entity);
				if(m_RenderType == RenderType::SHADOW_DIRECTIONALLIGHT)
					RendererModel::DrawModel(transform.GetTransform(), transform.GetNormalTransform(), model.ModelPath, (int)entity);
			}
		}
		RendererModel::EndScene(m_RenderType);

		// draw skybox as last
		
		if (m_RenderType == RenderType::NORMAL)
		{
			RendererAPI::SetDepthFunc(DepthFunc::LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
			Skybox::BeginScene(camera, transform);
			Skybox::DrawSkybox();
			Skybox::EndScene();
			RendererAPI::SetDepthFunc(DepthFunc::LESS); // set depth function back to default
		}

	}

	void Scene::UpdateLight()
	{
			Entity directionalLightEntity = GetDirectionalLightEntity();
			if (directionalLightEntity)
			{
				auto& transform = directionalLightEntity.GetComponent<TransformComponent>();
				auto& light = directionalLightEntity.GetComponent<LightComponent>();
				glm::vec3 direction = glm::rotate(glm::quat(transform.Rotation), glm::vec3(0.0f, 0.0f, -1.0f));
				s_DirectionalLightBuffer.direction = direction;
				s_DirectionalLightBuffer.ambient   = light.Ambient;
				s_DirectionalLightBuffer.diffuse   = light.Diffuse;
				s_DirectionalLightBuffer.specular  = light.Specular;
				s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.direction, sizeof(glm::vec3));
				s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.ambient,   sizeof(glm::vec3), 4 * sizeof(float));
				s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.diffuse,   sizeof(glm::vec3), (4 + 4) * sizeof(float));
				s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.specular,  sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));

				
				glm::mat4 lightProjection, lightView;
				glm::mat4 lightSpaceMatrix;
				float near_plane = -1.0f, far_plane = 170.5f;
				//lightProjection = glm::perspective(glm::radians(68.0f), 1.0f, 0.001f, 1000.0f);
				lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
				lightView = glm::inverse(transform.GetTransform());

				lightSpaceMatrix = lightProjection * lightView;
				s_DirectionalLightSpaceMatrixUniformBuffer->SetData(&lightSpaceMatrix, sizeof(glm::mat4));
			}
			else
			{
				s_DirectionalLightBuffer.direction = { 0, 0, 0 };
				s_DirectionalLightBuffer.ambient   = { 0, 0, 0 };
				s_DirectionalLightBuffer.diffuse   = { 0, 0, 0 };
				s_DirectionalLightBuffer.specular  = { 0, 0, 0 };
				s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.direction, sizeof(glm::vec3));
				s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.ambient,   sizeof(glm::vec3), 4 * sizeof(float));
				s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.diffuse,   sizeof(glm::vec3), (4 + 4) * sizeof(float));
				s_DirectionalLightUniformBuffer->SetData(&s_DirectionalLightBuffer.specular,  sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));

				glm::mat4 lightSpaceMatrix = glm::mat4(0);
				s_DirectionalLightSpaceMatrixUniformBuffer->SetData(&lightSpaceMatrix, sizeof(glm::mat4));
				/*
				glm::vec3 lightPos(-20.0f, 40.0f, -10.0f);
				glm::mat4 lightProjection, lightView;
				glm::mat4 lightSpaceMatrix;
				float near_plane = -1.0f, far_plane = 70.5f;
				lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
				glm::mat4 rotation = glm::toMat4(glm::quat(glm::vec3(glm::radians(-60.0f), glm::radians(-120.0f), 0.0f)));
				lightView = glm::inverse(glm::translate(glm::mat4(1.0f), lightPos) * rotation * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)));

				lightSpaceMatrix = lightProjection * lightView;
				s_LightSpaceMatrixUniformBuffer->SetData(&lightSpaceMatrix, sizeof(glm::mat4));
				*/
			}


			Entity pointLightEntity = GetPointLightEntity();
			if (pointLightEntity)
			{
				auto& transform = pointLightEntity.GetComponent<TransformComponent>();
				auto& light = pointLightEntity.GetComponent<LightComponent>();
		        s_PointLightBuffer.position  = transform.Translation;
		        s_PointLightBuffer.ambient   = light.Ambient;
		        s_PointLightBuffer.diffuse   = light.Diffuse;
		        s_PointLightBuffer.specular  = light.Specular;
		        s_PointLightBuffer.constant  = light.Constant;
		        s_PointLightBuffer.linear    = light.Linear;
		        s_PointLightBuffer.quadratic = light.Quadratic;
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.position,  sizeof(glm::vec3));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.ambient,   sizeof(glm::vec3), 4 * sizeof(float));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.diffuse,   sizeof(glm::vec3), (4 + 4) * sizeof(float));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.specular,  sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.constant,  sizeof(float),     (4 + 4 + 4 + 3) * sizeof(float));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.linear,    sizeof(float),     (4 + 4 + 4 + 3 + 1) * sizeof(float));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.quadratic, sizeof(float),     (4 + 4 + 4 + 3 + 1 + 1) * sizeof(float));


				glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
				const uint32_t SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
				float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
				float m_Near = 1.0f;
				float m_Far = 25.0f;
				glm::mat4 shadowProj = glm::perspective(90.0f, aspect, m_Near, m_Far);
				std::vector<glm::mat4> shadowTransforms;
				shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0, -1.0,  0.0)));
				
				s_PointLightSpaceMatrixUniformBuffer->SetData(&shadowTransforms[0], 6 * 4 * 4 * sizeof(float));
				s_PointLightSpaceMatrixUniformBuffer->SetData(&m_Far, sizeof(float), 6 * 4 * 4 * sizeof(float));
			}
			else
			{
		        s_PointLightBuffer.position  = { 0, 0, 0 };
		        s_PointLightBuffer.ambient   = { 0, 0, 0 };
		        s_PointLightBuffer.diffuse   = { 0, 0, 0 };
		        s_PointLightBuffer.specular  = { 0, 0, 0 };
		        s_PointLightBuffer.constant  = 0;
		        s_PointLightBuffer.linear    = 0;
		        s_PointLightBuffer.quadratic = 0;
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.position,  sizeof(glm::vec3));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.ambient,   sizeof(glm::vec3), 4 * sizeof(float));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.diffuse,   sizeof(glm::vec3), (4 + 4) * sizeof(float));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.specular,  sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.constant,  sizeof(float),     (4 + 4 + 4 + 3) * sizeof(float));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.linear,    sizeof(float),     (4 + 4 + 4 + 3 + 1) * sizeof(float));
		        s_PointLightUniformBuffer->SetData(&s_PointLightBuffer.quadratic, sizeof(float),     (4 + 4 + 4 + 3 + 1 + 1) * sizeof(float));

				glm::mat4 shadowTransform = glm::mat4(0);
				float m_Far = 0;
				for (uint32_t i = 0; i < 6; ++i)
					s_PointLightSpaceMatrixUniformBuffer->SetData(&shadowTransform, 4 * 4 * sizeof(float), i * 4 * 4 * sizeof(float));
				s_PointLightSpaceMatrixUniformBuffer->SetData(&m_Far, sizeof(float), 6 * 4 * 4 * sizeof(float));

			}


			Entity spotLightEntity = GetSpotLightEntity();
			if (spotLightEntity)
			{
				auto& transform = spotLightEntity.GetComponent<TransformComponent>();
				auto& light = spotLightEntity.GetComponent<LightComponent>();
		        s_SpotLightBuffer.position    = transform.Translation;
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
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.position,    sizeof(glm::vec3));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.direction,   sizeof(glm::vec3), 4 * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.ambient,     sizeof(glm::vec3), (4 + 4) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.diffuse,     sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.specular,    sizeof(glm::vec3), (4 + 4 + 4 + 4) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.constant,    sizeof(float),     (4 + 4 + 4 + 4 + 3) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.linear,      sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.quadratic,   sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.cutOff,      sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.outerCutOff, sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1 + 1) * sizeof(float));

				glm::mat4 lightProjection, lightView;
				glm::mat4 lightSpaceMatrix;
				float m_Near = 1.0f, m_Far = 25.0f;
				lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, m_Near, m_Far);
				lightView = glm::inverse(transform.GetTransform());

				lightSpaceMatrix = lightProjection * lightView;
				s_SpotLightSpaceMatrixUniformBuffer->SetData(&lightSpaceMatrix, sizeof(glm::mat4));
				s_SpotLightSpaceMatrixUniformBuffer->SetData(&m_Far, sizeof(float), 4 * 4 * sizeof(float));
			}
			else
			{
		        s_SpotLightBuffer.position    = { 0, 0, 0 };
		        s_SpotLightBuffer.direction   = { 0, 0, 0 };
		        s_SpotLightBuffer.ambient     = { 0, 0, 0 };
		        s_SpotLightBuffer.diffuse     = { 0, 0, 0 };
		        s_SpotLightBuffer.specular    = { 0, 0, 0 };
		        s_SpotLightBuffer.constant    = 0;
		        s_SpotLightBuffer.linear      = 0;
		        s_SpotLightBuffer.quadratic   = 0;
		        s_SpotLightBuffer.cutOff      = 0;
		        s_SpotLightBuffer.outerCutOff = 0;
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.position,    sizeof(glm::vec3));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.direction,   sizeof(glm::vec3), 4 * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.ambient,     sizeof(glm::vec3), (4 + 4) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.diffuse,     sizeof(glm::vec3), (4 + 4 + 4) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.specular,    sizeof(glm::vec3), (4 + 4 + 4 + 4) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.constant,    sizeof(float),     (4 + 4 + 4 + 4 + 3) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.linear,      sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.quadratic,   sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.cutOff,      sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1) * sizeof(float));
		        s_SpotLightUniformBuffer->SetData(&s_SpotLightBuffer.outerCutOff, sizeof(float),     (4 + 4 + 4 + 4 + 3 + 1 + 1 + 1 + 1) * sizeof(float));

			}
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
	void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& component)
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