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
		e_staticBody = 0,  // ��̬����
		e_kinematicBody,   // ����
		e_dynamicBody      // ��̬����
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

		// �������ͣ���̬�����壬��̬ The body type: static, kinematic, or dynamic. 
		// ע�⣺�����̬���������Ϊ�㣬����������Ϊ1�� Note: if a dynamic body would have zero mass, the mass is set to one.
		b3_BodyType type;   
		glm::vec3 position;
		glm::vec3 rotation; // ����radian
		glm::vec3 linearVelocity; // �����ٶ�
		glm::vec3 angularVelocity;// ���ٶ�
		float linearDamping;  // ��������
		float angularDamping; // ������
		bool allowSleep;          // ����˯�ߣ�������Body��Զ����sleep���뽫�˱�־����Ϊfalse����ע�⣬�������CPU��ʹ���ʡ�
		bool awake;               // ���Body�����awake����sleep
		bool fixedRotation;       // �̶���ת

		// ����һ��Ӧ���ڴ��������˶�����ʱ����ֹ�Ŀ����˶���������ע�⣬�������嶼���˶��;�̬������ֹ�����������ý��ڶ�̬Body�Ͽ��ǡ�
		// @warning ��Ӧ�ý���ʹ�ô˱�־����Ϊ�������Ӵ���ʱ�䡣
		bool bullet;
		bool enabled;
		b3_BodyUserData userData;// Use this to store application specific body data.
		float gravityScale;      // ����ʩ���ڸ������ϵ�������Scale the gravity applied to this body.
	};

	// ����ʵ��
	class b3_Body 
	{
	public:

		// ����fixture�����丽�ŵ���body�����⣬������ֱ�Ӵ�shape����fixture������ܶȷ��㣬��˺������Զ�����body��������ֱ����һ��ʱ�䲽�Żᴴ����ϵ(Contacts)��
		// @param def the fixture definition.
		// @warning This function is locked during callbacks.
		b3_Fixture* CreateFixture(const b3_FixtureDef* def);
		
		// ����һ�����������������Ҫ����Ħ��(friction)���ָ�(restitution)���û����ݻ���˵Ȳ�������ʹ��CreateFixture(const b3_FixtureDef* def)��
		// @param shape the shape to be cloned.
		// @param density the shape density (set to zero for static bodies).
		// @warning This function is locked during callbacks.
		b3_Fixture* CreateFixture(const b3_Shape* shape, float density);
		
		// ����һ��fixture����Ὣfixture�ӿ���λ(broad-phase)���Ƴ������ƻ����fixture��ص����д���(contacts)��
		// ��������Ƕ�̬�ģ�����fixture�������ܶȣ��⽫�Զ�����body�������� 
        // ��body������ʱ��������body�ϵ�����fixture���ᱻ��ʽ�����١�
		// @param fixture the fixture to be removed.
		// @warning This function is locked during callbacks.
		void DestroyFixture(b3_Fixture* fixture);

		// ����bodyԭ�����ת��λ�á�����body�ı任(transform)���ܻᵼ�·�������Ϊ�� ע��contact�����´ε���b3World::Stepʱ���¡�
		// @param position body�ֲ�ԭ�����������ϵλ�á�
		// @param angle    ��������ϵ��ת���Ի���Ϊ��λ����
		void SetTransform(const glm::vec3& position, glm::vec3 rotation);

		// @return bodyԭ�������任(transform)��
		const b3_Transform& GetTransform() const;

		// @return bodyԭ�������λ�á�
		const glm::vec3& GetPosition() const;

		// @return ������ת��(����)
		glm::vec3 GetRotation() const;

		// @return ���ĵ�����λ�á�
		const glm::vec3& GetWorldCenter() const;

		// @return ���ĵı���λ�á�
		const glm::vec3& GetLocalCenter() const;

		// �������ĵ����ٶȡ� @param v ���ĵ������ٶȡ�
		void SetLinearVelocity(const glm::vec3& v);

		// @return ���ĵ����ٶȡ�
		const glm::vec3& GetLinearVelocity() const;

		// �������ĵĽ��ٶȡ�
		// @param w(omega) �½��ٶȣ���λ radians/second.
		void SetAngularVelocity(glm::vec3 w);

		// @return ���ĵĽ��ٶȣ���λ radians/second.
		glm::vec3 GetAngularVelocity() const;

		// Ϊ�����ʩ����������������Ĵ�ʩ��������������Ť��(torque)��Ӱ����ٶȡ���ỽ��body 
		// @param force ������ʸ����ͨ����ţ�٣�N��Ϊ��λ�� 
		// @param point ʩ���������λ�á�
		// @param wake Ҳ�ỽ��body
		void ApplyForce(const glm::vec3& force, const glm::vec3& point, bool wake);

		// ������ʩ��������ỽ��body��
		// @param force ������ʸ����ͨ����ţ�٣�N��Ϊ��λ��
		// @param wake Ҳ�ỽ��body
		void ApplyForceToCenter(const glm::vec3& force, bool wake);


		// ʩ��Ť�ء����Ӱ����ٶȣ�������Ӱ�����ĵ����ٶȡ�
		// @param torque Ť�أ�ͨ����N - mΪ��λ��
		// @param wake Ҳ�ỽ��body
		void ApplyTorque(glm::vec3 torque, bool wake);

		// ��ĳһ����ʩ�ӳ�������������޸��ٶȡ�������õ㲻�����ģ�������ı���ٶȡ���ỽ��body
		// @param impluse �������������ͨ����N-seconds��kg-m/sΪ��λ��
		// @param point ʩ���������λ�á�
		// @param wake Ҳ�ỽ��body
		void ApplyLinearImpulse(const glm::vec3& impulse, const glm::vec3& point, bool awake);

		// ������ʩ�ӳ�������������ı��ٶȡ� 
		// @param impluse �������������ͨ����N-seconds��kg-m/sΪ��λ��
		// @param wake Ҳ�ỽ��body
		void ApplyLinearImpulseToCenter(const glm::vec3& impulse, bool wake);

		// ʩ�ӽǳ�����
		// @param impluse �ǳ�������λΪkg*m*m/s
		// @param wake Ҳ�ỽ��body
		void ApplyAngularImpulse(float impulse, bool wake);

		// ���body�������� @return������ͨ����ǧ��kilograms��kg��Ϊ��λ��
		float GetMass() const;

		// ��ȡ����Χ�����ĵĹ������� @return ת��������ͨ����kg-m^2Ϊ��λ��
		glm::mat3 GetInertia() const;

		// ��ȡBody���������� @return һ������Body�����������Ժ����ĵ�struct��
		b3_MassData GetMassData() const;

		// �������������Ը���fixture���������ԡ� ע�����ı�����λ�á�ע������������fixtureҲ��ı����������Body���Ƕ�̬�ģ���˹�����Ч��
        // @param data ��ʾ�������ԡ�
		void SetMassData(const b3_MassData* data);

		// ��������������Ϊfixture�������Ե��ܺ͡�
        // ͨ������Ҫ���ô˺���,�����������SetMassData�����������������Ժ���Ҫ����������
		void ResetMassData();

		// ���ݸ����ľֲ������ȡ����������ꡣ
		// @param localPoint body�������bodyԭ�����(measured)�ĵ㡣
		// @return �����������ʾ����ͬ�㡣
		glm::vec3 GetWorldPoint(const glm::vec3& localPoint) const;

		// ��ȡ�����ֲ������������������ꡣ 
		// @param localVector body��һ���ֲ�����
		// @return ���������ʾ����ͬ������
		glm::vec3 GetWorldVector(const glm::vec3& localVector) const;

		// ��������㣬��ȡ���������ԭ��ľֲ��㡣 
		// @param worldPoint��������ϵ�е�һ���㡣
		// @return ���������ԭ�����Ӧ�ֲ��㡣
		glm::vec3 GetLocalPoint(const glm::vec3& worldPoint) const;

		// ��ȡ�����������������ľֲ����ꡣ
		// @param worldVector һ��������������
		// @return �ֲ���������
		glm::vec3 GetLocalVector(const glm::vec3& worldVector) const;

		// ������������body�ϵ��������������ٶȡ�
		// @param worldPoint һ�����������
		// @return һ����������ٶ�
		glm::vec3 GetLinearVelocityFromWorldPoint(const glm::vec3& worldPoint) const;

		// ������������body�ϵľֲ�����������ٶȡ�
		// @param localPoint һ���ֲ������
		// @return һ����������ٶ�
		glm::vec3 GetLinearVelocityFromLocalPoint(const glm::vec3& localPoint) const;

		float GetLinearDamping() const;
		void SetLinearDamping(float linearDamping);
		float GetAngularDamping() const;
		void SetAngularDamping(float angularDamping);
		float GetGravityScale() const;
		void SetGravityScale(float scale);

		// ����body�����͡�����ܻ�ı��������ٶȡ�
		void SetType(b3_BodyType type);
		b3_BodyType GetType() const;

		// ���body�Ƿ�Ӧ�ñ���Ϊ(treated)������ײ�����ӵ���
		void SetBullet(bool flag);

		// ���body�Ƿ���Ϊ(treated)������ײ�����ӵ���
		bool IsBullet() const;

		// ����Խ�ֹ���body˯�ߡ���������˯�ߣ�body�ᱻ���ѡ�
		void SetSleepingAllowed(bool flag);

		// ���body�Ƿ�����˯��
		bool IsSleepingAllowed() const;

		// ����body��˯��״̬��˯��body��CPU���ķǳ��͡�
		// @param flag ����Ϊtrue�Ի���body������Ϊfalse��ʹbody����˯��״̬��
		void SetAwake(bool flag);

		// ��ȡbody��˯��״̬
		// @return ���body�ѻ����򷵻�true
		bool IsAwake() const;

		// ����body�����á����õ�body����ģ�⣬���ܱ���ײ���ѡ� 
		// ����㴫��true������fixtures��������ӵ�broad-phase�� 
		// ����㴫��false������fixtures������broad-phase��ɾ��������contact���������١� 
		// fixtures��joints��������Ӱ�졣�����Լ����ڽ��õ�body���ϴ���/����fixtures��joints�� 
		// ���õ�body�ϵ�fixtures����ʽ���ã����������ײ������Ͷ����ѯ(query)�� ���ӵ����õ�body��joint����ʽ���á� 
		// һ�����õ�body��Ȼ��b3_World����ӵ�У���������body�б��С�
		void SetEnabled(bool flag);

		// ��ȡbody�Ļ״̬
		bool IsEnabled() const;

		// ����body����Ϊ�̶���ת����ᵼ���������á�
		void SetFixedRotation(bool flag);

		// body�Ƿ�Ϊ�̶���ת
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

		// ͬ��Fixtures�����µ�broad-phase�����д���
		void SynchronizeFixtures();
		void SynchronizeTransform();

		// �����ڷ�ֹ���ӵ�body֮�����ײ�� �����ܻ����ѣ�����ȡ����collideConnected��־��
		bool ShouldCollide(const b3_Body* other) const;

		// ǰ��(Advance)���µİ�ȫʱ�䡣�ⲻ��ͬ��broad-phase��
		void Advance(float t);

		b3_BodyType m_type;

		uint16_t m_flags;

		int m_islandIndex;           // body�ϵ��������

		b3_Transform m_transform;	 // the body origin transform
		b3_Sweep m_sweep;		     // the swept motion for ������ײ���(CCD)

		glm::vec3 m_linearVelocity;
		glm::vec3 m_angularVelocity;   // ���ٶ�

		glm::vec3 m_force;        // Ӧ��
		glm::vec3 m_torque;       // Ť��

		b3_World* m_world;
		b3_Body* m_prev;
		b3_Body* m_next;

		b3_Fixture* m_fixtureList;
		int m_fixtureCount;

		b3_JointEdge* m_jointList;     // joint˫����
		b3_ContactEdge* m_contactList; // contace˫����

		float m_mass, m_invMass;// ������ǧ��
		//glm::vec3 m_I, m_invI; // ���ĵ�ת������  Rotational inertia about the center of mass.

		glm::mat3 m_I, m_invI;  // ��������

		float m_linearDamping;   // ��������
		float m_angularDamping;  // ���ٶ�����
		float m_gravityScale;    // ��������,Ĭ��1

		float m_sleepTime;       // ˯��ʱ��

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
		// ˯�߲�Ӱ�쾲̬body
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