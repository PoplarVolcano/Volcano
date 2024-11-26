#include "volpch.h"

#include "b3_Island.h"
#include "b3_StackAllocator.h"
#include <Volcano/Core/Timer.h>
#include "b3_ContactSolver.h"
#include "b3_Joint.h"
#include "b3_WorldCallbacks.h"

namespace Volcano {

	b3_Island::b3_Island(int bodyCapacity, int contactCapacity, int jointCapacity, b3_StackAllocator* allocator, b3_ContactListener* listener)
	{
		m_bodyCapacity = bodyCapacity;
		m_contactCapacity = contactCapacity;
		m_jointCapacity = jointCapacity;
		m_bodyCount = 0;
		m_contactCount = 0;
		m_jointCount = 0;

		m_allocator = allocator;
		m_listener = listener;

		m_bodies = (b3_Body**)m_allocator->Allocate(bodyCapacity * sizeof(b3_Body*));
		m_contacts = (b3_Contact**)m_allocator->Allocate(contactCapacity * sizeof(b3_Contact*));
		m_joints = (b3_Joint**)m_allocator->Allocate(jointCapacity * sizeof(b3_Joint*));

		m_velocities = (b3_Velocity*)m_allocator->Allocate(m_bodyCapacity * sizeof(b3_Velocity));
		m_positions = (b3_Position*)m_allocator->Allocate(m_bodyCapacity * sizeof(b3_Position));
	}
	b3_Island::~b3_Island()
	{
		// 警告：顺序应与构造函数顺序相反。
		m_allocator->Free(m_positions);
		m_allocator->Free(m_velocities);
		m_allocator->Free(m_joints);
		m_allocator->Free(m_contacts);
		m_allocator->Free(m_bodies);
	}
	void b3_Island::Solve(b3_Profile* profile, const b3_TimeStep& step, const glm::vec3& gravity, bool allowSleep)
	{
		Timer timer;

		float deltaTime = step.deltaTime;

		// 合并速度(velocities)并应用阻尼(damping)。初始化body状态。
		// 遍历body，将body的位置旋转和速度注入对应列表
		for (int i = 0; i < m_bodyCount; ++i)
		{
			b3_Body* body = m_bodies[i];

			glm::vec3 c = body->m_sweep.center;
			glm::vec3 a = body->m_sweep.rotation;
			glm::vec3 v = body->m_linearVelocity;
			glm::vec3 w = body->m_angularVelocity;

			// 存放连续碰撞的位置。 Store positions for continuous collision.
			body->m_sweep.center0 = body->m_sweep.center;
			body->m_sweep.rotation0 = body->m_sweep.rotation;

			if (body->m_type == b3_BodyType::e_dynamicBody)
			{
				// 速度插值 Integrate velocities.
				v += deltaTime * body->m_invMass * (body->m_gravityScale * body->m_mass * gravity + body->m_force); //  v2 = v1 + Ft/m
				w += deltaTime * body->m_invI * body->m_torque;  // 角加速度α=扭矩τ/转动惯量I  w2 = w1 + αt

				// 施加阻尼(damping)
				// v(t) = v(0) + v'(0)t + 2v''(0)t^2    v(t)：施加阻尼后的速度
				// a(t) = v'(t) = v'(0) + 4v''(0)t
				// F(t) = a'(t) = 4v''(0)
				// a(t) + F(t) = v'(0) + 4v''(0)(1+t) = 0     v(0) = v0:未施加阻尼前的速度，常量，故v'(0) = 0
				// ODE: dv/dt + c * v = 0  (常微分方程(Ordinary Differential Equation)，y' + Cy = 0 => y = y0 * exp(-c * x))
				// Solution: v(t) = v0 * exp(-c * t)
				// Time step: v(t + dt) = v0 * exp(-c * (t + dt)) = v0 * exp(-c * t) * exp(-c * dt) = v(t) * exp(-c * dt)
				// v2 = exp(-c * dt) * v1
				// Pade approximation: (取一阶泰勒展开作为近似项) 
				// e^x = f(0)+f'(0)x+f''(0)x^2... = 1 + x + x^2/2! ...
				// v2 = v1 * 1 / (1 + c * dt)
				v *= 1.0f / (1.0f + deltaTime * body->m_linearDamping);
				w *= 1.0f / (1.0f + deltaTime * body->m_angularDamping);
			}

			m_positions[i].position = c;
			m_positions[i].rotation = a;
			m_velocities[i].linearVelocity = v;
			m_velocities[i].angularVelocity = w;
		}

		timer.Reset();

		// 结算器数据
		b3_SolverData solverData;
		solverData.step = step;
		solverData.positions = m_positions;
		solverData.velocities = m_velocities;

		// 初始化contact速度约束
		b3_ContactSolverDef contactSolverDef;
		contactSolverDef.step = step;
		contactSolverDef.contacts = m_contacts;
		contactSolverDef.count = m_contactCount;
		contactSolverDef.positions = m_positions;
		contactSolverDef.velocities = m_velocities;
		contactSolverDef.allocator = m_allocator;

		b3_ContactSolver contactSolver(&contactSolverDef);
		contactSolver.InitializeVelocityConstraints();
		
		if (step.warmStarting)
		{
			contactSolver.WarmStart();
		}

		// 初始化关节速度约束
		// TODO：还未使用关节
		for (int i = 0; i < m_jointCount; ++i)
		{
			m_joints[i]->InitVelocityConstraints(solverData);
		}

		profile->solveInit = timer.ElapsedMillis();
		
		// 结算速度约束
		timer.Reset();
		for (int i = 0; i < step.velocityIterations; ++i)
		{
			// TODO：还未使用关节
			for (int j = 0; j < m_jointCount; ++j)
			{
				m_joints[j]->SolveVelocityConstraints(solverData);
			}
			// 结算contact速度约束
			contactSolver.SolveVelocityConstraints();
		}
		// 为热启动注入冲量
		contactSolver.StoreImpulses();
		profile->solveVelocity = timer.ElapsedMillis();


		// 合并position
		for (int i = 0; i < m_bodyCount; ++i)
		{
			glm::vec3 c = m_positions[i].position;
			glm::vec3 a = m_positions[i].rotation;
			glm::vec3 v = m_velocities[i].linearVelocity;
			glm::vec3 w = m_velocities[i].angularVelocity;

			// 检查是否存在大速度 Check for large velocities
			glm::vec3 translation = deltaTime * v;
			if (glm::dot(translation, translation) > b3_MaxTranslationSquared)
			{
				float ratio = b3_MaxTranslation / glm::length(translation);
				v *= ratio;
			}

			glm::vec3 rotation = deltaTime * w;
			if (glm::dot(rotation, rotation) > b3_MaxRotationSquared)
			{
				glm::vec3 ratio = b3_MaxRotation / glm::abs(rotation);
				w *= ratio;
			}

			// Integrate
			c += deltaTime * v;
			a += deltaTime * w;

			m_positions[i].position         = c;
			m_positions[i].rotation         = a;
			m_velocities[i].linearVelocity  = v;
			m_velocities[i].angularVelocity = w;
		}

		// 结算位置约束
		timer.Reset();
		bool positionSolved = false;
		for (int i = 0; i < step.positionIterations; ++i)
		{
			bool contactsOkay = contactSolver.SolvePositionConstraints();

			// TODO：还未使用关节
			bool jointsOkay = true;
			for (int j = 0; j < m_jointCount; ++j)
			{
				bool jointOkay = m_joints[j]->SolvePositionConstraints(solverData);
				jointsOkay = jointsOkay && jointOkay;
			}

			if (contactsOkay && jointsOkay)
			{
				// 如果位置误差较小就提前退出。
				positionSolved = true;
				break;
			}
		}

		// 将状态缓冲区复制回body
		for (int i = 0; i < m_bodyCount; ++i)
		{
			b3_Body* body = m_bodies[i];
			body->m_sweep.center = m_positions[i].position;
			body->m_sweep.rotation = m_positions[i].rotation;
			body->m_linearVelocity = m_velocities[i].linearVelocity;
			body->m_angularVelocity = m_velocities[i].angularVelocity;
			body->SynchronizeTransform();
		}

		profile->solvePosition = timer.ElapsedMillis();

		Report(contactSolver.m_velocityConstraints);

		if (allowSleep)
		{
			float minSleepTime = FLT_MAX;

			const float linTolSqr = b3_LinearSleepTolerance * b3_LinearSleepTolerance;
			const float angTolSqr = b3_AngularSleepTolerance * b3_AngularSleepTolerance;

			for (int i = 0; i < m_bodyCount; ++i)
			{
				b3_Body* body = m_bodies[i];
				if (body->GetType() == b3_BodyType::e_staticBody)
				{
					continue;
				}

				if ((body->m_flags & b3_Body::e_autoSleepFlag) == 0 ||
					glm::dot(body->m_angularVelocity, body->m_angularVelocity) > angTolSqr ||
					glm::dot(body->m_linearVelocity, body->m_linearVelocity) > linTolSqr)
				{
					body->m_sleepTime = 0.0f;
					minSleepTime = 0.0f;
				}
				else
				{
					body->m_sleepTime += deltaTime;
					minSleepTime = glm::min(minSleepTime, body->m_sleepTime);
				}
			}

			if (minSleepTime >= b3_TimeToSleep && positionSolved)
			{
				for (int i = 0; i < m_bodyCount; ++i)
				{
					b3_Body* body = m_bodies[i];
					body->SetAwake(false);
				}
			}
		}
		
	}

	
	void b3_Island::SolveTOI(const b3_TimeStep& subStep, int toiIndexA, int toiIndexB)
	{
		assert(toiIndexA < m_bodyCount);
		assert(toiIndexB < m_bodyCount);

		// Initialize the body state.
		for (int i = 0; i < m_bodyCount; ++i)
		{
			b3_Body* body = m_bodies[i];
			m_positions[i].position = body->m_sweep.center;
			m_positions[i].rotation = body->m_sweep.rotation;
			m_velocities[i].linearVelocity = body->m_linearVelocity;
			m_velocities[i].angularVelocity = body->m_angularVelocity;
		}

		b3_ContactSolverDef contactSolverDef;
		contactSolverDef.contacts = m_contacts;
		contactSolverDef.count = m_contactCount;
		contactSolverDef.allocator = m_allocator;
		contactSolverDef.step = subStep;
		contactSolverDef.positions = m_positions;
		contactSolverDef.velocities = m_velocities;
		b3_ContactSolver contactSolver(&contactSolverDef);

		// Solve position constraints.
		for (int i = 0; i < subStep.positionIterations; ++i)
		{
			bool contactsOkay = contactSolver.SolveTOIPositionConstraints(toiIndexA, toiIndexB);
			if (contactsOkay)
			{
				break;
			}
		}

		// Leap of faith to new safe state.
		m_bodies[toiIndexA]->m_sweep.center0 = m_positions[toiIndexA].position;
		m_bodies[toiIndexA]->m_sweep.rotation0 = m_positions[toiIndexA].rotation;
		m_bodies[toiIndexB]->m_sweep.center0 = m_positions[toiIndexB].position;
		m_bodies[toiIndexB]->m_sweep.rotation0 = m_positions[toiIndexB].rotation;

		// No warm starting is needed for TOI events because warm
		// starting impulses were applied in the discrete solver.
		contactSolver.InitializeVelocityConstraints();

		// Solve velocity constraints.
		for (int i = 0; i < subStep.velocityIterations; ++i)
		{
			contactSolver.SolveVelocityConstraints();
		}

		// Don't store the TOI contact forces for warm starting
		// because they can be quite large.

		float h = subStep.deltaTime;

		// Integrate positions
		for (int i = 0; i < m_bodyCount; ++i)
		{
			glm::vec3 c = m_positions[i].position;
			glm::vec3 a = m_positions[i].rotation;
			glm::vec3 v = m_velocities[i].linearVelocity;
			glm::vec3 w = m_velocities[i].angularVelocity;

			// Check for large velocities
			glm::vec3 translation = h * v;
			if (glm::dot(translation, translation) > b3_MaxTranslationSquared)
			{
				float ratio = b3_MaxTranslation / glm::length(translation);
				v *= ratio;
			}

			glm::vec3 rotation = h * w;
			if (glm::dot(rotation, rotation) > b3_MaxRotationSquared)
			{
				glm::vec3 ratio = b3_MaxRotation / glm::abs(rotation);
				w *= ratio;
			}

			// Integrate
			c += h * v;
			a += h * w;

			m_positions[i].position = c;
			m_positions[i].rotation = a;
			m_velocities[i].linearVelocity = v;
			m_velocities[i].angularVelocity = w;

			// Sync bodies
			b3_Body* body = m_bodies[i];
			body->m_sweep.center = c;
			body->m_sweep.rotation = a;
			body->m_linearVelocity = v;
			body->m_angularVelocity = w;
			body->SynchronizeTransform();
		}

		Report(contactSolver.m_velocityConstraints);
	}
	

	
	void b3_Island::Report(const b3_ContactVelocityConstraint* constraints)
	{
		if (m_listener == nullptr)
		{
			return;
		}

		for (int i = 0; i < m_contactCount; ++i)
		{
			b3_Contact* c = m_contacts[i];

			const b3_ContactVelocityConstraint* vc = constraints + i;

			b3_ContactImpulse impulse;
			impulse.count = vc->pointCount;
			for (int j = 0; j < vc->pointCount; ++j)
			{
				impulse.normalImpulses[j] = vc->points[j].normalImpulse;
				impulse.tangentImpulses[j] = vc->points[j].tangentImpulse;
			}

			m_listener->PostSolve(c, &impulse);
		}
	}
	
}
