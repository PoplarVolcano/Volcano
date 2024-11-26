#include "volpch.h"

#include "b3_World.h"
#include "b3_Body.h"
#include "b3_Fixture.h"
#include "b3_Joint.h"
#include "b3_Contact.h"
#include <Volcano/Core/Timer.h>
#include "b3_BroadPhase.h"
#include "b3_Draw.h"
#include "b3_Island.h"
#include "Collision/b3_SphereShape.h"
#include "b3_TimeOfImpact.h"

namespace Volcano {

	b3_World::b3_World(const glm::vec3& gravity)
	{
		m_destructionListener = nullptr;
		//m_debugDraw = nullptr;

		m_bodyList = nullptr;
		m_jointList = nullptr;
		m_bodyCount = 0;
		m_jointCount = 0;

		m_warmStarting = true;
		m_continuousPhysics = true;
		m_subStepping = false;

		m_stepComplete = true;
		m_allowSleep = true;
		m_gravity = gravity;
		m_newContacts = false;
		m_locked = false;
		m_clearForces = true;
		m_inv_deltaTime0 = 0.0f;
		m_contactManager.m_allocator = &m_blockAllocator;

		memset(&m_profile, 0, sizeof(b3_Profile));
	}

	b3_World::~b3_World()
	{
		b3_Body* body = m_bodyList;
		while (body)
		{
			b3_Body* bNext = body->m_next;

			b3_Fixture* f = body->m_fixtureList;
			while (f)
			{
				b3_Fixture* fNext = f->m_next;
				f->m_proxyCount = 0;
				f->Destroy(&m_blockAllocator);
				f = fNext;
			}

			body = bNext;
		}
	}

	void b3_World::SetDestructionListener(b3_DestructionListener* listener)
	{
		m_destructionListener = listener;
	}

	void b3_World::SetContactFilter(b3_ContactFilter* filter)
	{
		m_contactManager.m_contactFilter = filter;
	}

	void b3_World::SetContactListener(b3_ContactListener* listener)
	{
		m_contactManager.m_contactListener = listener;
	}

