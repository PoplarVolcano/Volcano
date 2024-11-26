#pragma once

#include "b3_Math.h"

namespace Volcano {

	class b3_Shape;

	// 距离代理(Distance Proxy)用于GJK算法， 它可以封装(encapsulates)任何shape。
	struct b3_DistanceProxy
	{
		b3_DistanceProxy() : m_vertices(nullptr), m_count(0), m_radius(0.0f) {}

		// 使用给定的shape初始化代理。使用代理时，shape必须保持在范围内。
		void Set(const b3_Shape* shape, int index);

		// 使用顶点云和半径初始化代理。使用代理时，顶点必须保持在范围内。
		void Set(const glm::vec3* vertices, int count, float radius);

		// 获取给定方向上的支持(supporting)顶点索引。 
		int GetSupport(const glm::vec3& direction) const;

		// 获取给定方向上的支持(supporting)顶点。 
		const glm::vec3& GetSupportVertex(const glm::vec3& direction) const;

		// 获取顶点数量
		int GetVertexCount() const;

		// 按索引获取顶点。由b3_Distance使用。
		const glm::vec3& GetVertex(int index) const;

		glm::vec3 m_buffer[2];
		const glm::vec3* m_vertices;
		int m_count;
		float m_radius;
	};

	// 用于热启动b3_Distance。 在第一次调用时将计数(count)设置为零
	struct b3_SimplexCache
	{
		float metric;		        // length or area
		unsigned short count;
		unsigned char indexA[3];	// vertices on shape A
		unsigned char indexB[3];	// vertices on shape B
	};

	// Input for b3_Distance.
	// 您必须选择(option)在计算中使用形状半径(shape radii)
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

	// 计算两个形状之间的最近点。支持以下任意组合：b3_SphereShape、b3_PolygonShape、b3_EdgeShape。
	// 单工缓存(simplex cache)是输入/输出。在第一次调用时，将b3_SimplexCache.count设置为零。
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