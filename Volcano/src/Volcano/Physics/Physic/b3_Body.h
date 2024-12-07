#pragma once

#include "b3_Math.h"
#include "b3_Shape.h"

namespace Volcano {

	class b3_World;
	class b3_Fixture;
	struct b3_FixtureDef;
	struct b3_JointEdge;
	struct b3_ContactEdge;

	enum class b3_BodyType 
	{
		e_staticBody = 0,  // 静态物体
		e_kinematicBody,   // 刚体
		e_dynamicBody      // 动态物体
	};

	// body definition
	struct b3_BodyDef 
	{
		b3_BodyDef()
		{
			type = b3_BodyType::e_staticBody;
			position        = { 0.0f, 0.0f, 0.0f };
			rotation        = { 0.0f, 0.0f, 0.0f };
			linearVelocity  = { 0.0f, 0.0f, 0.0f };
			angularVelocity = { 0.0f, 0.0f, 0.0f };
			linearDamping   = 0.0f;
			angularDamping  = 0.0f;
			allowSleep    = true;
			awake         = true;
			fixedRotation = false;
			bullet        = false;
			enabled       = true;
			gravityScale  = 1.0f;
		}

		// 物体类型：静态，刚体，动态 The body type: static, kinematic, or dynamic. 
		// 注意：如果动态物体的质量为零，则质量设置为1。 Note: if a dynamic body would have zero mass, the mass is set to one.
		b3_BodyType type;   
		glm::vec3 position;
		glm::vec3 rotation; // 弧度radian
		glm::vec3 linearVelocity; // 线性速度
		glm::vec3 angularVelocity;// 角速度
		float linearDamping;  // 线性阻尼
		float angularDamping; // 角阻尼
		bool allowSleep;          // 允许睡眠；如果这个Body永远不会sleep，请将此标志设置为false。请注意，这会增加CPU的使用率。
		bool awake;               // 这个Body最初是awake还是sleep
		bool fixedRotation;       // 固定旋转

		// 这是一个应该在穿过其他运动物体时被阻止的快速运动物体吗？请注意，所有物体都被运动和静态物体阻止穿过。此设置仅在动态Body上考虑。
		// @warning 您应该谨慎使用此标志，因为它会增加处理时间。
		bool bullet;
		bool enabled;
		b3_BodyUserData userData;// Use this to store application specific body data.
		float gravityScale;      // 缩放施加在该物体上的重力。Scale the gravity applied to this body.
	};

	// 物理实体
	class b3_Body 
	{
	public:

		// 创建fixture并将其附着到此body。另外，您可以直接从shape创建fixture。如果密度非零，则此函数会自动更新body的质量。直到下一个时间步才会创建联系(Contacts)。
		// @param def the fixture definition.
		// @warning This function is locked during callbacks.
		b3_Fixture* CreateFixture(const b3_FixtureDef* def);
		
		// 这是一个便利方法。如果需要设置摩擦(friction)、恢复(restitution)、用户数据或过滤等参数，请使用CreateFixture(const b3_FixtureDef* def)。
		// @param shape the shape to be cloned.
		// @param density the shape density (set to zero for static bodies).
		// @warning This function is locked during callbacks.
		b3_Fixture* CreateFixture(const b3_Shape* shape, float density);
		
		// 销毁一个fixture。这会将fixture从宽相位(broad-phase)中移除，并破坏与此fixture相关的所有触点(contacts)。
		// 如果物体是动态的，并且fixture具有正密度，这将自动调整body的质量。 
        // 当body被销毁时，附着在body上的所有fixture都会被隐式地销毁。
		// @param fixture the fixture to be removed.
		// @warning This function is locked during callbacks.
		void DestroyFixture(b3_Fixture* fixture);

		// 设置body原点的旋转和位置。操纵body的变换(transform)可能会导致非物理行为。 注：contact将在下次调用b3World::Step时更新。
		// @param position body局部原点的世界坐标系位置。
		// @param angle    世界坐标系旋转（以弧度为单位）。
		void SetTransform(const glm::vec3& position, glm::vec3 rotation);

