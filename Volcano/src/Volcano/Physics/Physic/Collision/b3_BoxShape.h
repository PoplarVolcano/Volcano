#pragma once

#include "Volcano/Physics/Physic/b3_Shape.h"
#include "Volcano/Physics/Physic/b3_Math.h"

namespace Volcano {

	struct b3_Hull;

	// 实心凸箱。假设箱的内部位于每个面的左侧。
	// 箱的最大顶点数等于b3_MaxBoxVertices。
	// 在大多数情况下，凸箱不需要很多顶点。
	class b3_BoxShape : public b3_Shape
	{
	public:
		b3_BoxShape();

		virtual b3_Shape* Clone(b3_BlockAllocator* allocator) const override;
		virtual int GetChildCount() const override;

		// 点point是否在shape内。这仅适用于凸形。 This only works for convex shapes.
		// @param transform the shape world transform.
		// @param point a point in world coordinates.
		virtual bool TestPoint(const b3_Transform& transform, const glm::vec3& point) const override;

		/*
			射线检测(ray casting)一个子结点
			@param output the ray-cast results.
			@param input the ray-cast input parameters.
			@param transform the transform to be applied to the shape.
			@param childIndex the child shape index
		    @note 由于Box是实心的，因此从内部开始的光线不会击中，因为无法定义法线。
		*/
		virtual bool RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input, const b3_Transform& transform, int childIndex) const override;

		/*
			给定一个transform，计算子shape的相关(associated)轴对齐边界框(axis aligned bounding box)。
			@param aabb returns the axis aligned bounding box.
			@param xf the world transform of the shape.
			@param childIndex the child shape
		*/
		virtual void ComputeAABB(b3_AABB* aabb, const b3_Transform& transform, int childIndex) const override;

		/*
		使用此shape的尺寸(dimensions)和密度(density)计算其质量特性。
		惯性张量(inertia tensor)是围绕局部原点计算的。
		@param massData returns the mass data for this shape.
		@param density the density in kg/m^2(kilograms per meter squared).
		*/
		virtual void ComputeMass(b3_MassData* massData, float density) const override;

		bool Set(const glm::vec3* size);
		glm::vec3 GetClosestPoint(const glm::vec3& point) const;

		float RayBoxIntersect(const glm::vec3& p1, const glm::vec3& p2, glm::vec3* normal) const;

		// 获取第i个索引的法向量
		glm::vec3 GetNormal(int i) const;
		glm::vec3 m_position;
		glm::vec3 m_size;
		glm::vec3 m_vertices[8];
		glm::vec3 m_normals[6];
		int m_indices[36];
	};
}