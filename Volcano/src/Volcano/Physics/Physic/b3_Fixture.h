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
		uint16_t categoryBits; // ��ײ���λ��ͨ����ֻ��Ҫ����һ��λ��
		uint16_t maskBits;     // ��ײ����(mask)λ����˵���˴�shape��������ײ�����
		short groupIndex;      // ��ײ������ĳһ�������Զ������ײ��������ʼ����ײ�����������ʾû����ײ�顣�������˲�(filtering)����ʤ������(mask)λ��
	};

	// fixture����(definition)���ڴ���fixture�����ඨ����һ������fixture���塣�����԰�ȫ������fixture����
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

		
		const b3_Shape* shape;        // shape���������á�shape������¡������������ڶ�ջ�ϴ���shape��
		b3_FixtureUserData userData; // ʹ�ô˹��ܴ洢�ض���Ӧ�ó����fixture���ݡ�
		float friction;               // Ħ��ϵ��(friction coefficient)��ͨ����[0,1]��Χ�ڡ�
		float restitution;            // �ָ���(restitution)������(elasticity)��ͨ����[0,1]��Χ�ڡ�
		float restitutionThreshold;   // �ָ��ٶ���ֵ(Restitution velocity threshold)��ͨ����m/sΪ��λ���������ٶȵ���ײ��Ӧ�ûָ���������(bounce)����
		float density;                // �ܶȣ�ͨ����kg/m^2Ϊ��λ��
		bool isSensor;                // ������(sensor)shape�ռ��Ӵ���Ϣ�����Ӳ�������ײ��Ӧ��
		b3_Filter filter;
	};

	// �˴���(proxy)���ڲ����ڽ�fixtures���ӵ�(broad-phase)��
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

		// ��ȡshape���������޸Ļ�ȡshape������Ӧ���Ķ�����������Ϊ���ʹĳЩ��ײ�������(caching mechanisms)������ ����(Manipulating)shape���ܻᵼ�·�������Ϊ��
		b3_Shape* GetShape();
		const b3_Shape* GetShape() const;

		// �������fixture�Ƿ�Ϊ������
		void SetSensor(bool sensor);
		
		// ���fixture�Ƿ�Ϊ������(non-solid)?
		// @return the true if the shape is a sensor.
		bool IsSensor() const;

		// ����contact���������ݡ�����һ��body�����ڻ״̬�����ڻ���״̬���⽫�������contact��ֱ����һ��ʱ�䲽������Զ�����Refilter��
		void SetFilterData(const b3_Filter& filter);

		// ��ȡcontact����������
		const b3_Filter& GetFilterData() const;

		// ���Ҫ����(establish)֮ǰ��b3_ContactFilter::ShouldClide���õ���ײ������ô����
		void Refilter();

		// ��ȡ���fixture��body
		// @return the parent body.
		b3_Body* GetBody();
		const b3_Body* GetBody() const;

		// ��ȡ��body��fixture�б��е���һ��fixture��
		// @return the next shape.
		b3_Fixture* GetNext();
		const b3_Fixture* GetNext() const;

		// ��ȡ��fixture������ָ�����û����ݡ�ʹ�ô˹��ܴ洢�ض���Ӧ�ó��������
		b3_FixtureUserData& GetUserData();
		const b3_FixtureUserData& GetUserData() const;

		// ����һ�����Ƿ���fixture�ڡ�
		// @param p a point in world coordinates.
		bool TestPoint(const glm::vec3& point) const;

		// ���߼�����shape
		// @param output the ray-cast results.
		// @param input the ray-cast input parameters.
		// @param childIndex the child shape index (e.g. edge index)
		bool RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input, int childIndex) const;

		// ��ȡ��Fixture���������ݡ��������ݻ����ܶȺ�shape��ת��������shape��ԭ���йء��˲������ܺܰ���
		void GetMassData(b3_MassData* massData) const;

		// ���ô�fixture���ܶȡ��ⲻ���Զ�����body�����������������b3_Body::ResetMassData������body��������
		void SetDensity(float density);

		// ��ȡ��fixture���ܶ�
		float GetDensity() const;

		// ��ȡĦ��ϵ��(coefficient of friction).
		float GetFriction() const;

		// ����Ħ��ϵ��(coefficient of friction)  �ⲻ��ı�����contacts��Ħ������
		void SetFriction(float friction);

		// �õ��ָ�ϵ��(coefficient of restitution).
		float GetRestitution() const;

		// ���ûָ�ϵ��(coefficient of restitution). �ⲻ��ı�����contacts�Ļָ���
		void SetRestitution(float restitution);

		// ��ȡ�ָ��ٶ���ֵ(restitution velocity threshold).
		float GetRestitutionThreshold() const;

		// ���ûָ��ٶ���ֵ(restitution velocity threshold) �ⲻ��ı�����contacts�Ļָ��ٶ���ֵ��
		void SetRestitutionThreshold(float threshold);

		// ��ȡfixture��AABB����AABB�����ѷŴ�(enlarge)��/���ʱ(stale)�� 
		// �����Ҫ����ȷ(accurate)��AABB����ʹ��shape��body��transform���м��㡣
		const b3_AABB& GetAABB(int childIndex) const;

		// Dump this fixture to the log file.
		//void Dump(int bodyIndex);
		
	protected:
		friend class b3_Body;
		friend class b3_World;
		friend class b3_Contact;
		friend class b3_ContactManager;

		b3_Fixture();

		// ������Ҫ������/���ٺ����빹�캯��/������������(separation)����Ϊ���������޷�����(access)������(allocator)��C++������ʹ��������������(arguments)����
		void Create(b3_BlockAllocator* allocator, b3_Body* body, const b3_FixtureDef* def);
		// ��allocator������fixture�����д����shape
		void Destroy(b3_BlockAllocator* allocator); 

		// ��Щ֧��body����/ͣ�á� These support body activation/deactivation.
		void CreateProxies(b3_BroadPhase* broadPhase, const b3_Transform& transform);
		// ��broadPhase������fixture�����д���
		void DestroyProxies(b3_BroadPhase* broadPhase);

		// ����fixtureλ�õ�transform1��transform2��AABB�ĺϲ�AABB�����µ�fixture�����д���
		void Synchronize(b3_BroadPhase* broadPhase, const b3_Transform& transform1, const b3_Transform& transform2);

		b3_FixtureUserData m_userData;
		float m_density;    // �ܶ�
		float m_friction;
		float m_restitution;
		float m_restitutionThreshold;
		b3_Body* m_body;
		b3_Fixture* m_next;
		b3_Filter m_filter;
		bool m_isSensor;         // �Ƿ񴫸�����Ĭ��false
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