		// @return body原点的世界变换(transform)。
		const b3_Transform& GetTransform() const;

		// @return body原点的世界位置。
		const glm::vec3& GetPosition() const;

		// @return 世界旋转角(弧度)
		glm::vec3 GetRotation() const;

		// @return 质心的世界位置。
		const glm::vec3& GetWorldCenter() const;

		// @return 质心的本地位置。
		const glm::vec3& GetLocalCenter() const;

		// 设置质心的线速度。 @param v 质心的新线速度。
		void SetLinearVelocity(const glm::vec3& v);

		// @return 质心的线速度。
		const glm::vec3& GetLinearVelocity() const;

		// 设置质心的角速度。
		// @param w(omega) 新角速度，单位 radians/second.
		void SetAngularVelocity(glm::vec3 w);

		// @return 质心的角速度，单位 radians/second.
		glm::vec3 GetAngularVelocity() const;

		// 为世界点施加力。如果不在质心处施加力，它将产生扭矩(torque)并影响角速度。这会唤醒body 
		// @param force 世界力矢量，通常以牛顿（N）为单位。 
		// @param point 施力点的世界位置。
		// @param wake 也会唤醒body
		void ApplyForce(const glm::vec3& force, const glm::vec3& point, bool wake);

		// 向质心施加力。这会唤醒body。
		// @param force 世界力矢量，通常以牛顿（N）为单位。
		// @param wake 也会唤醒body
		void ApplyForceToCenter(const glm::vec3& force, bool wake);


		// 施加扭矩。这会影响角速度，但不会影响质心的线速度。
		// @param torque 扭矩，通常以N - m为单位。
		// @param wake 也会唤醒body
		void ApplyTorque(glm::vec3 torque, bool wake);

		// 在某一点上施加冲量。这会立即修改速度。如果作用点不在质心，它还会改变角速度。这会唤醒body
		// @param impluse 世界冲量向量，通常以N-seconds或kg-m/s为单位。
		// @param point 施力点的世界位置。
		// @param wake 也会唤醒body
		void ApplyLinearImpulse(const glm::vec3& impulse, const glm::vec3& point, bool awake);

		// 对质心施加冲量。这会立即改变速度。 
		// @param impluse 世界冲量向量，通常以N-seconds或kg-m/s为单位。
		// @param wake 也会唤醒body
		void ApplyLinearImpulseToCenter(const glm::vec3& impulse, bool wake);

		// 施加角冲量。
		// @param impluse 角冲量，单位为kg*m*m/s
		// @param wake 也会唤醒body
		void ApplyAngularImpulse(float impulse, bool wake);

		// 求出body的总质量 @return质量，通常以千克kilograms（kg）为单位。
		float GetMass() const;

		// 获取物体围绕质心的惯性张量 @return 转动惯量，通常以kg-m^2为单位。
		glm::mat3 GetInertia() const;

		// 获取Body的质量数据 @return 一个包含Body的质量、惯性和中心的struct。
		b3_MassData GetMassData() const;

		// 设置质量特性以覆盖fixture的质量特性。 注：这会改变质心位置。注：创建或销毁fixture也会改变质量。如果Body不是动态的，则此功能无效。
        // @param data 表示质量特性。
		void SetMassData(const b3_MassData* data);

		// 将质量特性重置为fixture质量特性的总和。
        // 通常不需要调用此函数,除非你调用了SetMassData来覆盖质量，并且稍后想要重置质量。
		void ResetMassData();

		// 根据给定的局部坐标获取点的世界坐标。
		// @param localPoint body上相对于body原点测量(measured)的点。
		// @return 以世界坐标表示的相同点。
		glm::vec3 GetWorldPoint(const glm::vec3& localPoint) const;

		// 获取给定局部坐标向量的世界坐标。 
		// @param localVector body的一个局部向量
		// @return 世界坐标表示的相同向量。
		glm::vec3 GetWorldVector(const glm::vec3& localVector) const;