	/*
	void b3_World::SetDebugDraw(b3_Draw* debugDraw)
	{
		m_debugDraw = debugDraw;
	}
	*/
	b3_Body* b3_World::CreateBody(const b3_BodyDef* def)
	{
		if (IsLocked())
		{
			return nullptr;
		}

		void* mem = m_blockAllocator.Allocate(sizeof(b3_Body));
		b3_Body* body = new (mem) b3_Body(def, this);

		// Add to world doubly linked list.
		// 加入世界的双向链表表头
		body->m_prev = nullptr;
		body->m_next = m_bodyList;
		if (m_bodyList)
		{
			m_bodyList->m_prev = body;
		}
		m_bodyList = body;
		++m_bodyCount;

		return body;
	}
	void b3_World::DestroyBody(b3_Body* body)
	{
		assert(m_bodyCount > 0);
		assert(IsLocked() == false);
		if (IsLocked())
		{
			return;
		}

		// Delete the attached joints.
		b3_JointEdge* je = body->m_jointList;
		while (je)
		{
			b3_JointEdge* je0 = je;
			je = je->next;

			if (m_destructionListener)
			{
				m_destructionListener->SayGoodbye(je0->joint);
			}

			DestroyJoint(je0->joint);

			body->m_jointList = je;
		}
		body->m_jointList = nullptr;

		// Delete the attached contacts.
		b3_ContactEdge* ce = body->m_contactList;
		while (ce)
		{
			b3_ContactEdge* ce0 = ce;
			ce = ce->next;
			m_contactManager.Destroy(ce0->contact);
		}
		body->m_contactList = nullptr;

		// Delete the attached fixtures. This destroys broad-phase proxies.
		b3_Fixture* f = body->m_fixtureList;
		while (f)
		{
			b3_Fixture* f0 = f;
			f = f->m_next;

			if (m_destructionListener)
			{
				m_destructionListener->SayGoodbye(f0);
			}

			f0->DestroyProxies(&m_contactManager.m_broadPhase);
			f0->Destroy(&m_blockAllocator);
			f0->~b3_Fixture();
			m_blockAllocator.Free(f0, sizeof(b3_Fixture));

			body->m_fixtureList = f;
			body->m_fixtureCount -= 1;
		}
		body->m_fixtureList = nullptr;
		body->m_fixtureCount = 0;

		// Remove world body list.
		if (body->m_prev)
		{
			body->m_prev->m_next = body->m_next;
		}

		if (body->m_next)
		{
			body->m_next->m_prev = body->m_prev;
		}

		if (body == m_bodyList)
		{
			if (body->m_next)
				m_bodyList = body->m_next;
			else
				m_bodyList = body->m_prev;
		}

		--m_bodyCount;
		body->~b3_Body();
		m_blockAllocator.Free(body, sizeof(b3_Body));
	}
	b3_Joint* b3_World::CreateJoint(const b3_JointDef* def)
	{
		assert(IsLocked() == false);
		if (IsLocked())
		{
			return nullptr;
		}

		b3_Joint* j = b3_Joint::Create(def, &m_blockAllocator);

		// Connect to the world list.
		j->m_prev = nullptr;
		j->m_next = m_jointList;
		if (m_jointList)
		{
			m_jointList->m_prev = j;
		}
		m_jointList = j;
		++m_jointCount;

		// Connect to the bodies' doubly linked lists.
		j->m_edgeA.joint = j;
		j->m_edgeA.other = j->m_bodyB;
		j->m_edgeA.prev = nullptr;
		j->m_edgeA.next = j->m_bodyA->m_jointList;
		if (j->m_bodyA->m_jointList) j->m_bodyA->m_jointList->prev = &j->m_edgeA;
		j->m_bodyA->m_jointList = &j->m_edgeA;

		j->m_edgeB.joint = j;
		j->m_edgeB.other = j->m_bodyA;
		j->m_edgeB.prev = nullptr;
		j->m_edgeB.next = j->m_bodyB->m_jointList;
		if (j->m_bodyB->m_jointList) j->m_bodyB->m_jointList->prev = &j->m_edgeB;
		j->m_bodyB->m_jointList = &j->m_edgeB;

		b3_Body* bodyA = def->bodyA;
		b3_Body* bodyB = def->bodyB;

		// 如果关节可以阻止碰撞，则标记bodyA和bodyB的contacts进行过滤。
		if (def->collideConnected == false)
		{
			b3_ContactEdge* edge = bodyB->GetContactList();
			while (edge)
			{
				if (edge->other == bodyA)
				{
					// 标记contact，以便在下一时间步进行过滤（其中任何一方处于唤醒状态）。
					edge->contact->FlagForFiltering();
				}

				edge = edge->next;
			}
		}

		return j;
	}
	void b3_World::DestroyJoint(b3_Joint* joint)
	{
		assert(IsLocked() == false);
		if (IsLocked())
		{
			return;
		}

		bool collideConnected = joint->m_collideConnected;

		// Remove from the doubly linked list.
		if (joint->m_prev)
		{
			joint->m_prev->m_next = joint->m_next;
		}

		if (joint->m_next)
		{
			joint->m_next->m_prev = joint->m_prev;
		}

		if (joint == m_jointList)
		{
			m_jointList = joint->m_next;
		}

		// Disconnect from island graph.
		b3_Body* bodyA = joint->m_bodyA;
		b3_Body* bodyB = joint->m_bodyB;

		// Wake up connected bodies.
		bodyA->SetAwake(true);
		bodyB->SetAwake(true);

		// Remove from body 1.
		if (joint->m_edgeA.prev)
		{
			joint->m_edgeA.prev->next = joint->m_edgeA.next;
		}

		if (joint->m_edgeA.next)
		{
			joint->m_edgeA.next->prev = joint->m_edgeA.prev;
		}

		if (&joint->m_edgeA == bodyA->m_jointList)
		{
			if (joint->m_edgeA.next)
				bodyA->m_jointList = joint->m_edgeA.next;
			else
				bodyA->m_jointList = joint->m_edgeA.prev;
		}

		joint->m_edgeA.prev = nullptr;
		joint->m_edgeA.next = nullptr;

		// Remove from body 2
		if (joint->m_edgeB.prev)
		{
			joint->m_edgeB.prev->next = joint->m_edgeB.next;
		}

		if (joint->m_edgeB.next)
		{
			joint->m_edgeB.next->prev = joint->m_edgeB.prev;
		}

		if (&joint->m_edgeB == bodyB->m_jointList)
		{
			if (joint->m_edgeB.next)
				bodyB->m_jointList = joint->m_edgeB.next;
			else
				bodyB->m_jointList = joint->m_edgeB.prev;
		}

		joint->m_edgeB.prev = nullptr;
		joint->m_edgeB.next = nullptr;

		b3_Joint::Destroy(joint, &m_blockAllocator);

		assert(m_jointCount > 0);
		--m_jointCount;

		// 如果关节可以阻止碰撞，则标记bodyA和bodyB的contacts进行过滤。
		if (collideConnected == false)
		{
			b3_ContactEdge* edge = bodyB->GetContactList();
			while (edge)
			{
				if (edge->other == bodyA)
				{
					// 标记contact，以便在下一时间步进行过滤（其中任何一方处于唤醒状态）。
					edge->contact->FlagForFiltering();
				}

				edge = edge->next;
			}
		}
	}
	
