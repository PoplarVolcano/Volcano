#include "volpch.h"

#include "b3_Fixture.h"
#include "b3_BlockAllocator.h"
#include "b3_BroadPhase.h"
#include "Collision/b3_SphereShape.h"
#include "b3_Contact.h"
#include "b3_World.h"

namespace Volcano {

	inline void b3_Fixture::Refilter()
	{
		if (m_body == nullptr)
		{
			return;
		}

		//标记关联contacts以进行过滤。
		b3_ContactEdge* edge = m_body->GetContactList();
		while (edge)
		{
			b3_Contact* contact = edge->contact;
			b3_Fixture* fixtureA = contact->GetFixtureA();
			b3_Fixture* fixtureB = contact->GetFixtureB();
			if (fixtureA == this || fixtureB == this)
			{
				contact->FlagForFiltering();
			}

			edge = edge->next;
		}

		b3_World* world = m_body->GetWorld();

		if (world == nullptr)
		{
			return;
		}

		// 对代理执行TouchProxy，以便创建新的pair
		b3_BroadPhase* broadPhase = &world->m_contactManager.m_broadPhase;
		for (int i = 0; i < m_proxyCount; ++i)
		{
			broadPhase->TouchProxy(m_proxies[i].proxyId);
		}
	}

	b3_Fixture::b3_Fixture()
	{
		m_density = 0.0f;
		m_body = nullptr;
		m_next = nullptr;
		m_shape = nullptr;
		m_proxies = nullptr;
		m_proxyCount = 0;
	}

	void b3_Fixture::Create(b3_BlockAllocator* allocator, b3_Body* body, const b3_FixtureDef* def)
	{
		m_userData = def->userData;
		m_density = def->density;
		m_friction = def->friction;
		m_restitution = def->restitution;
		m_restitutionThreshold = def->restitutionThreshold;
		m_body = body;
		m_next = nullptr;
		m_filter = def->filter;
		m_isSensor = def->isSensor;
		m_shape = def->shape->Clone(allocator);

		// 预留代理空间  Reserve proxy space
		int childCount = m_shape->GetChildCount();
		m_proxies = (b3_FixtureProxy*)allocator->Allocate(childCount * sizeof(b3_FixtureProxy));
		for (int i = 0; i < childCount; ++i)
		{
			m_proxies[i].fixture = nullptr;
			m_proxies[i].proxyId = b3_BroadPhase::e_nullProxy;
		}
		m_proxyCount = 0;
	}

	void b3_Fixture::Destroy(b3_BlockAllocator* allocator)
	{
		// The proxies must be destroyed before calling this.
		assert(m_proxyCount == 0);

		// Free the proxy array.
		int childCount = m_shape->GetChildCount();
		allocator->Free(m_proxies, childCount * sizeof(b3_FixtureProxy));
		m_proxies = nullptr;

		// Free the child shape.
		switch (m_shape->m_type)
		{
		case b3_Shape::e_sphere:
		{
			b3_SphereShape* s = (b3_SphereShape*)m_shape;
			s->~b3_SphereShape();
			allocator->Free(s, sizeof(b3_SphereShape));
		}
		break;
		/*
		case b3_Shape::b3Shape_Edge:
		{
			b3_EdgeShape* s = (b3_EdgeShape*)m_shape;
			s->~b3_EdgeShape();
			allocator->Free(s, sizeof(b3_EdgeShape));
		}
		break;

		case b3_Shape::b3Shape_Polygon:
		{
			b3_PolygonShape* s = (b3_PolygonShape*)m_shape;
			s->~b3_PolygonShape();
			allocator->Free(s, sizeof(b3_PolygonShape));
		}
		break;

		case b3_Shape::b3Shape_Chain:
		{
			b3_ChainShape* s = (b3_ChainShape*)m_shape;
			s->~b3_ChainShape();
			allocator->Free(s, sizeof(b3_ChainShape));
		}
		break;
		*/
		default:
			assert(false);
			break;
		}

		m_shape = nullptr;
	}

	void b3_Fixture::CreateProxies(b3_BroadPhase* broadPhase, const b3_Transform& transform)
	{
		assert(m_proxyCount == 0);

		// 在broad-phase中创建代理 Create proxies in the broad-phase.
		m_proxyCount = m_shape->GetChildCount();

		for (int i = 0; i < m_proxyCount; ++i)
		{
			b3_FixtureProxy* proxy = m_proxies + i;
			m_shape->ComputeAABB(&proxy->aabb, transform, i);
			proxy->proxyId = broadPhase->CreateProxy(proxy->aabb, proxy);
			proxy->fixture = this;
			proxy->childIndex = i;
		}
	}

	void b3_Fixture::DestroyProxies(b3_BroadPhase* broadPhase)
	{
		// Destroy proxies in the broad-phase.
		for (int i = 0; i < m_proxyCount; ++i)
		{
			b3_FixtureProxy* proxy = m_proxies + i;
			broadPhase->DestroyProxy(proxy->proxyId);
			proxy->proxyId = b3_BroadPhase::e_nullProxy;
		}

		m_proxyCount = 0;
	}

	void b3_Fixture::Synchronize(b3_BroadPhase* broadPhase, const b3_Transform& transform1, const b3_Transform& transform2)
	{
		if (m_proxyCount == 0)
		{
			return;
		}

		for (int i = 0; i < m_proxyCount; ++i)
		{
			b3_FixtureProxy* proxy = m_proxies + i;

			// 计算一个覆盖swept shape的AABB（可能会错过一些旋转效果）
			b3_AABB aabb1, aabb2;
			m_shape->ComputeAABB(&aabb1, transform1, proxy->childIndex);
			m_shape->ComputeAABB(&aabb2, transform2, proxy->childIndex);

			proxy->aabb.Combine(aabb1, aabb2);

			glm::vec3 displacement = aabb2.GetCenter() - aabb1.GetCenter();

			broadPhase->MoveProxy(proxy->proxyId, proxy->aabb, displacement);
		}
	}

}