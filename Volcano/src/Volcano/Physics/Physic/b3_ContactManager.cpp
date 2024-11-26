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

		// TODO_ERIN ���������嶼�кܶ�contactsʱ��ʹ�ù�ϣ��������Ǳ�ڵ�ƿ��(bottleneck)��
		// contacts�Ƿ��Ѿ����ڣ�
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

		// �Ƿ��йؽڸ�����ײ���Ƿ�������һ��body�Ƕ�̬�ģ�
		if (bodyB->ShouldCollide(bodyA) == false)
		{
			return;
		}

		// ��������
		if (m_contactFilter && m_contactFilter->ShouldCollide(fixtureA, fixtureB) == false)
		{
			return;
		}

		// ���ù���(create)
		b3_Contact* c = b3_Contact::Create(fixtureA, indexA, fixtureB, indexB, m_allocator);
		if (c == nullptr)
		{
			return;
		}

		// Contact�������ܻύ��fixtures
		fixtureA = c->GetFixtureA();
		fixtureB = c->GetFixtureB();
		indexA = c->GetChildIndexA();
		indexB = c->GetChildIndexB();
		bodyA = fixtureA->GetBody();
		bodyB = fixtureB->GetBody();

		// ��contact����contact�б�
		c->m_prev = nullptr;
		c->m_next = m_contactList;
		if (m_contactList != nullptr)
		{
			m_contactList->m_prev = c;
		}
		m_contactList = c;

		// ���ӵ�islandͼ

		// ���ӵ�bodyA��contact�б�
		c->m_nodeA.contact = c;
		c->m_nodeA.other = bodyB;

		c->m_nodeA.prev = nullptr;
		c->m_nodeA.next = bodyA->m_contactList;
		if (bodyA->m_contactList != nullptr)
		{
			bodyA->m_contactList->prev = &c->m_nodeA;
		}
		bodyA->m_contactList = &c->m_nodeA;

		// ���ӵ�bodyB��contact�б�
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

			// ��contact�Ƿ��ѱ��Ϊ���ˣ�
			if (c->m_flags & b3_Contact::e_filterFlag)
			{
				// body�Ƿ�Ӧ����ײ��
				if (bodyB->ShouldCollide(bodyA) == false)
				{
					b3_Contact* cNotCollide = c;
					c = cNotCollide->GetNext();
					Destroy(cNotCollide);
					continue;
				}

				if (m_contactFilter && m_contactFilter->ShouldCollide(fixtureA, fixtureB) == false)
				{
					// contact�����ˣ�����
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

			// ��������һ��body�ѻ������Ƕ�̬���˶�body
			if (activeA == false && activeB == false)
			{
				c = c->GetNext();
				continue;
			}

			int proxyIdA = fixtureA->m_proxies[indexA].proxyId;
			int proxyIdB = fixtureB->m_proxies[indexB].proxyId;
			bool overlap = m_broadPhase.TestOverlap(proxyIdA, proxyIdB);

			// �����������������broad-phase�׶β����ص���contacts��
			if (overlap == false)
			{
				b3_Contact* cNotCollide = c;
				c = cNotCollide->GetNext();
				Destroy(cNotCollide);
				continue;
			}

			// ������ײ��contact��Ȼ���ڣ�ֱ��broad-phase������ײ������
			c->Update(m_contactListener);
			c = c->GetNext();
		}
	}

}