	void b3_World::Step(float timeStep, int velocityIterations, int positionIterations)
	{
		Timer stepTimer;

		// 如果添加了新的fixtures，我们需要找到新的contacts。 
		if (m_newContacts)
		{
			m_contactManager.FindNewContacts();
			m_newContacts = false;
		}

		m_locked = true;

		b3_TimeStep step;
		step.deltaTime = timeStep;
		step.velocityIterations = velocityIterations;
		step.positionIterations = positionIterations;
		if (timeStep > 0.0f)
		{
			step.inv_deltaTime = 1.0f / timeStep;
		}
		else
		{
			step.inv_deltaTime = 0.0f;
		}

		step.deltaTimeRatio = m_inv_deltaTime0 * timeStep;

		step.warmStarting = m_warmStarting;

		// 处理ContactManager的contact列表(broad-phase中检测获得)所有contact的碰撞检测。
		// 被过滤和broad-phase未碰撞的contact会被销毁。最后进行narrow phase处理
		{
			Timer timer;
			m_contactManager.Collide();
			m_profile.collide = timer.ElapsedMillis();
		}

		// 合并(Integrate)速度，结算(solve)速度约束，合并位置。
		if (m_stepComplete && step.deltaTime > 0.0f)
		{
			Timer timer;
			Solve(step);
			m_profile.solve = timer.ElapsedMillis();
		}

		// 处理 TOI（Time of Impact）事件
		if (m_continuousPhysics && step.deltaTime > 0.0f)
		{
			Timer timer;
			//SolveTOI(step);
			m_profile.solveTOI = timer.ElapsedMillis();
		}

		if (step.deltaTime > 0.0f)
		{
			m_inv_deltaTime0 = step.inv_deltaTime;
		}

		if (m_clearForces)
		{
			ClearForces();
		}

		m_locked = false;

		m_profile.step = stepTimer.ElapsedMillis();
	}

	void b3_World::ClearForces()
	{
		for (b3_Body* body = m_bodyList; body; body = body->GetNext())
		{
			body->m_force = { 0.0f, 0.0f, 0.0f };
			body->m_torque = { 0.0f, 0.0f, 0.0f };
		}
	}

	struct b3_WorldQueryWrapper
	{
		bool QueryCallback(int proxyId)
		{
			b3_FixtureProxy* proxy = (b3_FixtureProxy*)broadPhase->GetUserData(proxyId);
			return callback->ReportFixture(proxy->fixture);
		}

		const b3_BroadPhase* broadPhase;
		b3_QueryCallback* callback;
	};

	void b3_World::QueryAABB(b3_QueryCallback* callback, const b3_AABB& aabb) const
	{
		b3_WorldQueryWrapper wrapper;
		wrapper.broadPhase = &m_contactManager.m_broadPhase;
		wrapper.callback = callback;
		m_contactManager.m_broadPhase.Query(&wrapper, aabb);
	}

	struct b3_WorldRayCastWrapper
	{
		float RayCastCallback(const b3_RayCastInput& input, int proxyId)
		{
			void* userData = broadPhase->GetUserData(proxyId);
			b3_FixtureProxy* proxy = (b3_FixtureProxy*)userData;
			b3_Fixture* fixture = proxy->fixture;
			int index = proxy->childIndex;
			b3_RayCastOutput output;
			bool hit = fixture->RayCast(&output, input, index);

			if (hit)
			{
				float fraction = output.fraction;
				glm::vec3 point = (1.0f - fraction) * input.p1 + fraction * input.p2;
				return callback->ReportFixture(fixture, point, output.normal, fraction);
			}

			return input.maxFraction;
		}

		const b3_BroadPhase* broadPhase;
		b3_RayCastCallback* callback;
	};

	void b3_World::RayCast(b3_RayCastCallback* callback, const glm::vec3& point1, const glm::vec3& point2) const
	{
		b3_WorldRayCastWrapper wrapper;
		wrapper.broadPhase = &m_contactManager.m_broadPhase;
		wrapper.callback = callback;
		b3_RayCastInput input;
		input.maxFraction = 1.0f;
		input.p1 = point1;
		input.p2 = point2;
		m_contactManager.m_broadPhase.RayCast(&wrapper, input);
	}

	void b3_World::SetAllowSleeping(bool flag)
	{
		if (flag == m_allowSleep)
		{
			return;
		}

		m_allowSleep = flag;
		// 设置不许睡，唤醒所有body
		if (m_allowSleep == false)
		{
			for (b3_Body* b = m_bodyList; b; b = b->m_next)
			{
				b->SetAwake(true);
			}
		}
	}

