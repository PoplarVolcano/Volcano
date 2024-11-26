#pragma once

#include "b3_Collision.h"
#include "b3_Shape.h"
#include "b3_Setting.h"
#include "b3_Body.h"

namespace Volcano {

	class b3_World;
	class b3_Fixture;
	class b3_BroadPhase;

	struct b3_Filter
	{
		b3_Filter()
		{
			categoryBits = 0x0001;
			maskBits = 0xFFFF;
			groupIndex = 0;
		}
		uint16_t categoryBits; // 碰撞类别位。通常你只需要设置一个位。
		uint16_t maskBits;     // 碰撞掩码(mask)位。这说明了此shape将接受碰撞的类别。
		short groupIndex;      // 碰撞组允许某一组对象永远不会碰撞（负）或始终碰撞（正）。零表示没有碰撞组。非零组滤波(filtering)总是胜过掩码(mask)位。
	};

	// fixture定义(definition)用于创建fixture。此类定义了一个抽象fixture定义。您可以安全地重用fixture定义
	struct b3_FixtureDef
	{
		b3_FixtureDef()
		{
			shape = nullptr;
			friction = 0.2f;
			restitution = 0.0f;
			restitutionThreshold = 1.0f;
			density = 0.0f;
			isSensor = false;
		}

		
		const b3_Shape* shape;        // shape，必须设置。shape将被克隆，因此您可以在堆栈上创建shape。
		b3_FixtureUserData userData; // 使用此功能存储特定于应用程序的fixture数据。
		float friction;               // 摩擦系数(friction coefficient)，通常在[0,1]范围内。
		float restitution;            // 恢复力(restitution)（弹性(elasticity)）通常在[0,1]范围内。
		float restitutionThreshold;   // 恢复速度阈值(Restitution velocity threshold)，通常以m/s为单位。超过此速度的碰撞将应用恢复（将反弹(bounce)）。
		float density;                // 密度，通常以kg/m^2为单位。
		bool isSensor;                // 传感器(sensor)shape收集接触信息，但从不产生碰撞响应。
		b3_Filter filter;
	};

	// 此代理(proxy)在内部用于将fixtures连接到(broad-phase)。
	struct b3_FixtureProxy
	{
		b3_AABB aabb;
		b3_Fixture* fixture;
		int childIndex;
		int proxyId;
	};

	class b3_Fixture
	{
	public:

		// @return the shape type.
		b3_Shape::Type GetType() const;

		// 获取shape。您可以修改获取shape，但不应更改顶点数量，因为这会使某些碰撞缓存机制(caching mechanisms)崩溃。 操纵(Manipulating)shape可能会导致非物理行为。
		b3_Shape* GetShape();
		const b3_Shape* GetShape() const;

		// 设置这个fixture是否为传感器
		void SetSensor(bool sensor);
		
		// 这个fixture是否为传感器(non-solid)?
		// @return the true if the shape is a sensor.
		bool IsSensor() const;

		// 设置contact过滤器数据。当任一父body都处于活动状态并处于唤醒状态，这将不会更新contact，直到下一个时间步。这会自动调用Refilter。
		void SetFilterData(const b3_Filter& filter);

		// 获取contact过滤器数据
		const b3_Filter& GetFilterData() const;

		// 如果要建立(establish)之前由b3_ContactFilter::ShouldClide禁用的碰撞，请调用此命令。
		void Refilter();

		// 获取这个fixture的body
		// @return the parent body.
		b3_Body* GetBody();
		const b3_Body* GetBody() const;

		// 获取父body的fixture列表中的下一个fixture。
		// @return the next shape.
		b3_Fixture* GetNext();
		const b3_Fixture* GetNext() const;

		// 获取在fixture定义中指定的用户数据。使用此功能存储特定于应用程序的数据
		b3_FixtureUserData& GetUserData();
		const b3_FixtureUserData& GetUserData() const;

		// 测试一个点是否在fixture内。
		// @param p a point in world coordinates.
		bool TestPoint(const glm::vec3& point) const;

		// 光线检测这个shape
		// @param output the ray-cast results.
		// @param input the ray-cast input parameters.
		// @param childIndex the child shape index (e.g. edge index)
		bool RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input, int childIndex) const;

		// 获取此Fixture的质量数据。质量数据基于密度和shape。转动惯量与shape的原点有关。此操作可能很昂贵。
		void GetMassData(b3_MassData* massData) const;

		// 设置此fixture的密度。这不会自动调整body的质量。您必须调用b3_Body::ResetMassData来更新body的质量。
		void SetDensity(float density);

		// 获取此fixture的密度
		float GetDensity() const;

		// 获取摩擦系数(coefficient of friction).
		float GetFriction() const;

		// 设置摩擦系数(coefficient of friction)  这不会改变现有contacts的摩擦力。
		void SetFriction(float friction);

		// 得到恢复系数(coefficient of restitution).
		float GetRestitution() const;

		// 设置恢复系数(coefficient of restitution). 这不会改变现有contacts的恢复。
		void SetRestitution(float restitution);

