#pragma once

#include "b3_Body.h"
#include "b3_Math.h"
#include "b3_TimeStep.h"

namespace Volcano {

	class b3_Contact;
	class b3_Joint;
	class b3_StackAllocator;
	class b3_ContactListener;
	struct b3_ContactVelocityConstraint;
	struct b3_Profile;

	// b3_Word�ڴ�����ײ��ʱ����Ҫ����ʱ�������ֶ�������b3_Island�н��м��㣬Ҳ����˵�������ڵ�������Ǽ��㡣
	class b3_Island
	{
	public:
		b3_Island(int bodyCapacity, int contactCapacity, int jointCapacity, b3_StackAllocator* allocator, b3_ContactListener* listener);
		~b3_Island();

		// ���body��contact��joint����
		void Clear()
		{
			m_bodyCount = 0;
			m_contactCount = 0;
			m_jointCount = 0;
		}

		// ���㣬���ط�������profile
		void Solve(b3_Profile* profile, const b3_TimeStep& step, const glm::vec3& gravity, bool allowSleep);

		void SolveTOI(const b3_TimeStep& subStep, int toiIndexA, int toiIndexB);

		void Add(b3_Body* body)
		{
			assert(m_bodyCount < m_bodyCapacity);
			body->m_islandIndex = m_bodyCount;
			m_bodies[m_bodyCount] = body;
			++m_bodyCount;
		}

		void Add(b3_Contact* contact)
		{
			assert(m_contactCount < m_contactCapacity);
			m_contacts[m_contactCount++] = contact;
		}

		void Add(b3_Joint* joint)
		{
			assert(m_jointCount < m_jointCapacity);
			m_joints[m_jointCount++] = joint;
		}

		void Report(const b3_ContactVelocityConstraint* constraints);

		b3_StackAllocator* m_allocator;
		b3_ContactListener* m_listener;

		b3_Body** m_bodies;
		b3_Contact** m_contacts;
		b3_Joint** m_joints;

		b3_Position* m_positions;  // λ�����飬b3_Island::Solve�б���m_bodies����sweep.center��sweep.rotationע��positions
		b3_Velocity* m_velocities; // �ٶ����飬b3_Island::Solve�б���m_bodies�������������ٶȽ��ٶ�ע��velocities

		int m_bodyCount;
		int m_jointCount;
		int m_contactCount;
		int m_bodyCapacity;
		int m_contactCapacity;
		int m_jointCapacity;
	};

}