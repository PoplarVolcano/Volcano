#pragma once

#include "Volcano/Physics/Physic/b3_Shape.h"
#include "Volcano/Physics/Physic/b3_Math.h"

namespace Volcano {

	// 线段(边)形状(line segment (edge))。这些可以以链(chains)或环(loops)的形式连接到其他边(edge)形状。独立创建的边是双面(two-sided)的，不能在连接(junctions)处平滑移动。
	class b3_EdgeShape : public b3_Shape
	{
	public:
		b3_EdgeShape();

		glm::vec3 m_position;

		virtual b3_Shape* Clone(b3_BlockAllocator* allocator) const override;
		virtual int GetChildCount() const override;
		virtual bool TestPoint(const b3_Transform& transform, const glm::vec3& point) const override;
		virtual bool RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input, const b3_Transform& transform, int childIndex) const override;
		virtual void ComputeAABB(b3_AABB* aabb, const b3_Transform& transform, int childIndex) const override;
		virtual void ComputeMass(b3_MassData* massData, float density) const override;

		// 将其设置为序列(sequence)的一部分。顶点v0位于边之前(precedes)，顶点v3位于边之后(follows)。
		// 这些额外的顶点用于在交叉点之间提供平滑的移动。这也使得碰撞变成单向的。从v1到v2，边缘法线指向右侧。

		/// Set this as a part of a sequence. Vertex v0 precedes the edge and vertex v3
		/// follows. These extra vertices are used to provide smooth movement
		/// across junctions. This also makes the collision one-sided. The edge
		/// normal points to the right looking from v1 to v2.
		void SetOneSided(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);

		/// Set this as an isolated edge. Collision is two-sided.
		void SetTwoSided(const glm::vec3& v1, const glm::vec3& v2);

		/// Implement b3_Shape.
		b3_Shape* Clone(b3_BlockAllocator* allocator) const override;

		/// @see b3_Shape::GetChildCount
		int GetChildCount() const override;

		/// @see b3_Shape::TestPoint
		bool TestPoint(const b3_Transform& transform, const glm::vec3& p) const override;

		/// Implement b3_Shape.
		bool RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input,
			const b3_Transform& transform, int childIndex) const override;

		/// @see b3_Shape::ComputeAABB
		void ComputeAABB(b3_AABB* aabb, const b3_Transform& transform, int childIndex) const override;

		/// @see b3_Shape::ComputeMass
		void ComputeMass(b3_MassData* massData, float density) const override;

		/// These are the edge vertices
		glm::vec3 m_vertex1, m_vertex2;

		/// Optional adjacent vertices. These are used for smooth collision.
		glm::vec3 m_vertex0, m_vertex3;

		/// Uses m_vertex0 and m_vertex3 to create smooth collision.
		bool m_oneSided;
	};
}