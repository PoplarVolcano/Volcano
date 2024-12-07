#pragma once

#include "b3_Math.h"
#include "b3_Collision.h"

namespace Volcano {

	class b3_BlockAllocator;

	struct b3_MassData
	{
		
		float mass;       // shape��������ͨ����ǧ��Ϊ��λ�� The mass of the shape, usually in kilograms.
		glm::vec3 center; // shape���������shapeԭ���λ�á�The position of the shape's centroid relative to the shape's origin.
		glm::mat3 I;      // shape��������
	};

	// shape������ײ��⡣��b3_World������ģ���shape���ڴ���b3_Fixtureʱ�Զ������ġ�shape���Է�װ(encapsulate)һ��������shape��
	class b3_Shape
	{
	public:

		enum Type
		{
			e_sphere    = 0,
			e_box       = 1,
			e_edge      = 2,
			e_polygon   = 3,
			e_chain     = 4,
			e_typeCount = 5
		};

		virtual ~b3_Shape() {}

		virtual b3_Shape* Clone(b3_BlockAllocator* allocator) const = 0;

		Type GetType() const;// ��ȡshape������. You can use this to down cast to the concrete shape. @return the shape type.
		
		virtual int GetChildCount() const = 0;  // ��ȡ�ӽڵ�����

		// ��point�Ƿ���shape�ڡ����������͹�Ρ� This only works for convex shapes.
		// @param transform the shape world transform.
		// @param p a point in world coordinates.
		virtual bool TestPoint(const b3_Transform& transform, const glm::vec3& point) const = 0;

		/*
		    ���߼��(ray casting)һ���ӽ��
		    @param output the ray-cast results.
		    @param input the ray-cast input parameters.
		    @param transform the transform to be applied to the shape.
		    @param childIndex the child shape index
		*/
		virtual bool RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input, const b3_Transform& transform, int childIndex) const = 0;

		/*
		    ����һ��transform��������shape�����(associated)�����߽��(axis aligned bounding box)��
		    @param aabb returns the axis aligned bounding box.
		    @param xf the world transform of the shape.
		    @param childIndex the child shape
		*/
		virtual void ComputeAABB(b3_AABB* aabb, const b3_Transform& transform, int childIndex) const = 0;
		
		/*
		ʹ�ô�shape�ĳߴ�(dimensions)���ܶ�(density)�������������ԡ�
        ��������(inertia tensor)��Χ�ƾֲ�ԭ�����ġ�
		@param massData returns the mass data for this shape.
		@param density the density in kg/m^2(kilograms per meter squared).
		*/
		virtual void ComputeMass(b3_MassData* massData, float density) const = 0;

		Type m_type;
		float m_radius;   // shape�İ뾶�����ں�(box)���������b3_BoxRadius����֧������Բ�ζ���Ρ�
	};

	inline b3_Shape::Type b3_Shape::GetType() const
	{
		return m_type;
	}

}