	/*
	void b3_World::DebugDraw()
	{
		if (m_debugDraw == nullptr)
		{
			return;
		}

		uint_t flags = m_debugDraw->GetFlags();

		if (flags & b3_Draw::e_shapeBit)
		{
			for (b3_Body* b = m_bodyList; b; b = b->GetNext())
			{
				const b3_Transform& transform = b->GetTransform();
				for (b3_Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
				{
					if (b->GetType() == e_dynamicBody && b->m_mass == 0.0f)
					{
						// Bad body
						DrawShape(f, transform, b3_Color(1.0f, 0.0f, 0.0f));
					}
					else if (b->IsEnabled() == false)
					{
						DrawShape(f, transform, b3_Color(0.5f, 0.5f, 0.3f));
					}
					else if (b->GetType() == b3__staticBody)
					{
						DrawShape(f, transform, b3_Color(0.5f, 0.9f, 0.5f));
					}
					else if (b->GetType() == b3__kinematicBody)
					{
						DrawShape(f, transform, b3_Color(0.5f, 0.5f, 0.9f));
					}
					else if (b->IsAwake() == false)
					{
						DrawShape(f, transform, b3_Color(0.6f, 0.6f, 0.6f));
					}
					else
					{
						DrawShape(f, transform, b3_Color(0.9f, 0.7f, 0.7f));
					}
				}
			}
		}

		if (flags & b3_Draw::e_jointBit)
		{
			for (b3_Joint* j = m_jointList; j; j = j->GetNext())
			{
				j->Draw(m_debugDraw);
			}
		}

		if (flags & b3_Draw::e_pairBit)
		{
			b3_Color color(0.3f, 0.9f, 0.9f);
			for (b3_Contact* c = m_contactManager.m_contactList; c; c = c->GetNext())
			{
				b3_Fixture* fixtureA = c->GetFixtureA();
				b3_Fixture* fixtureB = c->GetFixtureB();
				int indexA = c->GetChildIndexA();
				int indexB = c->GetChildIndexB();
				glm::vec3 cA = fixtureA->GetAABB(indexA).GetCenter();
				glm::vec3 cB = fixtureB->GetAABB(indexB).GetCenter();

				m_debugDraw->DrawSegment(cA, cB, color);
			}
		}

		if (flags & b3_Draw::e_aabbBit)
		{
			b3_Color color(0.9f, 0.3f, 0.9f);
			b3_BroadPhase* bp = &m_contactManager.m_broadPhase;

			for (b3_Body* b = m_bodyList; b; b = b->GetNext())
			{
				if (b->IsEnabled() == false)
				{
					continue;
				}

				for (b3_Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
				{
					for (int i = 0; i < f->m_proxyCount; ++i)
					{
						b3_FixtureProxy* proxy = f->m_proxies + i;
						b3_AABB aabb = bp->GetFatAABB(proxy->proxyId);
						glm::vec3 vs[4];
						vs[0].Set(aabb.lowerBound.x, aabb.lowerBound.y);
						vs[1].Set(aabb.upperBound.x, aabb.lowerBound.y);
						vs[2].Set(aabb.upperBound.x, aabb.upperBound.y);
						vs[3].Set(aabb.lowerBound.x, aabb.upperBound.y);

						m_debugDraw->DrawPolygon(vs, 4, color);
					}
				}
			}
		}

		if (flags & b3_Draw::e_centerOfMassBit)
		{
			for (b3_Body* b = m_bodyList; b; b = b->GetNext())
			{
				b3_Transform transform = b->GetTransform();
				transform.p = b->GetWorldCenter();
				m_debugDraw->DrawTransform(transform);
			}
		}
	}
	*/

	int b3_World::GetProxyCount() const
	{
		return m_contactManager.m_broadPhase.GetProxyCount();
	}

	int b3_World::GetTreeHeight() const
	{
		return m_contactManager.m_broadPhase.GetTreeHeight();
	}

	int b3_World::GetTreeBalance() const
	{
		return m_contactManager.m_broadPhase.GetTreeBalance();
	}