		// 给定世界点，获取相对于物体原点的局部点。 
		// @param worldPoint世界坐标系中的一个点。
		// @return 相对于物体原点的相应局部点。
		glm::vec3 GetLocalPoint(const glm::vec3& worldPoint) const;

		// 获取给定世界坐标向量的局部坐标。
		// @param worldVector 一个世界坐标向量
		// @return 局部坐标向量
		glm::vec3 GetLocalVector(const glm::vec3& worldVector) const;

		// 求出附着在这个body上的世界点的世界线速度。
		// @param worldPoint 一个世界坐标点
		// @return 一个点的世界速度
		glm::vec3 GetLinearVelocityFromWorldPoint(const glm::vec3& worldPoint) const;

		// 求出附着在这个body上的局部点的世界线速度。
		// @param localPoint 一个局部坐标点
		// @return 一个点的世界速度
		glm::vec3 GetLinearVelocityFromLocalPoint(const glm::vec3& localPoint) const;

		float GetLinearDamping() const;
		void SetLinearDamping(float linearDamping);
		float GetAngularDamping() const;
		void SetAngularDamping(float angularDamping);
		float GetGravityScale() const;
		void SetGravityScale(float scale);

		// 设置body的类型。这可能会改变质量和速度。
		void SetType(b3_BodyType type);
		b3_BodyType GetType() const;

		// 这个body是否应该被视为(treated)持续碰撞检测的子弹？
		void SetBullet(bool flag);

		// 这个body是否被视为(treated)持续碰撞检测的子弹？
		bool IsBullet() const;

		// 你可以禁止这个body睡眠。如果你禁用睡眠，body会被唤醒。
		void SetSleepingAllowed(bool flag);

		// 这个body是否允许睡眠
		bool IsSleepingAllowed() const;

		// 设置body的睡眠状态。睡眠body的CPU消耗非常低。
		// @param flag 设置为true以唤醒body，设置为false以使body进入睡眠状态。
		void SetAwake(bool flag);

		// 获取body的睡眠状态
		// @return 如果body已唤醒则返回true
		bool IsAwake() const;

		// 允许body被禁用。禁用的body不会模拟，不能被碰撞或唤醒。 
		// 如果你传递true，所有fixtures都将被添加到broad-phase。 
		// 如果你传递false，所有fixtures都将从broad-phase中删除，所有contact都将被销毁。 
		// fixtures和joints不受其他影响。您可以继续在禁用的body身上创建/销毁fixtures和joints。 
		// 禁用的body上的fixtures被隐式禁用，不会参与碰撞、光线投射或查询(query)。 连接到禁用的body的joint被隐式禁用。 
		// 一个禁用的body仍然由b3_World对象拥有，并保留在body列表中。
		void SetEnabled(bool flag);

		// 获取body的活动状态
		bool IsEnabled() const;

		// 将此body设置为固定旋转。这会导致质量重置。
		void SetFixedRotation(bool flag);

		// body是否为固定旋转
		bool IsFixedRotation() const;

		b3_Fixture* GetFixtureList();
		const b3_Fixture* GetFixtureList() const;
		b3_JointEdge* GetJointList();
		const b3_JointEdge* GetJointList() const;
		b3_ContactEdge* GetContactList();            // @warning this list changes during the time step and you may miss some collisions if you don't use b3_ContactListener.
		const b3_ContactEdge* GetContactList() const;
		b3_Body* GetNext();
		const b3_Body* GetNext() const;
		b3_BodyUserData& GetUserData();
		const b3_BodyUserData& GetUserData() const;
		b3_World* GetWorld();
		const b3_World* GetWorld() const;

		// Dump this body to a file
		//void Dump();

	private:
		friend class b3_World;
		friend class b3_Island;
		friend class b3_ContactManager;
		friend class b3_ContactSolver;

		friend class b3_DistanceJoint;

