#include "volpch.h"

#include "b3_Body.h"
#include "b3_World.h"
#include "b3_BroadPhase.h"
#include "b3_ContactManager.h"
#include "b3_Fixture.h"
#include "b3_Contact.h"
#include "b3_Joint.h"

namespace Volcano {
	void b3_Body::SetFixedRotation(bool flag)
	{
		bool status = (m_flags & e_fixedRotationFlag) == e_fixedRotationFlag;
		if (status == flag)
			return;

		if (flag)
			m_flags |= e_fixedRotationFlag;
		else
			m_flags &= ~e_fixedRotationFlag;

		m_angularVelocity = { 0.0f, 0.0f, 0.0f };

		ResetMassData();
	}

	void b3_Body::SetTransform(const glm::vec3& position, glm::vec3 rotation)
	{
		assert(m_world->IsLocked() == false);
		if (m_world->IsLocked() == true)
		{
			return;
		}

		m_transform.rotation.Set(rotation);
		m_transform.position = position;

		m_sweep.center = b3_Multiply(m_transform, m_sweep.localCenter);
		m_sweep.rotation = rotation;

		m_sweep.center0 = m_sweep.center;
		m_sweep.rotation0 = rotation;

		b3_BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
		for (b3_Fixture* f = m_fixtureList; f; f = f->m_next)
		{
			f->Synchronize(broadPhase, m_transform, m_transform);
		}

		// Check for new contacts the next step
		m_world->m_newContacts = true;
	}

	b3_Body::b3_Body(const b3_BodyDef* bd, b3_World* world)
	{
		/*
		assert(bd->position.IsValid());
		assert(bd->linearVelocity.IsValid());
		assert(b3_IsValid(bd->angle));
		assert(b3_IsValid(bd->angularVelocity));
		assert(b3_IsValid(bd->angularDamping) && bd->angularDamping >= 0.0f);
		assert(b3_IsValid(bd->linearDamping) && bd->linearDamping >= 0.0f);
		*/
		m_flags = 0;

		if (bd->bullet)
		{
			m_flags |= e_bulletFlag;
		}
		if (bd->fixedRotation)
		{
			m_flags |= e_fixedRotationFlag;
		}
		if (bd->allowSleep)
		{
			m_flags |= e_autoSleepFlag;
		}
		if (bd->awake && bd->type != b3_BodyType::e_staticBody)
		{
			m_flags |= e_awakeFlag;
		}
		if (bd->enabled)
		{
			m_flags |= e_enabledFlag;
		}

		m_world = world;

		m_prev = nullptr;
		m_next = nullptr;

		m_linearVelocity = bd->linearVelocity;
		m_angularVelocity = bd->angularVelocity;

		m_transform.position = bd->position;
		m_transform.rotation.Set(bd->rotation);

		m_sweep.localCenter = { 0.0f, 0.0f, 0.0f };
		m_sweep.center0 = m_transform.position;
		m_sweep.center = m_transform.position;
		m_sweep.rotation0 = bd->rotation;
		m_sweep.rotation = bd->rotation;
		m_sweep.alpha0 = 0.0f;

		m_jointList = nullptr;
		m_contactList = nullptr;
		m_linearDamping = bd->linearDamping;
		m_angularDamping = bd->angularDamping;
		m_gravityScale = bd->gravityScale;

		m_force = { 0.0f, 0.0f, 0.0f };
		m_torque = { 0.0f, 0.0f, 0.0f };
		m_sleepTime = 0.0f;
		m_type = bd->type;

		m_mass = 0.0f;
		m_invMass = 0.0f;
		m_I = glm::mat3(0.0f);
		m_invI = glm::mat3(0.0f);
		m_userData = bd->userData;
		m_fixtureList = nullptr;
		m_fixtureCount = 0;
	}

	b3_Body::~b3_Body()
	{
		// shapes and joints are destroyed in b3_World::Destroy
	}

