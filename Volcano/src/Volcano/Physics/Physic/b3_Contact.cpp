#include "volpch.h"

#include "b3_Contact.h"
#include "b3_WorldCallbacks.h"
#include "Volcano/Physics/physic/Collision/b3_SphereContact.h"
#include "Volcano/Physics/physic/Collision/b3_BoxSphereContact.h"
#include "Volcano/Physics/physic/Collision/b3_BoxContact.h"

namespace Volcano {

	b3_ContactRegister b3_Contact::s_registers[b3_Shape::e_typeCount][b3_Shape::e_typeCount];
	bool b3_Contact::s_initialized = false;

	void b3_Contact::AddType(b3_ContactCreateFcn* createFcn, b3_ContactDestroyFcn* destroyFcn, b3_Shape::Type typeA, b3_Shape::Type typeB)
	{
		assert(0 <= typeA && typeA < b3_Shape::e_typeCount);
		assert(0 <= typeB && typeB < b3_Shape::e_typeCount);

		s_registers[typeA][typeB].createFcn = createFcn;
		s_registers[typeA][typeB].destroyFcn = destroyFcn;
		s_registers[typeA][typeB].primary = true;

		if (typeA != typeB)
		{
			s_registers[typeB][typeA].createFcn = createFcn;
			s_registers[typeB][typeA].destroyFcn = destroyFcn;
			s_registers[typeB][typeA].primary = false;
		}

	}
	void b3_Contact::InitializeRegisters()
	{
		AddType(b3_SphereContact::Create, b3_SphereContact::Destroy, b3_Shape::e_sphere, b3_Shape::e_sphere);
		AddType(b3_BoxSphereContact::Create, b3_BoxSphereContact::Destroy, b3_Shape::e_box, b3_Shape::e_sphere);
		AddType(b3_BoxContact::Create, b3_BoxContact::Destroy, b3_Shape::e_box, b3_Shape::e_box);
		//AddType(b3_EdgeAndCircleContact::Create, b3_EdgeAndCircleContact::Destroy, b3_Shape::e_edge, b3_Shape::e_circle);
		//AddType(b3_EdgeAndPolygonContact::Create, b3_EdgeAndPolygonContact::Destroy, b3_Shape::e_edge, b3_Shape::e_polygon);
		//AddType(b3_ChainAndCircleContact::Create, b3_ChainAndCircleContact::Destroy, b3_Shape::e_chain, b3_Shape::e_circle);
		//AddType(b3_ChainAndPolygonContact::Create, b3_ChainAndPolygonContact::Destroy, b3_Shape::e_chain, b3_Shape::e_polygon);
	}
	b3_Contact* b3_Contact::Create(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB, b3_BlockAllocator* allocator)
	{
		if (s_initialized == false)
		{
			InitializeRegisters();
			s_initialized = true;
		}

		b3_Shape::Type type1 = fixtureA->GetType();
		b3_Shape::Type type2 = fixtureB->GetType();

		assert(0 <= type1 && type1 < b3_Shape::e_typeCount);
		assert(0 <= type2 && type2 < b3_Shape::e_typeCount);

		b3_ContactCreateFcn* createFcn = s_registers[type1][type2].createFcn;
		if (createFcn)
		{
			if (s_registers[type1][type2].primary)
			{
				return createFcn(fixtureA, indexA, fixtureB, indexB, allocator);
			}
			else
			{
				return createFcn(fixtureB, indexB, fixtureA, indexA, allocator);
			}
		}
		else
		{
			return nullptr;
		}
	}
	void b3_Contact::Destroy(b3_Contact* contact, b3_BlockAllocator* allocator)
	{
		assert(s_initialized == true);

		b3_Fixture* fixtureA = contact->m_fixtureA;
		b3_Fixture* fixtureB = contact->m_fixtureB;

		if (contact->m_manifold.pointCount > 0 && fixtureA->IsSensor() == false && fixtureB->IsSensor() == false)
		{
			// contact碰撞且两个body的fixture不是传感器，唤醒两个body
			fixtureA->GetBody()->SetAwake(true);
			fixtureB->GetBody()->SetAwake(true);
		}

		b3_Shape::Type typeA = fixtureA->GetType();
		b3_Shape::Type typeB = fixtureB->GetType();

		assert(0 <= typeA && typeA < b3_Shape::e_typeCount);
		assert(0 <= typeB && typeB < b3_Shape::e_typeCount);

		b3_ContactDestroyFcn* destroyFcn = s_registers[typeA][typeB].destroyFcn;
		destroyFcn(contact, allocator);
	}
	b3_Contact::b3_Contact(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB)
	{
		m_flags = e_enabledFlag;

		m_fixtureA = fixtureA;
		m_fixtureB = fixtureB;

		m_indexA = indexA;
		m_indexB = indexB;

		m_manifold.pointCount = 0;

		m_prev = nullptr;
		m_next = nullptr;

		m_nodeA.contact = nullptr;
		m_nodeA.prev = nullptr;
		m_nodeA.next = nullptr;
		m_nodeA.other = nullptr;

		m_nodeB.contact = nullptr;
		m_nodeB.prev = nullptr;
		m_nodeB.next = nullptr;
		m_nodeB.other = nullptr;

		m_toiCount = 0;

		m_friction = b3_MixFriction(m_fixtureA->m_friction, m_fixtureB->m_friction);
		m_restitution = b3_MixRestitution(m_fixtureA->m_restitution, m_fixtureB->m_restitution);
		m_restitutionThreshold = b3_MixRestitutionThreshold(m_fixtureA->m_restitutionThreshold, m_fixtureB->m_restitutionThreshold);

		m_tangentSpeed = 0.0f;
	}

	
	void b3_Contact::Update(b3_ContactListener* listener)
	{
		// 上一帧的交点
		b3_Manifold oldManifold = m_manifold;

		// Re-enable this contact.
		m_flags |= e_enabledFlag;

		bool touching = false;  // 碰撞检测后是否发生碰撞
		bool wasTouching = (m_flags & e_touchingFlag) == e_touchingFlag;  // 碰撞检测前是否发生碰撞

		bool sensorA = m_fixtureA->IsSensor();
		bool sensorB = m_fixtureB->IsSensor();
		bool sensor = sensorA || sensorB;

		b3_Body* bodyA = m_fixtureA->GetBody();
		b3_Body* bodyB = m_fixtureB->GetBody();
		const b3_Transform& transformA = bodyA->GetTransform();
		const b3_Transform& transformB = bodyB->GetTransform();

		// Is this contact a sensor?
		if (sensor)
		{
			// 对有传感器的contact进行通用碰撞测试
			const b3_Shape* shapeA = m_fixtureA->GetShape();
			const b3_Shape* shapeB = m_fixtureB->GetShape();
			touching = b3_TestOverlap(shapeA, m_indexA, shapeB, m_indexB, transformA, transformB);

			// 传感器不产生manifolds
			m_manifold.pointCount = 0;
		}
		else
		{
			// 执行一次碰撞检测并将碰撞信息注入m_manifold
			Evaluate(&m_manifold, transformA, transformB);
			touching = m_manifold.pointCount > 0;

			// 将旧contactID与新contactID匹配，并复制存储的冲量以热启动求解器(solver)。
			for (int i = 0; i < m_manifold.pointCount; ++i)
			{
				b3_ManifoldPoint* mp2 = m_manifold.points + i;
				mp2->normalImpulse = 0.0f;
				mp2->tangentImpulse = 0.0f;
				b3_ContactID id2 = mp2->id;

				for (int j = 0; j < oldManifold.pointCount; ++j)
				{
					b3_ManifoldPoint* mp1 = oldManifold.points + j;

					if (mp1->id.key == id2.key)
					{
						mp2->normalImpulse = mp1->normalImpulse;
						mp2->tangentImpulse = mp1->tangentImpulse;
						break;
					}
				}
			}

			// 碰撞检测前后碰撞状态不同都会触发一次唤醒
			if (touching != wasTouching)
			{
				bodyA->SetAwake(true);
				bodyB->SetAwake(true);
			}
		}

		if (touching)
		{
			m_flags |= e_touchingFlag;
		}
		else
		{
			m_flags &= ~e_touchingFlag;
		}

		// 碰撞检测前未碰撞，碰撞检测后碰撞，listener不为空，调用BeginContact
		if (wasTouching == false && touching == true && listener)
		{
			listener->BeginContact(this);
		}

		// 碰撞检测前碰撞，碰撞检测后未碰撞，listener不为空，调用EndContact
		if (wasTouching == true && touching == false && listener)
		{
			listener->EndContact(this);
		}

		// 碰撞检测后碰撞，不是传感器，listener不为空，调用PreSolve
		if (sensor == false && touching && listener)
		{
			listener->PreSolve(this, &oldManifold);
		}
	}
	
}