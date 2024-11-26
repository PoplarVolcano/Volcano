#pragma once

#include "b3_Collision.h"
#include "b3_Shape.h"
#include "b3_Fixture.h"

namespace Volcano {

	class b3_Body;
	class b3_Contact;
	class b3_World;
	class b3_BlockAllocator;
	class b3_StackAllocator;
	class b3_ContactListener;

	// Ħ����Ϸ����������κ�һ��fixture��Ħ���������㡣  ���磬�κζ��������ڱ��ϻ�����
	inline float b3_MixFriction(float friction1, float friction2)
	{
		return sqrtf(friction1 * friction2);
	}

	// �ָ���Ϸ����������κζ����ӷǵ��Ա���(inelastic surface)����(bounce)��  ���磬������������κ������Ϸ�����
	inline float b3_MixRestitution(float restitution1, float restitution2)
	{
		return restitution1 > restitution2 ? restitution1 : restitution2;
	}

	// �ָ���Ϸ�����ѡ�����ֵ
	inline float b3_MixRestitutionThreshold(float threshold1, float threshold2)
	{
		return threshold1 < threshold2 ? threshold1 : threshold2;
	}

	typedef b3_Contact* b3_ContactCreateFcn(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB, b3_BlockAllocator* allocator);
	typedef void b3_ContactDestroyFcn(b3_Contact* contact, b3_BlockAllocator* allocator);

	// contactע����
	struct b3_ContactRegister
	{
		b3_ContactCreateFcn* createFcn;   //contactע�᷽��
		b3_ContactDestroyFcn* destroyFcn; //contact���ٷ���
		bool primary;
	};

	// �Ӵ���(contact edge)�����ڽӴ�ͼ(graph)�н�body��contacts������һ������ÿ��body���ǽڵ㣬ÿ��contacts���Ǳߡ�
	// �Ӵ�������ÿ������body��ά����˫����ÿ��contact������contact�ڵ㣬ÿ������body��Ӧһ����
	struct b3_ContactEdge
	{
		b3_Body* other;			
		b3_Contact* contact;	
		b3_ContactEdge* prev;	// body��contact�б��е�ǰһ���Ӵ���
		b3_ContactEdge* next;	// body��contact�б��е���һ���Ӵ���
	};

	// �����������shape֮��ĽӴ�����broad-phase�У�ÿ���ص�(overlapping)��AABB������һ���Ӵ�(contact)(���Ǿ����˲�)��
	// ��ˣ����ܴ���û��Contact���Contact����
	class b3_Contact
	{
	public:

		// ��ȡcontact manifold��
		b3_Manifold* GetManifold();
		const b3_Manifold* GetManifold() const;

		// ��ȡworld manifold.
		void GetWorldManifold(b3_WorldManifold* worldManifold) const;

		// ���contact�Ƿ���ײ
		bool IsTouching() const;

		// ����/���ô�contact���������Ԥ����contact��������ʹ�á����ڵ�ǰʱ�䲽����������ײ�е��Ӳ������ø�contact��
		void SetEnabled(bool flag);

		// ���contact�Ƿ�����
		bool IsEnabled() const;

		// ��ȡworld��contact�б��е���һ��
		b3_Contact* GetNext();
		const b3_Contact* GetNext() const;

		//��ȡcontact��fixtureA
		b3_Fixture* GetFixtureA();
		const b3_Fixture* GetFixtureA() const;

		// ��ȡfixtureA����ͼԪ(primitive)����
		int GetChildIndexA() const;

		//��ȡcontact��fixtureB
		b3_Fixture* GetFixtureB();
		const b3_Fixture* GetFixtureB() const;

		// ��ȡfixtureB����ͼԪ(primitive)����
		int GetChildIndexB() const;

		// ����Ĭ�ϵ�Ħ����Ϸ�������������b3_ContactListener::PreSolve�е��ô˺�����  ��ֵ��һֱ���ڣ�ֱ�����û����á�
		void SetFriction(float friction);