		// m_Flags
		enum
		{
			e_islandFlag        = 0x0001,
			e_awakeFlag         = 0x0002,
			e_autoSleepFlag     = 0x0004,
			e_bulletFlag        = 0x0008,
			e_fixedRotationFlag = 0x0010,
			e_enabledFlag       = 0x0020,
			e_toiFlag           = 0x0040
		};

		b3_Body(const b3_BodyDef* bd, b3_World* world);
		~b3_Body();

		// 同步Fixtures并更新到broad-phase上所有代理
		void SynchronizeFixtures();
		void SynchronizeTransform();

		// 这用于防止连接的body之间的碰撞。 它可能会撒谎，具体取决于collideConnected标志。
		bool ShouldCollide(const b3_Body* other) const;

		// 前进(Advance)到新的安全时间。这不会同步broad-phase。
		void Advance(float t);

		b3_BodyType m_type;

		uint16_t m_flags;

		int m_islandIndex;           // body上岛后的索引

		b3_Transform m_transform;	 // the body origin transform
		b3_Sweep m_sweep;		     // the swept motion for 连续碰撞检测(CCD)

		glm::vec3 m_linearVelocity;
		glm::vec3 m_angularVelocity;   // 角速度

		glm::vec3 m_force;        // 应力
		glm::vec3 m_torque;       // 扭矩

		b3_World* m_world;
		b3_Body* m_prev;
		b3_Body* m_next;

		b3_Fixture* m_fixtureList;
		int m_fixtureCount;

		b3_JointEdge* m_jointList;     // joint双链表
		b3_ContactEdge* m_contactList; // contace双链表

		float m_mass, m_invMass;// 质量，千克
		//glm::vec3 m_I, m_invI; // 质心的转动惯量  Rotational inertia about the center of mass.

		glm::mat3 m_I, m_invI;  // 惯性张量

		float m_linearDamping;   // 线性阻尼
		float m_angularDamping;  // 角速度阻尼
		float m_gravityScale;    // 重力缩放,默认1

		float m_sleepTime;       // 睡眠时间

		b3_BodyUserData m_userData;
	};

	inline const b3_Transform& b3_Body::GetTransform() const { return m_transform; }
	inline const glm::vec3& b3_Body::GetPosition() const { return m_transform.position; }
	inline glm::vec3 b3_Body::GetRotation() const { return m_sweep.rotation; }
	inline const glm::vec3& b3_Body::GetWorldCenter() const { return m_sweep.center; }
	inline const glm::vec3& b3_Body::GetLocalCenter() const { return m_sweep.localCenter; }
	inline void b3_Body::SetLinearVelocity(const glm::vec3& v)
	{
		if (m_type == b3_BodyType::e_staticBody)
		{
			return;
		}

		if (glm::dot(v, v) > 0.0f)
		{
			SetAwake(true);
		}

		m_linearVelocity = v;
	}

	inline const glm::vec3& b3_Body::GetLinearVelocity() const { return m_linearVelocity; }
	inline void b3_Body::SetAngularVelocity(glm::vec3 w)
	{
		if (m_type == b3_BodyType::e_staticBody)
		{
			return;
		}

		if (glm::dot(w, w) > 0.0f)
		{
			SetAwake(true);
		}

		m_angularVelocity = w;
	}

	inline glm::vec3 b3_Body::GetAngularVelocity() const { return m_angularVelocity; }

	inline void b3_Body::ApplyForce(const glm::vec3& force, const glm::vec3& point, bool wake)
	{
		if (m_type != b3_BodyType::e_dynamicBody)
		{
			return;
		}

		if (wake && (m_flags & e_awakeFlag) == 0)
		{
			SetAwake(true);
		}

		// Don't accumulate a force if the body is sleeping.
		if (m_flags & e_awakeFlag)
		{
			m_force += force;
			m_torque += glm::cross(point - m_sweep.center, force);
		}
	}

