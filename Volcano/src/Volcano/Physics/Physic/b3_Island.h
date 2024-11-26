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

	// b3_Word在处理碰撞的时候需要计算时，将各种东西塞进b3_Island中进行计算，也就是说这个类存在的意义就是计算。
	class b3_Island
	{
	public:
		b3_Island(int bodyCapacity, int contactCapacity, int jointCapacity, b3_StackAllocator* allocator, b3_ContactListener* listener);
		~b3_Island();

		// 清空body，contact，joint数组
		void Clear()
		{
			m_bodyCount = 0;
			m_contactCount = 0;
			m_jointCount = 0;
		}

		// 结算，返回分析数据profile
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

		b3_Position* m_positions;  // 位置数组，b3_Island::Solve中遍历m_bodies，将sweep.center和sweep.rotation注入positions
		b3_Velocity* m_velocities; // 速度数组，b3_Island::Solve中遍历m_bodies，将计算后的线速度角速度注入velocities

		int m_bodyCount;
		int m_jointCount;
		int m_contactCount;
		int m_bodyCapacity;
		int m_contactCapacity;
		int m_jointCapacity;
	};

}