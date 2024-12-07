#pragma once

#include "b3_Math.h"
#include "b3_Collision.h"

namespace Volcano {

	class b3_BlockAllocator;

	struct b3_MassData
	{
		
		float mass;       // shape的质量，通常以千克为单位。 The mass of the shape, usually in kilograms.
		glm::vec3 center; // shape质心相对于shape原点的位置。The position of the shape's centroid relative to the shape's origin.
		glm::mat3 I;      // shape惯性张量
	};

	// shape用于碰撞检测。在b3_World中用于模拟的shape是在创建b3_Fixture时自动创建的。shape可以封装(encapsulate)一个或多个子shape。
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

		Type GetType() const;// 获取shape的类型. You can use this to down cast to the concrete shape. @return the shape type.
		
		virtual int GetChildCount() const = 0;  // 获取子节点数量

		// 点point是否在shape内。这仅适用于凸形。 This only works for convex shapes.
		// @param transform the shape world transform.
		// @param p a point in world coordinates.
		virtual bool TestPoint(const b3_Transform& transform, const glm::vec3& point) const = 0;

		/*
		    射线检测(ray casting)一个子结点
		    @param output the ray-cast results.
		    @param input the ray-cast input parameters.
		    @param transform the transform to be applied to the shape.
		    @param childIndex the child shape index
		*/
		virtual bool RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input, const b3_Transform& transform, int childIndex) const = 0;

		/*
		    给定一个transform，计算子shape的相关(associated)轴对齐边界框(axis aligned bounding box)。
		    @param aabb returns the axis aligned bounding box.
		    @param xf the world transform of the shape.
		    @param childIndex the child shape
		*/
		virtual void ComputeAABB(b3_AABB* aabb, const b3_Transform& transform, int childIndex) const = 0;
		
		/*
		使用此shape的尺寸(dimensions)和密度(density)计算其质量特性。
        惯性张量(inertia tensor)是围绕局部原点计算的。
		@param massData returns the mass data for this shape.
		@param density the density in kg/m^2(kilograms per meter squared).
		*/
		virtual void ComputeMass(b3_MassData* massData, float density) const = 0;

		Type m_type;
		float m_radius;   // shape的半径。对于盒(box)，这必须是b3_BoxRadius。不支持制作圆形多边形。
	};

	inline b3_Shape::Type b3_Shape::GetType() const
	{
		return m_type;
	}

}