#pragma once

#include "b3_Math.h"
#include "b3_TimeStep.h"
#include "b3_Collision.h"

#include "glm/glm/detail/qualifier.hpp"

namespace Volcano {

	class b3_Contact;
	class b3_StackAllocator;
	struct b3_ContactPositionConstraint;

	// contact�ٶ�Լ���ĵ�
	struct b3_VelocityConstraintPoint
	{
		glm::vec3 rA;        // ʸ��A��contact��bodyA�����ĵ���������� 
		glm::vec3 rB;        // ʸ��B��contact��bodyB�����ĵ���������� 
		float normalImpulse;  // �Ǵ�͸(non-penetration)������ ��b3_ContactSolver::SolveVelocityConstraints�б���ֵ
		float tangentImpulse; // ��һ֡�����߳���
		float normalMass;
		float tangentMass;
		float velocityBias;   // �ָ��ٶ�ƫ�� -e(v2-v1)
	};

	// contact�ٶ�Լ��
	struct b3_ContactVelocityConstraint
	{
		b3_VelocityConstraintPoint points[b3_MaxManifoldPoints];  // contact����
		glm::vec3 normal;     // ���㷨��������λ����
		float normalMass[6][6];
		float K[6][6];
		int indexA;              // bodyA�ĵ�������
		int indexB;				 // bodyB�ĵ�������
		float invMassA, invMassB;
		glm::vec3 invIA, invIB;
		float friction;            // Ħ����
		float restitution;         // ����
		float threshold;           // ������ֵ
		float tangentSpeed;// not used
		int pointCount;  // contact��������
		int contactIndex;// contact��island�ϵ�����
	};

	struct b3_ContactSolverDef
	{
		b3_TimeStep step;        // ʱ�䲽
		b3_Contact** contacts;   // contact����
		int count;               // contact����
		b3_Position* positions;  // λ�����飬b3_Island::Solve�б���m_bodies����sweep.center��sweep.rotationע��positions
		b3_Velocity* velocities; // �ٶ����飬b3_Island::Solve�б���m_bodies�������������ٶȽ��ٶ�ע��velocities
		b3_StackAllocator* allocator;
	};

	class b3_ContactSolver
	{
	public:
		b3_ContactSolver(b3_ContactSolverDef* def);
		~b3_ContactSolver();

		// ��ʼ���ٶ�Լ������λ����صĲ���
		void InitializeVelocityConstraints();

		void WarmStart();
		// �����ٶ�Լ��
		void SolveVelocityConstraints();
		// ע�����
		void StoreImpulses();

		// ����λ��Լ��
		bool SolvePositionConstraints();
		// ����TOIλ��Լ��
		bool SolveTOIPositionConstraints(int toiIndexA, int toiIndexB);

		b3_TimeStep m_step;               // ʱ�䲽
		b3_Position* m_positions;         // λ�����飬b3_Island::Solve�б���m_bodies����sweep.center��sweep.rotationע��positions
		b3_Velocity* m_velocities;        // �ٶ����飬b3_Island::Solve�б���m_bodies�������������ٶȽ��ٶ�ע��velocities
		b3_StackAllocator* m_allocator;
		b3_ContactPositionConstraint* m_positionConstraints; // λ��Լ�����飬ÿ��contact��Ӧһ��λ��Լ��
		b3_ContactVelocityConstraint* m_velocityConstraints; // �ٶ�Լ�����飬ÿ��contact��Ӧһ���ٶ�Լ��
		b3_Contact** m_contacts;          // island��contact�б�
		int m_count;                      // island��contact����
	};

}