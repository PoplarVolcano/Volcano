#pragma once

#include "b3_Joint.h"
#include "b3_TimeStep.h"

namespace Volcano {

	// Distance joint���塣����Ҫ�����������϶���һ��ê�㣬������distance joint�ķ�����롣
	// �ö���ʹ�þֲ�ê�㣬��˳�ʼ���ÿ��ܻ���΢Υ��(violate)Լ��(constraint)���������ڱ���ͼ�����Ϸ��
	struct b3_DistanceJointDef : public b3_JointDef
	{
		b3_DistanceJointDef()
		{
			type = e_distanceJoint;
			localAnchorA = { 0.0f, 0.0f, 0.0f };
			localAnchorB = { 0.0f, 0.0f, 0.0f };
			length = 1.0f;
			minLength = 0.0f;
			maxLength = FLT_MAX;
			stiffness = 0.0f;
			damping = 0.0f;
		}

		// ʹ������ռ�ê���ʼ��ʵ�塢ê��ͳ�ʼ��ࡣ��С����󳤶�����Ϊ��ʼ���
		void Initialize(b3_Body* bodyA, b3_Body* bodyB, const glm::vec3& anchorA, const glm::vec3& anchorB);
		
		glm::vec3 localAnchorA; // �����bodyAԭ��ľֲ�ê�㡣
		glm::vec3 localAnchorB; // �����bodyBԭ��ľֲ�ê�㡣
		float length;           // ����ؽڵľ�ֹ(rest)���ȡ�������(Clamped)��һ���ȶ�����Сֵ��
		float minLength;        // ��С���ȡ�������(Clamped)��һ���ȶ�����Сֵ��
		float maxLength;        // ��󳤶ȡ�������ڻ������С���ȡ�
		float stiffness;        // ���Ընȣ���λΪN/m��
		float damping;	        // �������ᣬ��λΪN*s/m��
	};

	// һ������ؽ� Լ�����������ϵ������㱣�ֱ˴˹̶��ľ��롣����Խ�����Ϊһ���������ĸ��Ը�(rod)
	class b3_DistanceJoint : public b3_Joint
	{
	public:

		glm::vec3 GetAnchorA() const override;
		glm::vec3 GetAnchorB() const override;

		// ���ݷ�ʱ�䲽��(inverse time step)�õ�����������  ��λΪN��
		glm::vec3 GetReactionForce(float int_deltaTime) const override;

		// ��ø�����ʱ�䲽��(inverse time step)�ķ�����Ť�ء���λΪN*m�����ھ���ؽڣ���ֵʼ��Ϊ�㡣
		glm::vec3 GetReactionTorque(float int_deltaTime) const override;

		// �����bodyAԭ��ľֲ�ê��
		const glm::vec3& GetLocalAnchorA() const { return m_localAnchorA; }

		// �����bodyBԭ��ľֲ�ê��
		const glm::vec3& GetLocalAnchorB() const { return m_localAnchorB; }

		// ��ȡ��ֹ(rest)����
		float GetLength() const { return m_length; }

		// ���þ�ֹ(rest)����
		// @returns clamped rest length
		float SetLength(float length);

		// ��ȡ��С����
		float GetMinLength() const { return m_minLength; }

		// ������С����
		// @returns the clamped minimum length
		float SetMinLength(float minLength);

		// ��ȡ��󳤶�
		float GetMaxLength() const { return m_maxLength; }

		// ������󳤶�
		// @returns the clamped maximum length
		float SetMaxLength(float maxLength);

		// ��ȡ���ڳ���
		float GetCurrentLength() const;

		// ����/��ȡ���Ընȣ���λΪN/m
		void SetStiffness(float stiffness) { m_stiffness = stiffness; }
		float GetStiffness() const { return m_stiffness; }

		// ����/��ȡ�������ᣬ��λΪN*s/m
		void SetDamping(float damping) { m_damping = damping; }
		float GetDamping() const { return m_damping; }

		// Dump joint to dmLog
		//void Dump() override;

		//void Draw(b3_Draw* draw) const override;

	protected:

		friend class b3_Joint;

		b3_DistanceJoint(const b3_DistanceJointDef* def);

		void InitVelocityConstraints(const b3_SolverData& data) override;
		void SolveVelocityConstraints(const b3_SolverData& data) override;
		bool SolvePositionConstraints(const b3_SolverData& data) override;

		float m_stiffness;  // �ն�
		float m_damping;    // ����
		float m_bias;       // Ӱ��
		float m_length;     // ��ֹ(rest)����
		float m_minLength;  // ��С����
		float m_maxLength;  // ��󳤶�

		// Solver shared
		glm::vec3 m_localAnchorA;// bodyA�ֲ�ê��
		glm::vec3 m_localAnchorB;// bodyB�ֲ�ê��
		float m_gamma;           // v/Ft
		float m_impulse;         // ����
		float m_lowerImpulse;    // ��С����
		float m_upperImpulse;    // ������

		// Solver temp
		int m_indexA;
		int m_indexB;
		glm::vec3 m_u;
		glm::vec3 m_rA;            // ����rotationA��ת������ԭ��a->ê��a
		glm::vec3 m_rB;            // ����rotationB��ת������ԭ��b->ê��b
		glm::vec3 m_localCenterA;
		glm::vec3 m_localCenterB;
		float m_currentLength;
		float m_invMassA;
		float m_invMassB;
		glm::vec3 m_invIA;
		glm::vec3 m_invIB;
		float m_softMass;
		float m_mass;
	};

}