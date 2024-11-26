#pragma once

#include "Volcano/Physics/Physic/b3_Shape.h"
#include "Volcano/Physics/Physic/b3_Math.h"

namespace Volcano {

	struct b3_Hull;

	// ʵ��͹�䡣��������ڲ�λ��ÿ�������ࡣ
	// �����󶥵�������b3_MaxBoxVertices��
	// �ڴ��������£�͹�䲻��Ҫ�ܶඥ�㡣
	class b3_BoxShape : public b3_Shape
	{
	public:
		b3_BoxShape();

		virtual b3_Shape* Clone(b3_BlockAllocator* allocator) const override;
		virtual int GetChildCount() const override;

		// ��point�Ƿ���shape�ڡ����������͹�Ρ� This only works for convex shapes.
		// @param transform the shape world transform.
		// @param point a point in world coordinates.
		virtual bool TestPoint(const b3_Transform& transform, const glm::vec3& point) const override;

		/*
			���߼��(ray casting)һ���ӽ��
			@param output the ray-cast results.
			@param input the ray-cast input parameters.
			@param transform the transform to be applied to the shape.
			@param childIndex the child shape index
		    @note ����Box��ʵ�ĵģ���˴��ڲ���ʼ�Ĺ��߲�����У���Ϊ�޷����巨�ߡ�
		*/
		virtual bool RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input, const b3_Transform& transform, int childIndex) const override;

		/*
			����һ��transform��������shape�����(associated)�����߽��(axis aligned bounding box)��
			@param aabb returns the axis aligned bounding box.
			@param xf the world transform of the shape.
			@param childIndex the child shape
		*/
		virtual void ComputeAABB(b3_AABB* aabb, const b3_Transform& transform, int childIndex) const override;

		/*
		ʹ�ô�shape�ĳߴ�(dimensions)���ܶ�(density)�������������ԡ�
		��������(inertia tensor)��Χ�ƾֲ�ԭ�����ġ�
		@param massData returns the mass data for this shape.
		@param density the density in kg/m^2(kilograms per meter squared).
		*/
		virtual void ComputeMass(b3_MassData* massData, float density) const override;

		bool Set(const glm::vec3* size);
		glm::vec3 GetClosestPoint(const glm::vec3& point) const;

		float RayBoxIntersect(const glm::vec3& p1, const glm::vec3& p2, glm::vec3* normal) const;

		// ��ȡ��i�������ķ�����
		glm::vec3 GetNormal(int i) const;
		glm::vec3 m_position;
		glm::vec3 m_size;
		glm::vec3 m_vertices[8];
		glm::vec3 m_normals[6];
		int m_indices[36];
	};
}