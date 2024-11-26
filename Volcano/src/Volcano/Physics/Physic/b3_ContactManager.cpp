#include "volpch.h"

#include "b3_ContactManager.h"
#include "b3_Fixture.h"
#include "b3_Contact.h"
#include "b3_WorldCallbacks.h"

namespace Volcano{

	b3_ContactFilter b3_defaultFilter;
	b3_ContactListener b3_defaultListener;

	b3_ContactManager::b3_ContactManager()
	{
		m_contactList = nullptr;
		m_contactCount = 0;
		m_contactFilter = &b3_defaultFilter;
		m_contactListener = &b3_defaultListener;
		m_allocator = nullptr;
	}

	void b3_ContactManager::AddPair(void* proxyUserDataA, void* proxyUserDataB)
	{
		b3_FixtureProxy* proxyA = (b3_FixtureProxy*)proxyUserDataA;
		b3_FixtureProxy* proxyB = (b3_FixtureProxy*)proxyUserDataB;

		b3_Fixture* fixtureA = proxyA->fixture;
		b3_Fixture* fixtureB = proxyB->fixture;

		int indexA = proxyA->childIndex;
		int indexB = proxyB->childIndex;

		b3_Body* bodyA = fixtureA->GetBody();
		b3_Body* bodyB = fixtureB->GetBody();

		// Are the fixtures on the same body?
		if (bodyA == bodyB)
		{
			return;
		}

		// TODO_ERIN 当两个主体都有很多contacts时，使用哈希表来消除潜在的瓶颈(bottleneck)。
		// contacts是否已经存在？
		b3_ContactEdge* edge = bodyB->GetContactList();
		while (edge)
		{
			if (edge->other == bodyA)
			{
				b3_Fixture* fA = edge->contact->GetFixtureA();
				b3_Fixture* fB = edge->contact->GetFixtureB();
				int iA = edge->contact->GetChildIndexA();
				int iB = edge->contact->GetChildIndexB();

				if (fA == fixtureA && fB == fixtureB && iA == indexA && iB == indexB)
				{
					// A contact already exists.
					return;
				}

				if (fA == fixtureB && fB == fixtureA && iA == indexB && iB == indexA)
				{
					// A contact already exists.
					return;
				}
			}

			edge = edge->next;
		}

		// 是否有关节覆盖碰撞？是否至少有一个body是动态的？
		if (bodyB->ShouldCollide(bodyA) == false)
		{
			return;
		}

		// 检查过滤器
		if (m_contactFilter && m_contactFilter->ShouldCollide(fixtureA, fixtureB) == false)
		{
			return;
		}

		// 调用工厂(create)
		b3_Contact* c = b3_Contact::Create(fixtureA, indexA, fixtureB, indexB, m_allocator);
		if (c == nullptr)
		{
			return;
		}

		// Contact创建可能会交换fixtures
		fixtureA = c->GetFixtureA();
		fixtureB = c->GetFixtureB();
		indexA = c->GetChildIndexA();
		indexB = c->GetChildIndexB();
		bodyA = fixtureA->GetBody();
		bodyB = fixtureB->GetBody();

		// 将contact插入contact列表
		c->m_prev = nullptr;
		c->m_next = m_contactList;
		if (m_contactList != nullptr)
		{
			m_contactList->m_prev = c;
		}
		m_contactList = c;

		// 连接到island图

		// 连接到bodyA的contact列表
		c->m_nodeA.contact = c;
		c->m_nodeA.other = bodyB;

		c->m_nodeA.prev = nullptr;
		c->m_nodeA.next = bodyA->m_contactList;
		if (bodyA->m_contactList != nullptr)
		{
			bodyA->m_contactList->prev = &c->m_nodeA;
		}
		bodyA->m_contactList = &c->m_nodeA;

		// 连接到bodyB的contact列表
		c->m_nodeB.contact = c;
		c->m_nodeB.other = bodyA;

		c->m_nodeB.prev = nullptr;
		c->m_nodeB.next = bodyB->m_contactList;
		if (bodyB->m_contactList != nullptr)
		{
			bodyB->m_contactList->prev = &c->m_nodeB;
		}
		bodyB->m_contactList = &c->m_nodeB;

		++m_contactCount;
	}

