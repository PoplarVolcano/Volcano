#pragma once

#include "b3_Math.h"

namespace Volcano {

	class b3_Body;
	class b3_Joint;
	class b3_BlockAllocator;
	struct b3_SolverData;

	enum b3_JointType
	{
		e_unknownJoint,     // 未知  
		e_revoluteJoint,	// 转动
		e_prismaticJoint,	// 棱柱形
		e_distanceJoint,	// 距离
		e_pulleyJoint,	    // 滑轮
		e_mouseJoint,		// 老鼠
		e_gearJoint,		// 齿轮
		e_wheelJoint,		// 轮
		e_weldJoint,		// 焊接
		e_frictionJoint,	// 摩擦
		e_motorJoint		// 马达
	};

	struct b3_Jacobian
	{
		glm::vec3 linear;
		float angularA;
		float angularB;
	};

	/*
	    关节边(joint edge)用于在关节图(joint graph)中将body和Joint连接在一起，其中每个body都是节点，每个joint都是edge。
	    关节边属于每个所依附的body中维护的双链表。每个joint有两个joint节点，每个所依附的body一个。
	*/
	struct b3_JointEdge
	{
		b3_Body* other;			// provides quick access to the other body attached.
		b3_Joint* joint;
		b3_JointEdge* prev;		// body m_jointList中前一个
		b3_JointEdge* next;		// body m_jointList中后一个
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
		b3_JointUserData userData;  // 使用此功能将特定于应用程序的数据附加到joints。
		b3_Body* bodyA;  // The first attached body.
		b3_Body* bodyB;  // The second attached body.
		bool collideConnected; // Set this flag to true if the attached bodies should collide.
	};

	// 根据频率和阻尼比计算线性刚度值和阻尼的实用程序 Utility to compute linear stiffness values from frequency and damping ratio
	//void b3_LinearStiffness(float& stiffness, float& damping, float frequencyHertz, float dampingRatio, const b3_Body* bodyA, const b3_Body* bodyB);

	// 根据频率和阻尼比计算旋转刚度值的实用程序 Utility to compute rotational stiffness values from frequency and damping ratio
	//void b3_AngularStiffness(float& stiffness, float& damping, float frequencyHertz, float dampingRatio, const b3_Body* bodyA, const b3_Body* bodyB);

	// 基础关节类。关节用于以各种方式将两个物体约束(constraint)在一起。一些关节还具有特殊限制和马达(motors)。
	class b3_Joint
	{
	public:
		b3_JointType GetType() const;
		b3_Body* GetBodyA();
		b3_Body* GetBodyB();
		virtual glm::vec3 GetAnchorA() const = 0;  // 在世界坐标系中获取bodyA的锚点
		virtual glm::vec3 GetAnchorB() const = 0;  // 在世界坐标系中获取bodyB的锚点
		virtual glm::vec3 GetReactionForce(float inv_deltaTime) const = 0;  // 获取关节锚处bodyB上的反作用力，单位为牛顿。
		virtual glm::vec3 GetReactionTorque(float inv_deltaTime) const = 0;     // 获得bodyB上的反作用扭矩(torque)，单位为N*m。
		b3_Joint* GetNext();                // Get the next joint the world joint list.
		const b3_Joint* GetNext() const;
		b3_JointUserData& GetUserData();
		const b3_JointUserData& GetUserData() const;
		bool IsEnabled() const;  // 快捷方式功能，用于确定(determine)是否启用了2个body之一。

		// 建立碰撞连接。注意：修改碰撞连接标志(collide connect flag)将不会正常工作，因为该标志仅在fixture的AABB开始重叠时才被检查。
		bool GetCollideConnected() const;

		// 将此关节转储到日志文件。 Dump this joint to the log file.
		//virtual void Dump() { b3_Dump("// Dump is not supported for this joint type.\n"); }

		// 移动存储在世界坐标系中的点的原点。 Shift the origin for any points stored in world coordinates.
		virtual void ShiftOrigin(const glm::vec3& newOriginTranslate) { (void)(newOriginTranslate); /* not used */ }

		// 调试绘制此关节 Debug draw this joint
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

		// 初始化速度约束
		virtual void InitVelocityConstraints(const b3_SolverData& data) = 0;
		// 结算速度约束
		virtual void SolveVelocityConstraints(const b3_SolverData& data) = 0;

		// 如果位置误差在公差(tolerance)范围内，则返回true。
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
		bool m_collideConnected;  // 关节的两个body之间是否碰撞

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