	inline void b3_Body::ApplyForceToCenter(const glm::vec3& force, bool wake)
	{
		if (m_type != b3_BodyType::e_dynamicBody)
		{
			return;
		}

		if (wake && (m_flags & e_awakeFlag) == 0)
		{
			SetAwake(true);
		}

		// Don't accumulate a force if the body is sleeping
		if (m_flags & e_awakeFlag)
		{
			m_force += force;
		}
	}

	inline void b3_Body::ApplyTorque(glm::vec3 torque, bool wake)
	{
		if (m_type != b3_BodyType::e_dynamicBody)
		{
			return;
		}

		if (wake && (m_flags & e_awakeFlag) == 0)
		{
			SetAwake(true);
		}

		// Don't accumulate a force if the body is sleeping
		if (m_flags & e_awakeFlag)
		{
			m_torque += torque;
		}
	}

	inline void b3_Body::ApplyLinearImpulse(const glm::vec3& impulse, const glm::vec3& point, bool awake)
	{
		if (m_type != b3_BodyType::e_dynamicBody)
		{
			return;
		}

		if (awake && (m_flags & e_awakeFlag) == 0)
		{
			SetAwake(true);
		}

		// Don't accumulate velocity if the body is sleeping
		if (m_flags & e_awakeFlag)
		{
			m_linearVelocity += m_invMass * impulse;
			m_angularVelocity += m_invI * glm::cross(point - m_sweep.center, impulse);
		}
	}

	inline void b3_Body::ApplyLinearImpulseToCenter(const glm::vec3& impulse, bool wake)
	{
		if (m_type != b3_BodyType::e_dynamicBody)
		{
			return;
		}

		if (wake && (m_flags & e_awakeFlag) == 0)
		{
			SetAwake(true);
		}

		// Don't accumulate velocity if the body is sleeping
		if (m_flags & e_awakeFlag)
		{
			m_linearVelocity += m_invMass * impulse;
		}
	}

	inline void b3_Body::ApplyAngularImpulse(float impulse, bool wake)
	{
		if (m_type != b3_BodyType::e_dynamicBody)
		{
			return;
		}

		if (wake && (m_flags & e_awakeFlag) == 0)
		{
			SetAwake(true);
		}

		// Don't accumulate velocity if the body is sleeping
		if (m_flags & e_awakeFlag)
		{
			float I = ComputeInertia(m_I, GetLocalCenter(), glm::normalize(GetRotation()));
			if (I != 0.0f)
				m_angularVelocity += 1.0f / I * impulse;
			
		}
	}