	/*
	float b3_World::GetTreeQuality() const
	{
		return m_contactManager.m_broadPhase.GetTreeQuality();
	}
	*/
	void b3_World::ShiftOrigin(const glm::vec3& newOriginTranslate)
	{
		assert(m_locked == false);
		if (m_locked)
		{
			return;
		}

		for (b3_Body* b = m_bodyList; b; b = b->m_next)
		{
			b->m_transform.position -= newOriginTranslate;
			b->m_sweep.center0 -= newOriginTranslate;
			b->m_sweep.center  -= newOriginTranslate;
		}

		for (b3_Joint* j = m_jointList; j; j = j->m_next)
		{
			j->ShiftOrigin(newOriginTranslate);
		}

		m_contactManager.m_broadPhase.ShiftOrigin(newOriginTranslate);
	}

	
	void b3_World::Solve(const b3_TimeStep& step)
	{
		m_profile.solveInit = 0.0f;
		m_profile.solveVelocity = 0.0f;
		m_profile.solvePosition = 0.0f;

		// 为最坏的情况设置island的大小。
		b3_Island island(m_bodyCount, m_contactManager.m_contactCount, m_jointCount, &m_stackAllocator, m_contactManager.m_contactListener);

		// 清除所有body的island标志
		for (b3_Body* b = m_bodyList; b; b = b->m_next)
		{
			b->m_flags &= ~b3_Body::e_islandFlag;
		}
		// 清除所有contact的island标志
		for (b3_Contact* c = m_contactManager.m_contactList; c; c = c->m_next)
		{
			c->m_flags &= ~b3_Contact::e_islandFlag;
		}
		// 清除所有joint的island标志
		for (b3_Joint* j = m_jointList; j; j = j->m_next)
		{
			j->m_islandFlag = false;
		}

		// 构建并模拟所有醒的island。
		int stackSize = m_bodyCount;
		b3_Body** stack = (b3_Body**)m_stackAllocator.Allocate(stackSize * sizeof(b3_Body*));

		for (b3_Body* seed = m_bodyList; seed; seed = seed->m_next)
		{
			// body已上岛，跳过
			if (seed->m_flags & b3_Body::e_islandFlag)
			{
				continue;
			}

			// body睡眠或禁用，跳过
			if (seed->IsAwake() == false || seed->IsEnabled() == false)
			{
				continue;
			}

			// body静态，跳过
			if (seed->GetType() == b3_BodyType::e_staticBody)
			{
				continue;
			}

			// 重置island和栈
			island.Clear();
			int stackCount = 0;

			// body入栈，标记入栈，后续不再入栈
			stack[stackCount++] = seed;
			seed->m_flags |= b3_Body::e_islandFlag;

			// 在约束图上执行深度优先搜索(depth first search)（DFS）。
			// 递归遍历栈中的body的contact列表，contact上岛，获取contact另一个body并入栈，再继续遍历新入栈body的contact列表，递归，直到所有的关联contact上岛
			// 递归遍历栈中的body的joint列表，joint上岛，获取joint另一个body并入栈，再继续遍历新入栈body的joint列表，递归，直到所有的关联joint上岛
			while (stackCount > 0)
			{
				// 从栈中取出body，上岛
				b3_Body* b = stack[--stackCount];
				assert(b->IsEnabled() == true);
				island.Add(b);

				// 为了使island尽可能小，静态body不上岛
				if (b->GetType() == b3_BodyType::e_staticBody)
				{
					continue;
				}

				// 确保body非睡眠（不重置睡眠定时器）。
				b->m_flags |= b3_Body::e_awakeFlag;

				// 遍历连接到这个body的contact
				for (b3_ContactEdge* ce = b->m_contactList; ce; ce = ce->next)
				{
					b3_Contact* contact = ce->contact;

					if (contact->m_flags & b3_Contact::e_islandFlag)
					{
						// contact已上岛，跳过
						continue;
					}

					if (contact->IsEnabled() == false || contact->IsTouching() == false)
					{
						//contact未启动或未碰撞，跳过
						continue;
					}

					// 跳过传感器
					bool sensorA = contact->m_fixtureA->m_isSensor;
					bool sensorB = contact->m_fixtureB->m_isSensor;
					if (sensorA || sensorB)
					{
						continue;
					}

					// contact上岛
					island.Add(contact);
					contact->m_flags |= b3_Contact::e_islandFlag;

					b3_Body* other = ce->other;

					if (other->m_flags & b3_Body::e_islandFlag)
					{
						// contact的另一个body已标记入栈，跳过
						continue;
					}

					// contact的另一个body入栈，标记入栈，下一轮递归前不再入栈
					assert(stackCount < stackSize);
					stack[stackCount++] = other;
					other->m_flags |= b3_Body::e_islandFlag;
				}

				// 遍历连接到这个body的所有关节。
				for (b3_JointEdge* je = b->m_jointList; je; je = je->next)
				{
					if (je->joint->m_islandFlag == true)
					{
						// 关节已上岛，跳过
						continue;
					}

					b3_Body* other = je->other;

					if (other->IsEnabled() == false)
					{
						// 关节连接到已禁用body，跳过
						continue;
					}

					// 关节上岛
					island.Add(je->joint);
					je->joint->m_islandFlag = true;

					if (other->m_flags & b3_Body::e_islandFlag)
					{
						// 关节的另一个body已标记入栈，跳过
						continue;
					}

					// contact的另一个body入栈，标记入栈，下一轮递归前不再入栈
					assert(stackCount < stackSize);
					stack[stackCount++] = other;
					other->m_flags |= b3_Body::e_islandFlag;
				}
			}

			// seed所有关联contact和joint上岛后，执行结算(solve)
			b3_Profile profile;
			island.Solve(&profile, step, m_gravity, m_allowSleep);
			m_profile.solveInit += profile.solveInit;
			m_profile.solveVelocity += profile.solveVelocity;
			m_profile.solvePosition += profile.solvePosition;

			// 结算后重置已上岛的静态body的入栈标记，允许静态body加入其他body的结算。
			for (int i = 0; i < island.m_bodyCount; ++i)
			{
				b3_Body* b = island.m_bodies[i];
				if (b->GetType() == b3_BodyType::e_staticBody)
				{
					b->m_flags &= ~b3_Body::e_islandFlag;
				}
			}
		}

		m_stackAllocator.Free(stack);

		{
			Timer timer;
			// 同步fixtures，检查是否有超出范围的bodies，可能会导致broad-phase的动态树变动
			for (b3_Body* b = m_bodyList; b; b = b->GetNext())
			{
				// 如果一个body不在island上，那么它就不会移动。
				if ((b->m_flags & b3_Body::e_islandFlag) == 0)
				{
					continue;
				}

				if (b->GetType() == b3_BodyType::e_staticBody)
				{
					continue;
				}

				// 更新fixtures (for broad-phase).
				b->SynchronizeFixtures();
			}

			// 查找新的contact
			m_contactManager.FindNewContacts();
			m_profile.broadphase = timer.ElapsedMillis();
		}
	}