		// ��ȡĦ��
		float GetFriction() const;

		// ����Ħ����Ϸ���ΪĬ��
		void ResetFriction();

		// ����Ĭ�ϵĻָ���ϡ���������b3_ContactListener::PreSolve�е��ô˺�����  ��ֵ��һֱ���ڣ�ֱ�����û����á�
		void SetRestitution(float restitution);

		// ��ȡ�ָ�
		float GetRestitution() const;

		// ���ûָ�ΪĬ��
		void ResetRestitution();

		// ����Ĭ�ϵĻָ��ٶ���ֵ��ϡ���������b3_ContactListener::PreSolve�е��ô˺�����  ��ֵ��һֱ���ڣ�ֱ�������û�����Ϊֹ��
		void SetRestitutionThreshold(float threshold);

		// ��ȡ�ָ��ٶ���ֵ
		float GetRestitutionThreshold() const;

		// ���ûָ��ٶ���ֵΪĬ��
		void ResetRestitutionThreshold();

		// Ϊ���ʹ���Ϊ(conveyor belt behavior)��������������ٶȡ�����ÿ��Ϊ��λ�� 
		void SetTangentSpeed(float speed);

		// ��ȡ����������ٶȡ�����ÿ��Ϊ��λ�� 
		float GetTangentSpeed() const;

		// �����Լ���manifold��transforms������(Evaluate)����contact��
		virtual void Evaluate(b3_Manifold* manifold, const b3_Transform& transformA, const b3_Transform& transformB) = 0;

	protected:
		friend class b3_ContactManager;
		friend class b3_World;
		friend class b3_ContactSolver;
		friend class b3_Body;
		friend class b3_Fixture;

		// Flags stored in m_flags
		enum
		{
			
			e_islandFlag    = 0x0001,   // ���γ�islandʱ����contactͼʱʹ�á�      
			e_touchingFlag  = 0x0002,   // ��shapes�Ӵ�ʱ���á� ������ײ
			e_enabledFlag   = 0x0004,   // ��contact���ԣ����û�������
			e_filterFlag    = 0x0008,   // ��contact��Ҫ���ˣ���Ϊ������fixture������
			e_bulletHitFlag = 0x0010,   // ����ӵ�contact������TOI�¼�
			e_toiFlag       = 0x0020    // ��contact��m_TOI�о�����Ч��TOI
		};

		// ��Ǵ�contact���й��ˡ����˽�����һʱ�䲽���С�
		void FlagForFiltering();

		static void AddType(b3_ContactCreateFcn* createFcn, b3_ContactDestroyFcn* destroyFcn, b3_Shape::Type typeA, b3_Shape::Type typeB);
		static void InitializeRegisters();
		static b3_Contact* Create(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB, b3_BlockAllocator* allocator);
		//static void Destroy(b3_Contact* contact, b3_Shape::Type typeA, b3_Shape::Type typeB, b3_BlockAllocator* allocator);
		static void Destroy(b3_Contact* contact, b3_BlockAllocator* allocator);

		b3_Contact() : m_fixtureA(nullptr), m_fixtureB(nullptr) {}
		b3_Contact(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB);
		virtual ~b3_Contact() {}

		// ����contact manifold�ͽӴ�״̬��  ע�⣺��Ҫ����fixture��AABB�ص�����Ч��
		void Update(b3_ContactListener* listener);

		// ��������֮���contact��ע���
		static b3_ContactRegister s_registers[b3_Shape::e_typeCount][b3_Shape::e_typeCount];
		static bool s_initialized;

		uint32_t m_flags;

		// World pool and list pointers.
		b3_Contact* m_prev;
		b3_Contact* m_next;

		// Nodes for connecting bodies.
		b3_ContactEdge m_nodeA;
		b3_ContactEdge m_nodeB;

		b3_Fixture* m_fixtureA;
		b3_Fixture* m_fixtureB;