	void b3_ContactManager::FindNewContacts()
	{
		m_broadPhase.UpdatePairs(this);
	}

	void b3_ContactManager::Destroy(b3_Contact* c)
	{
		b3_Fixture* fixtureA = c->GetFixtureA();
		b3_Fixture* fixtureB = c->GetFixtureB();
		b3_Body* bodyA = fixtureA->GetBody();
		b3_Body* bodyB = fixtureB->GetBody();

		if (m_contactListener && c->IsTouching())
		{
			m_contactListener->EndContact(c);
		}

		// Remove from the world.
		if (c->m_prev)
		{
			c->m_prev->m_next = c->m_next;
		}

		if (c->m_next)
		{
			c->m_next->m_prev = c->m_prev;
		}

		if (c == m_contactList)
		{
			m_contactList = c->m_next;
		}

		// Remove from body 1
		if (c->m_nodeA.prev)
		{
			c->m_nodeA.prev->next = c->m_nodeA.next;
		}

		if (c->m_nodeA.next)
		{
			c->m_nodeA.next->prev = c->m_nodeA.prev;
		}

		if (&c->m_nodeA == bodyA->m_contactList)
		{
			bodyA->m_contactList = c->m_nodeA.next;
		}

		// Remove from body 2
		if (c->m_nodeB.prev)
		{
			c->m_nodeB.prev->next = c->m_nodeB.next;
		}

		if (c->m_nodeB.next)
		{
			c->m_nodeB.next->prev = c->m_nodeB.prev;
		}

		if (&c->m_nodeB == bodyB->m_contactList)
		{
			bodyB->m_contactList = c->m_nodeB.next;
		}

		// Call the factory.
		b3_Contact::Destroy(c, m_allocator);
		--m_contactCount;
	}

	void b3_ContactManager::Collide()
	{
		// Update awake contacts.
		b3_Contact* c = m_contactList;
		while (c)
		{
			b3_Fixture* fixtureA = c->GetFixtureA();
			b3_Fixture* fixtureB = c->GetFixtureB();
			int indexA = c->GetChildIndexA();
			int indexB = c->GetChildIndexB();
			b3_Body* bodyA = fixtureA->GetBody();
			b3_Body* bodyB = fixtureB->GetBody();

			// 此contact是否已标记为过滤？
			if (c->m_flags & b3_Contact::e_filterFlag)
			{
				// body是否应该碰撞？
				if (bodyB->ShouldCollide(bodyA) == false)
				{
					b3_Contact* cNotCollide = c;
					c = cNotCollide->GetNext();
					Destroy(cNotCollide);
					continue;
				}

				if (m_contactFilter && m_contactFilter->ShouldCollide(fixtureA, fixtureB) == false)
				{
					// contact被过滤，销毁
					b3_Contact* cNotCollide = c;
					c = cNotCollide->GetNext();
					Destroy(cNotCollide);
					continue;
				}

				// Clear the filtering flag.
				c->m_flags &= ~b3_Contact::e_filterFlag;
			}

			bool activeA = bodyA->IsAwake() && bodyA->m_type != b3_BodyType::e_staticBody;
			bool activeB = bodyB->IsAwake() && bodyB->m_type != b3_BodyType::e_staticBody;

			// 必须至少一个body已唤醒且是动态或运动body
			if (activeA == false && activeB == false)
			{
				c = c->GetNext();
				continue;
			}

			int proxyIdA = fixtureA->m_proxies[indexA].proxyId;
			int proxyIdB = fixtureB->m_proxies[indexB].proxyId;
			bool overlap = m_broadPhase.TestOverlap(proxyIdA, proxyIdB);

			// 在这里，我们销毁了在broad-phase阶段不再重叠的contacts。
			if (overlap == false)
			{
				b3_Contact* cNotCollide = c;
				c = cNotCollide->GetNext();
				Destroy(cNotCollide);
				continue;
			}

			// 发生碰撞后contact依然存在，直到broad-phase不再碰撞被销毁
			c->Update(m_contactListener);
			c = c->GetNext();
		}
	}

}