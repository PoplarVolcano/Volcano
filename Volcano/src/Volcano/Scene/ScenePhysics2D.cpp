#include "volpch.h"
#include "Scene.h"

#include "Volcano/Scene/Entity.h"
#include "Volcano/Core/Time.h"

// Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

#include "Volcano/Utils/Physics2DUtils.h"

namespace Volcano
{

	void Scene::OnPhysics()
	{
		// Physics2D
		{
			// 脚本影响pyhsic然后渲染，当前帧得到结果
			// 迭代速度：使用更少的迭代可以提高性能，但准确性会受到影响。使用更多迭代会降低性能但会提高模拟质量
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;
			m_PhysicsWorld->Step(Time::GetDeltaTime(), velocityIterations, positionIterations);

			// Retrieve transform from Box2D
			auto view = m_Registry.view<Rigidbody2DComponent>();
			for (auto e : view)
			{
				//Entity entity = { e, this };
				auto& transform = m_EntityEnttMap[e]->GetComponent<TransformComponent>();
				auto& rb2d = m_EntityEnttMap[e]->GetComponent<Rigidbody2DComponent>();

				// 获取物理模拟计算后的body
				b2Body* body = (b2Body*)rb2d.RuntimeBody;

				// 将计算后的值赋予实体
				const auto& position = body->GetPosition();
				transform.translation.x = position.x;
				transform.translation.y = position.y;
				//transform.rotation.z = body->GetAngle();
				transform.SetRotation(glm::angleAxis(body->GetAngle(), glm::vec3(0.0f, 0.0f, 1.0f)));
			}
		}

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
			Ref<Entity> entity = m_EntityEnttMap[e];
			auto& transform = entity->GetComponent<TransformComponent>();
			auto& rb2d = entity->GetComponent<Rigidbody2DComponent>();

			// 主体定义用来指定动态类型和参数
			b2BodyDef bodyDef;
			bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.type);
			bodyDef.position.Set(transform.translation.x, transform.translation.y);
			// 绕着z轴旋转 
			//bodyDef.angle = transform.rotation.z;
			bodyDef.angle = glm::radians(transform.inspectorEulerHint.z);

			// 由b2BodyDef创建主体
			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			// 是否固定旋转
			body->SetFixedRotation(rb2d.fixedRotation);
			rb2d.RuntimeBody = body;

			if (entity->HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity->GetComponent<BoxCollider2DComponent>();
				if (!bc2d.enabled)
					continue;
				// 定义盒子包围盒
				b2PolygonShape boxShape;
				boxShape.SetAsBox(bc2d.size.x * transform.scale.x, bc2d.size.y * transform.scale.y);

				// 定义fixture，fixture包含定义的包围盒
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.density;
				fixtureDef.friction = bc2d.friction;
				fixtureDef.restitution = bc2d.restitution;
				fixtureDef.restitutionThreshold = bc2d.restitutionThreshold;
				// 定义主体的fixture
				body->CreateFixture(&fixtureDef);
			}

			if (entity->HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity->GetComponent<CircleCollider2DComponent>();

				if (!cc2d.enabled)
					continue;

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2d.offset.x, cc2d.offset.y);
				circleShape.m_radius = transform.scale.x * cc2d.radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2d.density;
				fixtureDef.friction = cc2d.friction;
				fixtureDef.restitution = cc2d.restitution;
				fixtureDef.restitutionThreshold = cc2d.restitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::OnPhysics2DStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}


}