	b3_Fixture* b3_Body::CreateFixture(const b3_FixtureDef* def)
	{
		assert(m_world->IsLocked() == false); 
		if (m_world->IsLocked() == true)
		{
			return nullptr;
		}

		b3_BlockAllocator* allocator = &m_world->m_blockAllocator;

		void* memory = allocator->Allocate(sizeof(b3_Fixture));
		b3_Fixture* fixture = new (memory) b3_Fixture;
		fixture->Create(allocator, this, def);

		if (m_flags & e_enabledFlag)
		{
			b3_BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
			fixture->CreateProxies(broadPhase, m_transform);
		}

		fixture->m_next = m_fixtureList;
		m_fixtureList = fixture;
		++m_fixtureCount;

		fixture->m_body = this;

		// Adjust mass properties if needed.
		if (fixture->m_density > 0.0f)
		{
			ResetMassData();
		}

		// ��world֪�����������µ�fixture���⽫��������һʱ�䲽��ʼʱ������contacts��
		m_world->m_newContacts = true;

		return fixture;
	}

	b3_Fixture* b3_Body::CreateFixture(const b3_Shape* shape, float density)
	{
		b3_FixtureDef def;
		def.shape = shape;
		def.density = density;

		return CreateFixture(&def);

	}

	void b3_Body::DestroyFixture(b3_Fixture* fixture)
	{
		if (fixture == NULL)
		{
			return;
		}

		assert(m_world->IsLocked() == false);
		if (m_world->IsLocked() == true)
		{
			return;
		}

		assert(fixture->m_body == this);

		// �Ӹ�body�ĵ�������ɾ��fixture��
		assert(m_fixtureCount > 0);
		b3_Fixture** node = &m_fixtureList;
		bool found = false;
		while (*node != nullptr)
		{
			if (*node == fixture)
			{
				*node = fixture->m_next;
				found = true;
				break;
			}

			node = &(*node)->m_next;
		}

		// ����ͼɾ��δ���ŵ���body��shape��
		assert(found);

		const float density = fixture->m_density;

		// ������fixture���(associated)������contacts��
		b3_ContactEdge* edge = m_contactList;
		while (edge)
		{
			b3_Contact* c = edge->contact;
			edge = edge->next;

			b3_Fixture* fixtureA = c->GetFixtureA();
			b3_Fixture* fixtureB = c->GetFixtureB();

			if (fixture == fixtureA || fixture == fixtureB)
			{
				// �������contact������Ӹ�body��contact�б���ɾ����
				m_world->m_contactManager.Destroy(c);
			}
		}

		b3_BlockAllocator* allocator = &m_world->m_blockAllocator;

		if (m_flags & e_enabledFlag)
		{
			// ��broadPhase������fixture��صĴ���
			b3_BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
			fixture->DestroyProxies(broadPhase);
		}

		fixture->m_body = nullptr;
		fixture->m_next = nullptr;
		fixture->Destroy(allocator);
		fixture->~b3_Fixture();
		allocator->Free(fixture, sizeof(b3_Fixture));

		--m_fixtureCount;

		// Reset the mass data
		if (density > 0.0f)
		{
			ResetMassData();
		}

	}

