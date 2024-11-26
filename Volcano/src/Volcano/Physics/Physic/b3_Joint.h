#pragma once

#include "b3_Math.h"

namespace Volcano {

	class b3_Body;
	class b3_Joint;
	class b3_BlockAllocator;
	struct b3_SolverData;

	enum b3_JointType
	{
		e_unknownJoint,     // δ֪  
		e_revoluteJoint,	// ת��
		e_prismaticJoint,	// ������
		e_distanceJoint,	// ����
		e_pulleyJoint,	    // ����
		e_mouseJoint,		// ����
		e_gearJoint,		// ����
		e_wheelJoint,		// ��
		e_weldJoint,		// ����
		e_frictionJoint,	// Ħ��
		e_motorJoint		// ���
	};

	struct b3_Jacobian
	{
		glm::vec3 linear;
		float angularA;
		float angularB;
	};

	/*
	    �ؽڱ�(joint edge)�����ڹؽ�ͼ(joint graph)�н�body��Joint������һ������ÿ��body���ǽڵ㣬ÿ��joint����edge��
	    �ؽڱ�����ÿ����������body��ά����˫����ÿ��joint������joint�ڵ㣬ÿ����������bodyһ����
	*/
	struct b3_JointEdge
	{
		b3_Body* other;			// provides quick access to the other body attached.
		b3_Joint* joint;
		b3_JointEdge* prev;		// body m_jointList��ǰһ��
		b3_JointEdge* next;		// body m_jointList�к�һ��
	};

	// Joint definitions
	struct b3_JointDef
	{
		b3_JointDef()
		{
			type = e_unknownJoint;
			bodyA = nullptr;
			bodyB = nullptr;
			collideConnected = false;
		}

		b3_JointType type;
		b3_JointUserData userData;  // ʹ�ô˹��ܽ��ض���Ӧ�ó�������ݸ��ӵ�joints��
		b3_Body* bodyA;  // The first attached body.
		b3_Body* bodyB;  // The second attached body.
		bool collideConnected; // Set this flag to true if the attached bodies should collide.
	};

	// ����Ƶ�ʺ�����ȼ������Ըն�ֵ�������ʵ�ó��� Utility to compute linear stiffness values from frequency and damping ratio
	//void b3_LinearStiffness(float& stiffness, float& damping, float frequencyHertz, float dampingRatio, const b3_Body* bodyA, const b3_Body* bodyB);

	// ����Ƶ�ʺ�����ȼ�����ת�ն�ֵ��ʵ�ó��� Utility to compute rotational stiffness values from frequency and damping ratio
	//void b3_AngularStiffness(float& stiffness, float& damping, float frequencyHertz, float dampingRatio, const b3_Body* bodyA, const b3_Body* bodyB);

	// �����ؽ��ࡣ�ؽ������Ը��ַ�ʽ����������Լ��(constraint)��һ��һЩ�ؽڻ������������ƺ����(motors)��
	class b3_Joint
	{
	public:
		b3_JointType GetType() const;
		b3_Body* GetBodyA();
		b3_Body* GetBodyB();
		virtual glm::vec3 GetAnchorA() const = 0;  // ����������ϵ�л�ȡbodyA��ê��
		virtual glm::vec3 GetAnchorB() const = 0;  // ����������ϵ�л�ȡbodyB��ê��
		virtual glm::vec3 GetReactionForce(float inv_deltaTime) const = 0;  // ��ȡ�ؽ�ê��bodyB�ϵķ�����������λΪţ�١�
		virtual glm::vec3 GetReactionTorque(float inv_deltaTime) const = 0;     // ���bodyB�ϵķ�����Ť��(torque)����λΪN*m��
		b3_Joint* GetNext();                // Get the next joint the world joint list.
		const b3_Joint* GetNext() const;
		b3_JointUserData& GetUserData();
		const b3_JointUserData& GetUserData() const;
		bool IsEnabled() const;  // ��ݷ�ʽ���ܣ�����ȷ��(determine)�Ƿ�������2��body֮һ��

		// ������ײ���ӡ�ע�⣺�޸���ײ���ӱ�־(collide connect flag)������������������Ϊ�ñ�־����fixture��AABB��ʼ�ص�ʱ�ű���顣
		bool GetCollideConnected() const;

		// ���˹ؽ�ת������־�ļ��� Dump this joint to the log file.
		//virtual void Dump() { b3_Dump("// Dump is not supported for this joint type.\n"); }

		// �ƶ��洢����������ϵ�еĵ��ԭ�㡣 Shift the origin for any points stored in world coordinates.
		virtual void ShiftOrigin(const glm::vec3& newOriginTranslate) { (void)(newOriginTranslate); /* not used */ }

		// ���Ի��ƴ˹ؽ� Debug draw this joint
		//virtual void Draw(b3_Draw* draw) const;

	protected:
		friend class b3_World;
		friend class b3_Body;
		friend class b3_Island;
		//friend class b3_GearJoint;

		static b3_Joint* Create(const b3_JointDef* def, b3_BlockAllocator* allocator);
		static void Destroy(b3_Joint* joint, b3_BlockAllocator* allocator);

		b3_Joint(const b3_JointDef* def);
		virtual ~b3_Joint() {}

		// ��ʼ���ٶ�Լ��
		virtual void InitVelocityConstraints(const b3_SolverData& data) = 0;
		// �����ٶ�Լ��
		virtual void SolveVelocityConstraints(const b3_SolverData& data) = 0;

		// ���λ������ڹ���(tolerance)��Χ�ڣ��򷵻�true��
		virtual bool SolvePositionConstraints(const b3_SolverData& data) = 0;

		b3_JointType m_type;
		b3_Joint* m_prev;
		b3_Joint* m_next;
		b3_JointEdge m_edgeA;
		b3_JointEdge m_edgeB;
		b3_Body* m_bodyA;
		b3_Body* m_bodyB;

		int m_index;

		bool m_islandFlag;
		bool m_collideConnected;  // �ؽڵ�����body֮���Ƿ���ײ

		b3_JointUserData m_userData;
	};

	inline b3_JointType b3_Joint::GetType() const
	{
		return m_type;
	}

	inline b3_Body* b3_Joint::GetBodyA()
	{
		return m_bodyA;
	}

	inline b3_Body* b3_Joint::GetBodyB()
	{
		return m_bodyB;
	}

	inline b3_Joint* b3_Joint::GetNext()
	{
		return m_next;
	}

	inline const b3_Joint* b3_Joint::GetNext() const
	{
		return m_next;
	}

	inline b3_JointUserData& b3_Joint::GetUserData()
	{
		return m_userData;
	}

	inline const b3_JointUserData& b3_Joint::GetUserData() const
	{
		return m_userData;
	}

	inline bool b3_Joint::GetCollideConnected() const
	{
		return m_collideConnected;
	}

}