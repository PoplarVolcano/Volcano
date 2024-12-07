#include "volpch.h"

#include "b3_SphereShape.h"
#include "Volcano/Physics/Physic/b3_BlockAllocator.h"

namespace Volcano {

	b3_SphereShape::b3_SphereShape()
	{
		m_type = e_sphere;
		m_radius = 0.0f;
		m_position = { 0.0f, 0.0f, 0.0f };
	}

	b3_Shape* b3_SphereShape::Clone(b3_BlockAllocator* allocator) const
	{
		void* mem = allocator->Allocate(sizeof(b3_SphereShape));
		b3_SphereShape* clone = new (mem) b3_SphereShape;
		*clone = *this;
		return clone;
	}

	int b3_SphereShape::GetChildCount() const
	{
		return 1;
	}

	bool b3_SphereShape::TestPoint(const b3_Transform& transform, const glm::vec3& point) const
	{
		glm::vec3 center = transform.position + b3_Multiply(transform.rotation, m_position);
		glm::vec3 distance = point - center;
		return glm::dot(distance, distance) <= m_radius * m_radius;
	}

	bool b3_SphereShape::RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input, const b3_Transform& transform, int childIndex) const
	{
		(void)(childIndex);  // not used
		
		glm::vec3 center = transform.position + b3_Multiply(transform.rotation, m_position); // 球心
		glm::vec3 pc = center - input.p1;     // 向量p1->c
		float pcDotpc = glm::dot(pc, pc);	  
		glm::vec3 pp = (input.p2 - input.p1); // 向量p1->p2
		float absPP = glm::length(pp);		  
		float rr = m_radius * m_radius;		  
		float a = glm::dot(pp, pc) / absPP;   // pc在pp方向上的映射
		float square_b = pcDotpc - a * a;     // b为球心到线段的距离
		float square_f = rr - square_b;       // f^2 = r^2 - b^2

		if (square_f < 0)
			return false;

		float fraction;
		if (pcDotpc < rr)
			fraction = (a + sqrtf(square_f)) / absPP;
		else
			fraction = (a - sqrtf(square_f)) / absPP;

		if (fraction > input.maxFraction)
			return false;

		output->fraction = fraction;          // 线段与球交点为(p1 + fraction * (p2 - p1))
		output->normal = glm::normalize(pp * output->fraction - pc);
		return true;
	}

	void b3_SphereShape::ComputeAABB(b3_AABB* aabb, const b3_Transform& transform, int childIndex) const
	{
		(void)(childIndex);  // not used

		glm::vec3 position = transform.position + b3_Multiply(transform.rotation, m_position);
		aabb->lowerBound = position - m_radius;
		aabb->upperBound = position + m_radius;
	}

	void b3_SphereShape::ComputeMass(b3_MassData* massData, float density) const
	{
		massData->mass = density * (4.0f / 3.0f) * b3_PI * m_radius * m_radius * m_radius;
		massData->center = m_position;

		massData->I = 0.4f * massData->mass * m_radius * m_radius * glm::mat3(1.0f);  // 平行轴定理：j' = j + md^2 球体j = 2/5 mr^2

		massData->I = ComputeInertia(massData->I, massData->mass, massData->center);
	}
}