	void b3_Body::SetMassData(const b3_MassData* massData)
	{
		assert(m_world->IsLocked() == false);
		if (m_world->IsLocked() == true)
		{
			return;
		}

		if (m_type != b3_BodyType::e_dynamicBody)
		{
			return;
		}

		m_invMass = 0.0f;
		m_I = glm::mat3(0.0f);
		m_invI = glm::mat3(0.0f);

		m_mass = massData->mass;
		if (m_mass <= 0.0f)
		{
			m_mass = 1.0f;
		}

		m_invMass = 1.0f / m_mass;

		//if (massData->I.x > 0.0f && massData->I.y > 0.0f && massData->I.z > 0.0f && (m_flags & b3_Body::e_fixedRotationFlag) == 0)
		if ((m_flags & b3_Body::e_fixedRotationFlag) == 0)
		{
			// ��ȡ���������Ĵ��Ĺ�������
			m_I = ComputeInertia(massData->I, m_mass, -massData->center);
			m_invI = glm::inverse(m_I);
		}

		// �ƶ�����
		glm::vec3 oldCenter = m_sweep.center;
		m_sweep.localCenter = massData->center;
		m_sweep.center0 = m_sweep.center = b3_Multiply(m_transform, m_sweep.localCenter);

		// ���������ٶ�
		m_linearVelocity += glm::cross(m_angularVelocity, m_sweep.center - oldCenter);
	}

	void b3_Body::ResetMassData()
	{
		// ����shapes�����������ݡ�ÿ��shapes�����Լ����ܶȡ�
		m_mass = 0.0f;
		m_invMass = 0.0f;
		m_I = glm::mat3(0.0f);
		m_invI = glm::mat3(0.0f);
		m_sweep.localCenter = { 0.0f, 0.0f, 0.0f };

		// ��̬���˶�body��������Ϊ�㡣
		if (m_type == b3_BodyType::e_staticBody || m_type == b3_BodyType::e_kinematicBody)
		{
			m_sweep.center0 = m_transform.position;
			m_sweep.center = m_transform.position;
			m_sweep.rotation0 = m_sweep.rotation;
			return;
		}

		assert(m_type == b3_BodyType::e_dynamicBody);

		// ������fixtures�ۻ�����
		// ��ϸ���: �ȼ������ϸ�������ĵ�. �ټ����ÿ���Ӹ������Լ��ռ�����ϵ�µĹ�������,
		//   ͨ��������������ת��ƽ���ᶨ��,�Ϳ�������Ӹ�������ϸ��������ϵ��������ĵ�Ĺ�������. 
		//   �������Ӹ���Ĺ������������������,�õ���ϸ���Ĺ�������.
		glm::vec3 localCenter = { 0.0f, 0.0f, 0.0f };
		std::vector<b3_MassData> I;
		for (b3_Fixture* f = m_fixtureList; f; f = f->m_next)
		{
			if (f->m_density == 0.0f)
			{
				continue;
			}

			b3_MassData massData;
			f->GetMassData(&massData);
			m_mass += massData.mass;
			localCenter += massData.mass * massData.center;  // ������������� center = (f1.mass * f1.center + f2.mass * f2.center) / (f1.mass + f2.mass)
			// �����������ĳ��ĵ�ת������Ϊÿ���������ĳ���ת������
			// GetMassData�л�ȡ��ת������Ĭ����0��Ϊ�ᣬ�õ���0��Ϊ��ķ��������ת������I = f1.I + f2.I
			I.push_back(massData);
		}

		// ��������
		if (m_mass > 0.0f)
		{
			m_invMass = 1.0f / m_mass;
			localCenter *= m_invMass;
		}

		if ((m_flags & e_fixedRotationFlag) == 0)
		{
			// ʹ����������Ϊ���ġ�
			for (int i = 0; i < I.size(); i++)
				m_I += ComputeInertia(I[i].I, I[i].mass, localCenter - I[i].center);
			m_invI = glm::inverse(m_I);

		}
		else
		{
			m_I = glm::mat3(0.0f);
			m_invI = glm::mat3(0.0f);
		}

		// �ƶ�����
		glm::vec3 oldCenter = m_sweep.center;
		m_sweep.localCenter = localCenter;
		m_sweep.center0 = m_sweep.center = b3_Multiply(m_transform, m_sweep.localCenter);

		// ���������ٶȡ�
		m_linearVelocity += glm::cross(m_angularVelocity, m_sweep.center - oldCenter);
	}