	inline float b3_Body::GetMass() const { return m_mass; }
	inline glm::mat3 b3_Body::GetInertia() const { return ComputeInertia(m_I, m_mass, m_sweep.localCenter); }
	inline b3_MassData b3_Body::GetMassData() const
	{
		b3_MassData data;
		data.mass = m_mass;
		data.center = m_sweep.localCenter;
		data.I = ComputeInertia(m_I, m_mass, m_sweep.localCenter);
		return data;
	}
	inline glm::vec3 b3_Body::GetWorldPoint(const glm::vec3& localPoint) const { return b3_Multiply(m_transform, localPoint); }
	inline glm::vec3 b3_Body::GetWorldVector(const glm::vec3& localVector) const { return b3_Multiply(m_transform.rotation, localVector); }
	inline glm::vec3 b3_Body::GetLocalPoint(const glm::vec3& worldPoint) const { return b3_MultiplyT(m_transform, worldPoint); }
	inline glm::vec3 b3_Body::GetLocalVector(const glm::vec3& worldVector) const { return b3_MultiplyT(m_transform.rotation, worldVector); }
	inline glm::vec3 b3_Body::GetLinearVelocityFromWorldPoint(const glm::vec3& worldPoint) const { return m_linearVelocity + glm::cross(m_angularVelocity, worldPoint - m_sweep.center); }
	inline glm::vec3 b3_Body::GetLinearVelocityFromLocalPoint(const glm::vec3& localPoint) const { return GetLinearVelocityFromWorldPoint(GetWorldPoint(localPoint)); }
	inline float b3_Body::GetLinearDamping() const { return m_linearDamping; }
	inline void b3_Body::SetLinearDamping(float linearDamping) { m_linearDamping = linearDamping; }
	inline float b3_Body::GetAngularDamping() const { return m_angularDamping; }
	inline void b3_Body::SetAngularDamping(float angularDamping) { m_angularDamping = angularDamping; }
	inline float b3_Body::GetGravityScale() const { return m_gravityScale; }
	inline void b3_Body::SetGravityScale(float scale) { m_gravityScale = scale; }
	inline b3_BodyType b3_Body::GetType() const { return m_type; }
	inline void b3_Body::SetBullet(bool flag)
	{
		if (flag)
		{
			m_flags |= e_bulletFlag;
		}
		else
		{
			m_flags &= ~e_bulletFlag;
		}
	}
	inline bool b3_Body::IsBullet() const { return (m_flags & e_bulletFlag) == e_bulletFlag; }
	inline void b3_Body::SetSleepingAllowed(bool flag)
	{
		if (flag)
		{
			m_flags |= e_autoSleepFlag;
		}
		else
		{
			m_flags &= ~e_autoSleepFlag;
			SetAwake(true);
		}
	}
	inline bool b3_Body::IsSleepingAllowed() const { return (m_flags & e_autoSleepFlag) == e_autoSleepFlag; }
	inline void b3_Body::SetAwake(bool flag)
	{
		// 睡眠不影响静态body
		if (m_type == b3_BodyType::e_staticBody)
		{
			return;
		}

		if (flag)
		{
			m_flags |= e_awakeFlag;
			m_sleepTime = 0.0f;
		}
		else
		{
			m_flags &= ~e_awakeFlag;
			m_sleepTime = 0.0f;
			m_linearVelocity = { 0.0f, 0.0f, 0.0f };
			m_angularVelocity = { 0.0f, 0.0f, 0.0f };
			m_force = { 0.0f, 0.0f, 0.0f };
			m_torque = { 0.0f, 0.0f, 0.0f };
		}
	}

	inline bool b3_Body::IsAwake() const { return (m_flags & e_awakeFlag) == e_awakeFlag; }
	inline bool b3_Body::IsEnabled() const { return (m_flags & e_enabledFlag) == e_enabledFlag; }
	inline bool b3_Body::IsFixedRotation() const { return (m_flags & e_fixedRotationFlag) == e_fixedRotationFlag; }
	inline b3_Fixture* b3_Body::GetFixtureList() { return m_fixtureList; }
	inline const b3_Fixture* b3_Body::GetFixtureList() const { return m_fixtureList; }
	inline b3_JointEdge* b3_Body::GetJointList() { return m_jointList; }
	inline const b3_JointEdge* b3_Body::GetJointList() const { return m_jointList; }
	inline b3_ContactEdge* b3_Body::GetContactList() { return m_contactList; }
	inline const b3_ContactEdge* b3_Body::GetContactList() const { return m_contactList; }
	inline b3_Body* b3_Body::GetNext() { return m_next; }
	inline const b3_Body* b3_Body::GetNext() const { return m_next; }
	inline b3_BodyUserData& b3_Body::GetUserData() { return m_userData; }
	inline const b3_BodyUserData& b3_Body::GetUserData() const { return m_userData; }
	inline b3_World* b3_Body::GetWorld() { return m_world; }
	inline const b3_World* b3_Body::GetWorld() const { return m_world; }

	inline void b3_Body::SynchronizeTransform()
	{
		m_transform.rotation.Set(m_sweep.rotation);
		m_transform.position = m_sweep.center - b3_Multiply(m_transform.rotation, m_sweep.localCenter);
	}

	inline void b3_Body::Advance(float alpha)
	{
		m_sweep.Advance(alpha);
		m_sweep.center = m_sweep.center0;
		m_sweep.rotation = m_sweep.rotation0;
		m_transform.rotation.Set(m_sweep.rotation);
		m_transform.position = m_sweep.center - b3_Multiply(m_transform.rotation, m_sweep.localCenter);
	}

}