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

	// 摩擦混合方法。允许任何一个fixture将摩擦力降至零。  例如，任何东西都能在冰上滑动。
	inline float b3_MixFriction(float friction1, float friction2)
	{
		return sqrtf(friction1 * friction2);
	}

	// 恢复混合方法。允许任何东西从非弹性表面(inelastic surface)反弹(bounce)。  例如，超级球可以在任何物体上反弹。
	inline float b3_MixRestitution(float restitution1, float restitution2)
	{
		return restitution1 > restitution2 ? restitution1 : restitution2;
	}

	// 恢复混合方法。选择最低值
	inline float b3_MixRestitutionThreshold(float threshold1, float threshold2)
	{
		return threshold1 < threshold2 ? threshold1 : threshold2;
	}

	typedef b3_Contact* b3_ContactCreateFcn(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB, b3_BlockAllocator* allocator);
	typedef void b3_ContactDestroyFcn(b3_Contact* contact, b3_BlockAllocator* allocator);

	// contact注册器
	struct b3_ContactRegister
	{
		b3_ContactCreateFcn* createFcn;   //contact注册方法
		b3_ContactDestroyFcn* destroyFcn; //contact销毁方法
		bool primary;
	};

	// 接触边(contact edge)用于在接触图(graph)中将body和contacts连接在一起，其中每个body都是节点，每个contacts都是边。
	// 接触边属于每个关联body中维护的双链表。每个contact有两个contact节点，每个关联body对应一个。
	struct b3_ContactEdge
	{
		b3_Body* other;			
		b3_Contact* contact;	
		b3_ContactEdge* prev;	// body的contact列表中的前一个接触边
		b3_ContactEdge* next;	// body的contact列表中的下一个接触边
	};

	// 该类管理两个shape之间的接触。在broad-phase中，每个重叠(overlapping)的AABB都存在一个接触(contact)(除非经过滤波)。
	// 因此，可能存在没有Contact点的Contact对象。
	class b3_Contact
	{
	public:

		// 获取contact manifold。
		b3_Manifold* GetManifold();
		const b3_Manifold* GetManifold() const;

		// 获取world manifold.
		void GetWorldManifold(b3_WorldManifold* worldManifold) const;

		// 这个contact是否碰撞
		bool IsTouching() const;

		// 启用/禁用此contact。这可以在预结算contact监听器中使用。仅在当前时间步（或连续碰撞中的子步）禁用该contact。
		void SetEnabled(bool flag);

		// 这个contact是否启用
		bool IsEnabled() const;

		// 获取world的contact列表中的下一个
		b3_Contact* GetNext();
		const b3_Contact* GetNext() const;

		//获取contact的fixtureA
		b3_Fixture* GetFixtureA();
		const b3_Fixture* GetFixtureA() const;

		// 获取fixtureA的子图元(primitive)索引
		int GetChildIndexA() const;

		//获取contact的fixtureB
		b3_Fixture* GetFixtureB();
		const b3_Fixture* GetFixtureB() const;

		// 获取fixtureB的子图元(primitive)索引
		int GetChildIndexB() const;

		// 覆盖默认的摩擦混合方法。您可以在b3_ContactListener::PreSolve中调用此函数。  此值将一直存在，直到设置或重置。
		void SetFriction(float friction);

		// 获取摩擦
		float GetFriction() const;

		// 重置摩擦混合方法为默认
		void ResetFriction();

		// 覆盖默认的恢复混合。您可以在b3_ContactListener::PreSolve中调用此函数。  该值将一直存在，直到设置或重置。
		void SetRestitution(float restitution);

		// 获取恢复
		float GetRestitution() const;

		// 重置恢复为默认
		void ResetRestitution();

		// 覆盖默认的恢复速度阈值混合。您可以在b3_ContactListener::PreSolve中调用此函数。  该值将一直存在，直到您设置或重置为止。
		void SetRestitutionThreshold(float threshold);

		// 获取恢复速度阈值
		float GetRestitutionThreshold() const;

		// 重置恢复速度阈值为默认
		void ResetRestitutionThreshold();

		// 为传送带行为(conveyor belt behavior)设置所需的切线速度。以米每秒为单位。 
		void SetTangentSpeed(float speed);

		// 获取所需的切线速度。以米每秒为单位。 
		float GetTangentSpeed() const;

		// 用你自己的manifold和transforms来评估(Evaluate)这种contact。
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
			
			e_islandFlag    = 0x0001,   // 在形成island时遍历contact图时使用。      
			e_touchingFlag  = 0x0002,   // 当shapes接触时设置。 发生碰撞
			e_enabledFlag   = 0x0004,   // 此contact可以（由用户）禁用
			e_filterFlag    = 0x0008,   // 此contact需要过滤，因为更换了fixture过滤器
			e_bulletHitFlag = 0x0010,   // 这次子弹contact发生了TOI事件
			e_toiFlag       = 0x0020    // 此contact在m_TOI中具有有效的TOI
		};

		// 标记此contact进行过滤。过滤将在下一时间步进行。
		void FlagForFiltering();

		static void AddType(b3_ContactCreateFcn* createFcn, b3_ContactDestroyFcn* destroyFcn, b3_Shape::Type typeA, b3_Shape::Type typeB);
		static void InitializeRegisters();
		static b3_Contact* Create(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB, b3_BlockAllocator* allocator);
		//static void Destroy(b3_Contact* contact, b3_Shape::Type typeA, b3_Shape::Type typeB, b3_BlockAllocator* allocator);
		static void Destroy(b3_Contact* contact, b3_BlockAllocator* allocator);

		b3_Contact() : m_fixtureA(nullptr), m_fixtureB(nullptr) {}
		b3_Contact(b3_Fixture* fixtureA, int indexA, b3_Fixture* fixtureB, int indexB);
		virtual ~b3_Contact() {}

		// 更新contact manifold和接触状态。  注意：不要假设fixture的AABB重叠或有效。
		void Update(b3_ContactListener* listener);

		// 各种类型之间的contact的注册表
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