		// 获取恢复速度阈值(restitution velocity threshold).
		float GetRestitutionThreshold() const;

		// 设置恢复速度阈值(restitution velocity threshold) 这不会改变现有contacts的恢复速度阈值。
		void SetRestitutionThreshold(float threshold);

		// 获取fixture的AABB。此AABB可能已放大(enlarge)和/或过时(stale)。 
		// 如果需要更精确(accurate)的AABB，请使用shape和body的transform进行计算。
		const b3_AABB& GetAABB(int childIndex) const;

		// Dump this fixture to the log file.
		//void Dump(int bodyIndex);
		
	protected:
		friend class b3_Body;
		friend class b3_World;
		friend class b3_Contact;
		friend class b3_ContactManager;

		b3_Fixture();

		// 我们需要将创建/销毁函数与构造函数/析构函数分离(separation)，因为析构函数无法访问(access)分配器(allocator)（C++不允许使用析构函数参数(arguments)）。
		void Create(b3_BlockAllocator* allocator, b3_Body* body, const b3_FixtureDef* def);
		// 从allocator中销毁fixture下所有代理和shape
		void Destroy(b3_BlockAllocator* allocator); 

		// 这些支持body激活/停用。 These support body activation/deactivation.
		void CreateProxies(b3_BroadPhase* broadPhase, const b3_Transform& transform);
		// 从broadPhase中销毁fixture下所有代理
		void DestroyProxies(b3_BroadPhase* broadPhase);

		// 计算fixture位置的transform1和transform2的AABB的合并AABB，更新到fixture的所有代理
		void Synchronize(b3_BroadPhase* broadPhase, const b3_Transform& transform1, const b3_Transform& transform2);

		b3_FixtureUserData m_userData;
		float m_density;    // 密度
		float m_friction;
		float m_restitution;
		float m_restitutionThreshold;
		b3_Body* m_body;
		b3_Fixture* m_next;
		b3_Filter m_filter;
		bool m_isSensor;         // 是否传感器，默认false
		b3_Shape* m_shape;
		b3_FixtureProxy* m_proxies;
		int m_proxyCount;

	};

	inline b3_Shape::Type b3_Fixture::GetType() const
	{
		return m_shape->GetType();
	}

	inline b3_Shape* b3_Fixture::GetShape()
	{
		return m_shape;
	}

	inline const b3_Shape* b3_Fixture::GetShape() const
	{
		return m_shape;
	}

	inline void Volcano::b3_Fixture::SetSensor(bool sensor)
	{
		if (sensor != m_isSensor)
		{
			m_body->SetAwake(true);
			m_isSensor = sensor;
		}
	}

	inline bool b3_Fixture::IsSensor() const
	{
		return m_isSensor;
	}

	inline void b3_Fixture::SetFilterData(const b3_Filter& filter)
	{
		m_filter = filter;

		Refilter();
	}

	inline const b3_Filter& b3_Fixture::GetFilterData() const
	{
		return m_filter;
	}

	inline b3_FixtureUserData& b3_Fixture::GetUserData()
	{
		return m_userData;
	}

	inline const b3_FixtureUserData& b3_Fixture::GetUserData() const
	{
		return m_userData;
	}

	inline bool b3_Fixture::TestPoint(const glm::vec3& point) const
	{
		return m_shape->TestPoint(m_body->GetTransform(), point);
	}

	inline bool b3_Fixture::RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input, int childIndex) const
	{
		return m_shape->RayCast(output, input, m_body->GetTransform(), childIndex);
	}

	inline void b3_Fixture::GetMassData(b3_MassData* massData) const
	{
		m_shape->ComputeMass(massData, m_density);
	}

	inline b3_Body* b3_Fixture::GetBody()
	{
		return m_body;
	}

	inline const b3_Body* b3_Fixture::GetBody() const
	{
		return m_body;
	}

	inline b3_Fixture* b3_Fixture::GetNext()
	{
		return m_next;
	}

	inline const b3_Fixture* b3_Fixture::GetNext() const
	{
		return m_next;
	}

	inline void b3_Fixture::SetDensity(float density)
	{
		assert(isfinite(density) && density >= 0.0f);
		m_density = density;
	}

	inline float b3_Fixture::GetDensity() const
	{
		return m_density;
	}

	inline float b3_Fixture::GetFriction() const
	{
		return m_friction;
	}

	inline void b3_Fixture::SetFriction(float friction)
	{
		m_friction = friction;
	}

	inline float b3_Fixture::GetRestitution() const
	{
		return m_restitution;
	}

	inline void b3_Fixture::SetRestitution(float restitution)
	{
		m_restitution = restitution;
	}

	inline float b3_Fixture::GetRestitutionThreshold() const
	{
		return m_restitutionThreshold;
	}

	inline void b3_Fixture::SetRestitutionThreshold(float threshold)
	{
		m_restitutionThreshold = threshold;
	}

	inline const b3_AABB& b3_Fixture::GetAABB(int childIndex) const
	{
		assert(0 <= childIndex && childIndex < m_proxyCount);
		return m_proxies[childIndex].aabb;
	}

}