	//查找TOI contacts并结算。
	
	void b3_World::SolveTOI(const b3_TimeStep& step)
	{
		b3_Island island(2 * b3_MaxTOIContacts, b3_MaxTOIContacts, 0, &m_stackAllocator, m_contactManager.m_contactListener);

		if (m_stepComplete)
		{
			for (b3_Body* b = m_bodyList; b; b = b->m_next)
			{
				b->m_flags &= ~b3_Body::e_islandFlag;
				b->m_sweep.alpha0 = 0.0f;
			}

			for (b3_Contact* c = m_contactManager.m_contactList; c; c = c->m_next)
			{
				// 使TOI无效(Invalidate)
				c->m_flags &= ~(b3_Contact::e_toiFlag | b3_Contact::e_islandFlag);
				c->m_toiCount = 0;
				c->m_toi = 1.0f;
			}
		}

		// 查找TOI事件并结算它们。
		for (;;)
		{
			// Find the first TOI.
			b3_Contact* minContact = nullptr;
			float minAlpha = 1.0f;

			for (b3_Contact* c = m_contactManager.m_contactList; c; c = c->m_next)
			{
				// Is this contact disabled?
				if (c->IsEnabled() == false)
				{
					continue;
				}

				// 防止过度子步。 Prevent excessive sub-stepping.
				if (c->m_toiCount > b3_MaxSubSteps)
				{
					continue;
				}

				float alpha = 1.0f;
				if (c->m_flags & b3_Contact::e_toiFlag)
				{
					// 此contact具有有效的缓存TOI。
					alpha = c->m_toi;
				}
				else
				{
					b3_Fixture* fA = c->GetFixtureA();
					b3_Fixture* fB = c->GetFixtureB();

					if (fA->IsSensor() || fB->IsSensor())
					{
						// 有传感器，跳过
						continue;
					}

					b3_Body* bA = fA->GetBody();
					b3_Body* bB = fB->GetBody();

					b3_BodyType typeA = bA->m_type;
					b3_BodyType typeB = bB->m_type;
					// 至少一个是动态body
					assert(typeA == b3_BodyType::e_dynamicBody || typeB == b3_BodyType::e_dynamicBody);

					bool activeA = bA->IsAwake() && typeA != b3_BodyType::e_staticBody;
					bool activeB = bB->IsAwake() && typeB != b3_BodyType::e_staticBody;

					// Is at least one body active (awake and dynamic or kinematic)?
					if (activeA == false && activeB == false)
					{
						continue;
					}

					bool collideA = bA->IsBullet() || typeA != b3_BodyType::e_dynamicBody;
					bool collideB = bB->IsBullet() || typeB != b3_BodyType::e_dynamicBody;

					if (collideA == false && collideB == false)
					{
						//2个非子弹静态body，跳过
						continue;
					}

					// 计算此contact的TOI。将sweeps放在相同的时间间隔(interval)上。
					float alpha0 = bA->m_sweep.alpha0;

					if (bA->m_sweep.alpha0 < bB->m_sweep.alpha0)
					{
						alpha0 = bB->m_sweep.alpha0;
						bA->m_sweep.Advance(alpha0);
					}
					else if (bB->m_sweep.alpha0 < bA->m_sweep.alpha0)
					{
						alpha0 = bA->m_sweep.alpha0;
						bB->m_sweep.Advance(alpha0);
					}

					assert(alpha0 < 1.0f);

					int indexA = c->GetChildIndexA();
					int indexB = c->GetChildIndexB();

					// 计算间隔[0，minTOI]内的撞击时间(time of impact)
					b3_TOIInput input;
					input.proxyA.Set(fA->GetShape(), indexA);
					input.proxyB.Set(fB->GetShape(), indexB);
					input.sweepA = bA->m_sweep;
					input.sweepB = bB->m_sweep;
					input.tMax = 1.0f;

					b3_TOIOutput output;
					b3_TimeOfImpact(&output, &input);

					// Beta is the fraction of the remaining portion of the .
					float beta = output.t;
					if (output.state == b3_TOIOutput::e_touching)
					{
						alpha = glm::min(alpha0 + (1.0f - alpha0) * beta, 1.0f);
					}
					else
					{
						alpha = 1.0f;
					}

					c->m_toi = alpha;
					c->m_flags |= b3_Contact::e_toiFlag;
				}

				if (alpha < minAlpha)
				{
					// This is the minimum TOI found so far.
					minContact = c;
					minAlpha = alpha;
				}
			}

			if (minContact == nullptr || 1.0f - 10.0f * b3_Epsilon < minAlpha)
			{
				// No more TOI events. Done!
				m_stepComplete = true;
				break;
			}

			// Advance the bodies to the TOI.
			b3_Fixture* fA = minContact->GetFixtureA();
			b3_Fixture* fB = minContact->GetFixtureB();
			b3_Body* bA = fA->GetBody();
			b3_Body* bB = fB->GetBody();

			b3_Sweep backup1 = bA->m_sweep;
			b3_Sweep backup2 = bB->m_sweep;

			bA->Advance(minAlpha);
			bB->Advance(minAlpha);

			// The TOI contact likely has some new contact points.
			minContact->Update(m_contactManager.m_contactListener);
			minContact->m_flags &= ~b3_Contact::e_toiFlag;
			++minContact->m_toiCount;

			// Is the contact solid?
			if (minContact->IsEnabled() == false || minContact->IsTouching() == false)
			{
				// Restore the sweeps.
				minContact->SetEnabled(false);
				bA->m_sweep = backup1;
				bB->m_sweep = backup2;
				bA->SynchronizeTransform();
				bB->SynchronizeTransform();
				continue;
			}

			bA->SetAwake(true);
			bB->SetAwake(true);

			// Build the island
			island.Clear();
			island.Add(bA);
			island.Add(bB);
			island.Add(minContact);

			bA->m_flags |= b3_Body::e_islandFlag;
			bB->m_flags |= b3_Body::e_islandFlag;
			minContact->m_flags |= b3_Contact::e_islandFlag;

			// Get contacts on bodyA and bodyB.
			b3_Body* bodies[2] = { bA, bB };
			for (int i = 0; i < 2; ++i)
			{
				b3_Body* body = bodies[i];
				if (body->m_type == b3_BodyType::e_dynamicBody)
				{
					for (b3_ContactEdge* ce = body->m_contactList; ce; ce = ce->next)
					{
						if (island.m_bodyCount == island.m_bodyCapacity)
						{
							break;
						}

						if (island.m_contactCount == island.m_contactCapacity)
						{
							break;
						}

						b3_Contact* contact = ce->contact;

						// Has this contact already been added to the island?
						if (contact->m_flags & b3_Contact::e_islandFlag)
						{
							continue;
						}

						// Only add static, kinematic, or bullet bodies.
						b3_Body* other = ce->other;
						if (other->m_type == b3_BodyType::e_dynamicBody &&
							body->IsBullet() == false && other->IsBullet() == false)
						{
							continue;
						}

						// Skip sensors.
						bool sensorA = contact->m_fixtureA->m_isSensor;
						bool sensorB = contact->m_fixtureB->m_isSensor;
						if (sensorA || sensorB)
						{
							continue;
						}

						// Tentatively advance the body to the TOI.
						b3_Sweep backup = other->m_sweep;
						if ((other->m_flags & b3_Body::e_islandFlag) == 0)
						{
							other->Advance(minAlpha);
						}

						// Update the contact points
						contact->Update(m_contactManager.m_contactListener);

						// Was the contact disabled by the user?
						if (contact->IsEnabled() == false)
						{
							other->m_sweep = backup;
							other->SynchronizeTransform();
							continue;
						}

						// Are there contact points?
						if (contact->IsTouching() == false)
						{
							other->m_sweep = backup;
							other->SynchronizeTransform();
							continue;
						}

						// Add the contact to the island
						contact->m_flags |= b3_Contact::e_islandFlag;
						island.Add(contact);

						// Has the other body already been added to the island?
						if (other->m_flags & b3_Body::e_islandFlag)
						{
							continue;
						}

						// Add the other body to the island.
						other->m_flags |= b3_Body::e_islandFlag;

						if (other->m_type != b3_BodyType::e_staticBody)
						{
							other->SetAwake(true);
						}

						island.Add(other);
					}
				}
			}

			b3_TimeStep subStep;
			subStep.deltaTime = (1.0f - minAlpha) * step.deltaTime;
			subStep.inv_deltaTime = 1.0f / subStep.deltaTime;
			subStep.deltaTimeRatio = 1.0f;
			subStep.positionIterations = 20;
			subStep.velocityIterations = step.velocityIterations;
			subStep.warmStarting = false;
			island.SolveTOI(subStep, bA->m_islandIndex, bB->m_islandIndex);

			// Reset island flags and synchronize broad-phase proxies.
			for (int i = 0; i < island.m_bodyCount; ++i)
			{
				b3_Body* body = island.m_bodies[i];
				body->m_flags &= ~b3_Body::e_islandFlag;

				if (body->m_type != b3_BodyType::e_dynamicBody)
				{
					continue;
				}

				body->SynchronizeFixtures();

				// Invalidate all contact TOIs on this displaced body.
				for (b3_ContactEdge* ce = body->m_contactList; ce; ce = ce->next)
				{
					ce->contact->m_flags &= ~(b3_Contact::e_toiFlag | b3_Contact::e_islandFlag);
				}
			}

			// Commit fixture proxy movements to the broad-phase so that new contacts are created.
			// Also, some contacts can be destroyed.
			m_contactManager.FindNewContacts();

			if (m_subStepping)
			{
				m_stepComplete = false;
				break;
			}
		}
	}
	/*
	void b3_World::DrawShape(b3_Fixture* fixture, const b3_Transform& transform, const b3_Color& color)
	{
		switch (fixture->GetType())
		{
		case b3_Shape::e_sphere:
		{
			b3_SphereShape* sphere = (b3_SphereShape*)fixture->GetShape();

			glm::vec3 center = b3_Multiply(transform, sphere->m_position);
			float radius = sphere->m_radius;
			glm::vec3 axis = b3_Multiply(transform.rotation, glm::vec3(1.0f, 0.0f));

			m_debugDraw->DrawSolidCircle(center, radius, axis, color);
		}
		break;
		
		case b3_Shape::e_edge:
		{
			b3_EdgeShape* edge = (b3_EdgeShape*)fixture->GetShape();
			glm::vec3 v1 = b3_Multiply(transform, edge->m_vertex1);
			glm::vec3 v2 = b3_Multiply(transform, edge->m_vertex2);
			m_debugDraw->DrawSegment(v1, v2, color);

			if (edge->m_oneSided == false)
			{
				m_debugDraw->DrawPoint(v1, 4.0f, color);
				m_debugDraw->DrawPoint(v2, 4.0f, color);
			}
		}
		break;

		case b3_Shape::e_chain:
		{
			b3_ChainShape* chain = (b3_ChainShape*)fixture->GetShape();
			int count = chain->m_count;
			const glm::vec3* vertices = chain->m_vertices;

			glm::vec3 v1 = b3_Multiply(transform, vertices[0]);
			for (int i = 1; i < count; ++i)
			{
				glm::vec3 v2 = b3_Multiply(transform, vertices[i]);
				m_debugDraw->DrawSegment(v1, v2, color);
				v1 = v2;
			}
		}
		break;

		case b3_Shape::e_polygon:
		{
			b3_PolygonShape* poly = (b3_PolygonShape*)fixture->GetShape();
			int vertexCount = poly->m_count;
			assert(vertexCount <= b3__maxPolygonVertices);
			glm::vec3 vertices[b3__maxPolygonVertices];

			for (int i = 0; i < vertexCount; ++i)
			{
				vertices[i] = b3_Multiply(transform, poly->m_vertices[i]);
			}

			m_debugDraw->DrawSolidPolygon(vertices, vertexCount, color);
		}
		break;
		
		default:
			break;
		}
	}
	*/

}