	void b3_Body::SetType(b3_BodyType type)
	{
		assert(m_world->IsLocked() == false);
		if (m_world->IsLocked() == true)
		{
			return;
		}

		if (m_type == type)
		{
			return;
		}

		m_type = type;

		ResetMassData();

		if (m_type == b3_BodyType::e_staticBody)
		{
			m_linearVelocity = { 0.0f, 0.0f, 0.0f };
			m_angularVelocity = { 0.0f, 0.0f, 0.0f };
			m_sweep.rotation0 = m_sweep.rotation;
			m_sweep.center0 = m_sweep.center;
			m_flags &= ~e_awakeFlag;
			SynchronizeFixtures();
		}

		SetAwake(true);

		m_force = { 0.0f, 0.0f, 0.0f };
		m_torque = { 0.0f, 0.0f, 0.0f };

		// Delete the attached contacts.
		b3_ContactEdge* ce = m_contactList;
		while (ce)
		{
			b3_ContactEdge* ce0 = ce;
			ce = ce->next;
			m_world->m_contactManager.Destroy(ce0->contact);
		}
		m_contactList = nullptr;

		// �Ӵ�����(TouchProxy)���Ա㴴����contacts�����ʵ���ʱ��(appropriate)��
		b3_BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
		for (b3_Fixture* f = m_fixtureList; f; f = f->m_next)
		{
			int proxyCount = f->m_proxyCount;
			for (int i = 0; i < proxyCount; ++i)
			{
				broadPhase->TouchProxy(f->m_proxies[i].proxyId);
			}
		}
	}

	void b3_Body::SetEnabled(bool flag)
	{
		assert(m_world->IsLocked() == false);

		if (flag == IsEnabled())
		{
			return;
		}

		if (flag)
		{
			m_flags |= e_enabledFlag;

			// Create all proxies.
			b3_BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
			for (b3_Fixture* f = m_fixtureList; f; f = f->m_next)
			{
				f->CreateProxies(broadPhase, m_transform);
			}

			// Contacts����һ��ʱ�䲽��ʼʱ����
			m_world->m_newContacts = true;
		}
		else
		{
			m_flags &= ~e_enabledFlag;

			// Destroy all proxies.
			b3_BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
			for (b3_Fixture* f = m_fixtureList; f; f = f->m_next)
			{
				f->DestroyProxies(broadPhase);
			}

			// Destroy the attached contacts.
			b3_ContactEdge* ce = m_contactList;
			while (ce)
			{
				b3_ContactEdge* ce0 = ce;
				ce = ce->next;
				m_world->m_contactManager.Destroy(ce0->contact);
			}
			m_contactList = nullptr;
		}
	}

	void b3_Body::SynchronizeFixtures()
	{
		b3_BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;

		if (m_flags & b3_Body::e_awakeFlag)
		{
			b3_Transform transform1;
			transform1.rotation.Set(m_sweep.rotation0);
			transform1.position = m_sweep.center0 - b3_Multiply(transform1.rotation, m_sweep.localCenter);

			for (b3_Fixture* f = m_fixtureList; f; f = f->m_next)
			{
				f->Synchronize(broadPhase, transform1, m_transform);
			}
		}
		else
		{
			for (b3_Fixture* f = m_fixtureList; f; f = f->m_next)
			{
				f->Synchronize(broadPhase, m_transform, m_transform);
			}
		}
	}

	bool b3_Body::ShouldCollide(const b3_Body* other) const
	{
		// ����һ��bodyӦ���Ƕ�̬��
		if (m_type != b3_BodyType::e_dynamicBody && other->m_type != b3_BodyType::e_dynamicBody)
		{
			return false;
		}

		// �Ƿ��йؽ���ֹ��ײ
		for (b3_JointEdge* jn = m_jointList; jn; jn = jn->next)
		{
			if (jn->other == other)
			{
				if (jn->joint->m_collideConnected == false)
				{
					return false;
				}
			}
		}

		return true;
	}

}