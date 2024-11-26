#pragma once

#include "b3_Math.h"

namespace Volcano {

	class b3_Shape;

	// �������(Distance Proxy)����GJK�㷨�� �����Է�װ(encapsulates)�κ�shape��
	struct b3_DistanceProxy
	{
		b3_DistanceProxy() : m_vertices(nullptr), m_count(0), m_radius(0.0f) {}

		// ʹ�ø�����shape��ʼ������ʹ�ô���ʱ��shape���뱣���ڷ�Χ�ڡ�
		void Set(const b3_Shape* shape, int index);

		// ʹ�ö����ƺͰ뾶��ʼ������ʹ�ô���ʱ��������뱣���ڷ�Χ�ڡ�
		void Set(const glm::vec3* vertices, int count, float radius);

		// ��ȡ���������ϵ�֧��(supporting)���������� 
		int GetSupport(const glm::vec3& direction) const;

		// ��ȡ���������ϵ�֧��(supporting)���㡣 
		const glm::vec3& GetSupportVertex(const glm::vec3& direction) const;

		// ��ȡ��������
		int GetVertexCount() const;

		// ��������ȡ���㡣��b3_Distanceʹ�á�
		const glm::vec3& GetVertex(int index) const;

		glm::vec3 m_buffer[2];
		const glm::vec3* m_vertices;
		int m_count;
		float m_radius;
	};

	// ����������b3_Distance�� �ڵ�һ�ε���ʱ������(count)����Ϊ��
	struct b3_SimplexCache
	{
		float metric;		        // length or area
		unsigned short count;
		unsigned char indexA[3];	// vertices on shape A
		unsigned char indexB[3];	// vertices on shape B
	};

	// Input for b3_Distance.
	// ������ѡ��(option)�ڼ�����ʹ����״�뾶(shape radii)
	struct b3_DistanceInput
	{
		b3_DistanceProxy proxyA;
		b3_DistanceProxy proxyB;
		b3_Transform transformA;
		b3_Transform transformB;
		bool useRadii;
	};

	// Output for b3_Distance.
	struct b3_DistanceOutput
	{
		glm::vec3 pointA;		// closest point on shapeA
		glm::vec3 pointB;		// closest point on shapeB
		float distance;
		int iterations;	        // number of GJK iterations used
	};

	// ����������״֮�������㡣֧������������ϣ�b3_SphereShape��b3_PolygonShape��b3_EdgeShape��
	// ��������(simplex cache)������/������ڵ�һ�ε���ʱ����b3_SimplexCache.count����Ϊ�㡣
	void b3_Distance(b3_DistanceOutput* output, b3_SimplexCache* cache, const b3_DistanceInput* input);

	// Input parameters for b3_ShapeCast
	struct b3_ShapeCastInput
	{
		b3_DistanceProxy proxyA;
		b3_DistanceProxy proxyB;
		b3_Transform transformA;
		b3_Transform transformB;
		glm::vec3 translationB;
	};

	// Output results for b3_ShapeCast
	struct b3_ShapeCastOutput
	{
		glm::vec3 point;
		glm::vec3 normal;
		float lambda;
		int iterations;
	};

	// Perform a linear shape cast of shape B moving and shape A fixed. Determines the hit point, normal, and translation fraction.
	// @returns true if hit, false if there is no hit or an initial overlap
	bool b3_ShapeCast(b3_ShapeCastOutput* output, const b3_ShapeCastInput* input);

	//////////////////////////////////////////////////

	inline int b3_DistanceProxy::GetVertexCount() const
	{
		return m_count;
	}

	inline const glm::vec3& b3_DistanceProxy::GetVertex(int index) const
	{
		assert(0 <= index && index < m_count);
		return m_vertices[index];
	}

	inline int b3_DistanceProxy::GetSupport(const glm::vec3& d) const
	{
		int bestIndex = 0;
		float bestValue = glm::dot(m_vertices[0], d);
		for (int i = 1; i < m_count; ++i)
		{
			float value = glm::dot(m_vertices[i], d);
			if (value > bestValue)
			{
				bestIndex = i;
				bestValue = value;
			}
		}

		return bestIndex;
	}

	inline const glm::vec3& b3_DistanceProxy::GetSupportVertex(const glm::vec3& d) const
	{
		int bestIndex = 0;
		float bestValue = glm::dot(m_vertices[0], d);
		for (int i = 1; i < m_count; ++i)
		{
			float value = glm::dot(m_vertices[i], d);
			if (value > bestValue)
			{
				bestIndex = i;
				bestValue = value;
			}
		}

		return m_vertices[bestIndex];
	}

}