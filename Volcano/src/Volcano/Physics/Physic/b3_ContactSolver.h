#pragma once

#include "b3_Math.h"
#include "b3_TimeStep.h"
#include "b3_Collision.h"

#include "glm/glm/detail/qualifier.hpp"

namespace Volcano {

	class b3_Contact;
	class b3_StackAllocator;
	struct b3_ContactPositionConstraint;

	// contact速度约束的点
	struct b3_VelocityConstraintPoint
	{
		glm::vec3 rA;        // 矢径A，contact的bodyA的质心到交点的向量 
		glm::vec3 rB;        // 矢径B，contact的bodyB的质心到交点的向量 
		float normalImpulse;  // 非穿透(non-penetration)冲量， 在b3_ContactSolver::SolveVelocityConstraints中被赋值
		float tangentImpulse; // 上一帧的切线冲量
		float normalMass;
		float tangentMass;
		float velocityBias;   // 恢复速度偏差 -e(v2-v1)
	};

	// contact速度约束
	struct b3_ContactVelocityConstraint
	{
		b3_VelocityConstraintPoint points[b3_MaxManifoldPoints];  // contact交点
		glm::vec3 normal;     // 交点法向量，单位向量
		float normalMass[6][6];
		float K[6][6];
		int indexA;              // bodyA的岛上索引
		int indexB;				 // bodyB的岛上索引
		float invMassA, invMassB;
		glm::vec3 invIA, invIB;
		float friction;            // 摩擦力
		float restitution;         // 弹性
		float threshold;           // 弹性阈值
		float tangentSpeed;// not used
		int pointCount;  // contact交点数量
		int contactIndex;// contact在island上的索引
	};

	struct b3_ContactSolverDef
	{
		b3_TimeStep step;        // 时间步
		b3_Contact** contacts;   // contact数组
		int count;               // contact数量
		b3_Position* positions;  // 位置数组，b3_Island::Solve中遍历m_bodies，将sweep.center和sweep.rotation注入positions
		b3_Velocity* velocities; // 速度数组，b3_Island::Solve中遍历m_bodies，将计算后的线速度角速度注入velocities
		b3_StackAllocator* allocator;
	};

	class b3_ContactSolver
	{
	public:
		b3_ContactSolver(b3_ContactSolverDef* def);
		~b3_ContactSolver();

		// 初始化速度约束中与位置相关的部分
		void InitializeVelocityConstraints();

		void WarmStart();
		// 结算速度约束
		void SolveVelocityConstraints();
		// 注入冲量
		void StoreImpulses();

		// 结算位置约束
		bool SolvePositionConstraints();
		// 结算TOI位置约束
		bool SolveTOIPositionConstraints(int toiIndexA, int toiIndexB);

		b3_TimeStep m_step;               // 时间步
		b3_Position* m_positions;         // 位置数组，b3_Island::Solve中遍历m_bodies，将sweep.center和sweep.rotation注入positions
		b3_Velocity* m_velocities;        // 速度数组，b3_Island::Solve中遍历m_bodies，将计算后的线速度角速度注入velocities
		b3_StackAllocator* m_allocator;
		b3_ContactPositionConstraint* m_positionConstraints; // 位置约束数组，每个contact对应一个位置约束
		b3_ContactVelocityConstraint* m_velocityConstraints; // 速度约束数组，每个contact对应一个速度约束
		b3_Contact** m_contacts;          // island的contact列表
		int m_count;                      // island的contact数量
	};

}