		int m_indexA;
		int m_indexB;

		b3_Manifold m_manifold;

		int m_toiCount;
		float m_toi;

		float m_friction;
		float m_restitution;
		float m_restitutionThreshold;

		float m_tangentSpeed;
	};

	inline b3_Manifold* b3_Contact::GetManifold()
	{
		return &m_manifold;
	}

	inline const b3_Manifold* b3_Contact::GetManifold() const
	{
		return &m_manifold;
	}

	inline void b3_Contact::GetWorldManifold(b3_WorldManifold* worldManifold) const
	{
		const b3_Body* bodyA = m_fixtureA->GetBody();
		const b3_Body* bodyB = m_fixtureB->GetBody();
		const b3_Shape* shapeA = m_fixtureA->GetShape();
		const b3_Shape* shapeB = m_fixtureB->GetShape();

		worldManifold->Initialize(&m_manifold, bodyA->GetTransform(), shapeA->m_radius, bodyB->GetTransform(), shapeB->m_radius);
	}

	inline bool b3_Contact::IsTouching() const
	{
		return (m_flags & e_touchingFlag) == e_touchingFlag;
	}

	inline void b3_Contact::SetEnabled(bool flag)
	{
		if (flag)
		{
			m_flags |= e_enabledFlag;
		}
		else
		{
			m_flags &= ~e_enabledFlag;
		}
	}

	inline bool b3_Contact::IsEnabled() const
	{
		return (m_flags & e_enabledFlag) == e_enabledFlag;
	}

	inline b3_Contact* b3_Contact::GetNext()
	{
		return m_next;
	}

	inline const b3_Contact* b3_Contact::GetNext() const
	{
		return m_next;
	}

	inline b3_Fixture* b3_Contact::GetFixtureA()
	{
		return m_fixtureA;
	}

	inline const b3_Fixture* b3_Contact::GetFixtureA() const
	{
		return m_fixtureA;
	}

	inline b3_Fixture* b3_Contact::GetFixtureB()
	{
		return m_fixtureB;
	}

	inline int b3_Contact::GetChildIndexA() const
	{
		return m_indexA;
	}

	inline const b3_Fixture* b3_Contact::GetFixtureB() const
	{
		return m_fixtureB;
	}

	inline int b3_Contact::GetChildIndexB() const
	{
		return m_indexB;
	}

	inline void b3_Contact::SetFriction(float friction)
	{
		m_friction = friction;
	}

	inline float b3_Contact::GetFriction() const
	{
		return m_friction;
	}

	inline void b3_Contact::ResetFriction()
	{
		m_friction = b3_MixFriction(m_fixtureA->m_friction, m_fixtureB->m_friction);
	}

	inline void b3_Contact::SetRestitution(float restitution)
	{
		m_restitution = restitution;
	}

	inline float b3_Contact::GetRestitution() const
	{
		return m_restitution;
	}

	inline void b3_Contact::ResetRestitution()
	{
		m_restitution = b3_MixRestitution(m_fixtureA->m_restitution, m_fixtureB->m_restitution);
	}

	inline void b3_Contact::SetRestitutionThreshold(float threshold)
	{
		m_restitutionThreshold = threshold;
	}

	inline float b3_Contact::GetRestitutionThreshold() const
	{
		return m_restitutionThreshold;
	}

	inline void b3_Contact::ResetRestitutionThreshold()
	{
		m_restitutionThreshold = b3_MixRestitutionThreshold(m_fixtureA->m_restitutionThreshold, m_fixtureB->m_restitutionThreshold);
	}

	inline void b3_Contact::SetTangentSpeed(float speed)
	{
		m_tangentSpeed = speed;
	}

	inline float b3_Contact::GetTangentSpeed() const
	{
		return m_tangentSpeed;
	}

	inline void b3_Contact::FlagForFiltering()
	{
		m_flags |= e_filterFlag;
	}

}