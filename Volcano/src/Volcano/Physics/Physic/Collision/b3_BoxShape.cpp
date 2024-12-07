#include "volpch.h"

#include "b3_BoxShape.h"
#include "Volcano/Physics/Physic/b3_BlockAllocator.h"

namespace Volcano {

	b3_BoxShape::b3_BoxShape()
	{
		m_type = e_box;
		m_radius = b3_BoxRadius;
		m_position = { 0.0f, 0.0f, 0.0f };
		int indices[] = 
		{
			4, 5, 6, 6, 7, 4,
			0, 1, 2, 2, 3, 0,
			5, 0, 3, 3, 6, 5,
			1, 4, 7, 7, 2, 1,
			5, 4, 1, 1, 0, 5,
			3, 2, 7, 7, 6, 3
		};
		for (int i = 0; i < 36; i++)
			m_indices[i] = indices[i];

		glm::vec3 nromals[] =
		{
			{ 0.0f,  0.0f, -1.0f},
			{ 0.0f,  0.0f,  1.0f},
			{-1.0f,  0.0f,  0.0f},
			{ 1.0f,  0.0f,  0.0f},
			{ 0.0f, -1.0f,  0.0f},
			{ 0.0f,  1.0f,  0.0f}
		};
		for (int i = 0; i < 6; i++)
			m_normals[i] = nromals[i];

	}

	b3_Shape* b3_BoxShape::Clone(b3_BlockAllocator* allocator) const
	{
		void* mem = allocator->Allocate(sizeof(b3_BoxShape));
		b3_BoxShape* clone = new (mem) b3_BoxShape;
		*clone = *this;
		return clone;
	}

	int b3_BoxShape::GetChildCount() const
	{
		return 1;
	}

	bool b3_BoxShape::TestPoint(const b3_Transform& transform, const glm::vec3& point) const
	{
		glm::vec3 pLocal = b3_MultiplyT(transform.rotation, point - transform.position);

		if (pLocal.x >  m_size.x) return false;
		if (pLocal.x < -m_size.x) return false;
		if (pLocal.y >  m_size.y) return false;
		if (pLocal.y < -m_size.y) return false;
		if (pLocal.z >  m_size.z) return false;
		if (pLocal.z < -m_size.z) return false;

		return true;
	}

	// 光线与BoxShape相交测试, 起点在box内时返回0，法线normalize(p1-p2)
	// @param p1 局部坐标光线起点
	// @param p2 局部坐标光线终点
	float b3_BoxShape::RayBoxIntersect(const glm::vec3& p1, const glm::vec3& p2, glm::vec3* normal) const
	{
		glm::vec3 pp = p2 - p1; // 向量p1->p2
		//起点是否在box内部
		bool inside = true;
		float xt, xn;
		if (p1.x < -m_size.x)
		{
			xt = -m_size.x - p1.x;
			if (xt > pp.x) return -1.0f;
			xt /= pp.x;
			inside = false;
			xn = -1.0f;
		}
		else if (p1.x > m_size.x)
		{
			xt = m_size.x - p1.x;
			if (xt < pp.x) return -1.0f;
			xt /= pp.x;
			inside = false;
			xn = 1.0f;
		}
		else
			xt = -1.0f;

		float yt, yn;
		if (p1.y < -m_size.y)
		{
			yt = -m_size.y - p1.y;
			if (yt > pp.y) return -1.0f;
			yt /= pp.y;
			inside = false;
			yn = -1.0f;
		}
		else if (p1.y > m_size.y)
		{
			yt = m_size.y - p1.y;
			if (yt < pp.y) return -1.0f;
			yt /= pp.y;
			inside = false;
			yn = 1.0f;
		}
		else
			yt = -1.0f;

		float zt, zn;
		if (p1.z < -m_size.z)
		{
			zt = -m_size.z - p1.z;
			if (zt > pp.z) return -1.0f;
			zt /= pp.z;
			inside = false;
			zn = -1.0f;
		}
		else if (p1.z > m_size.z)
		{
			zt = m_size.z - p1.z;
			if (zt < pp.z) return -1.0f;
			zt /= pp.z;
			inside = false;
			zn = 1.0f;
		}
		else
			zt = -1.0f;

		if (inside)
		{
			if (normal != nullptr)
				*normal = glm::normalize(-pp);
			return 0.0f;
		}

		// 光线与哪个平面相交
		int which = 0;
		float t = xt;   // 参数化交点 [0,1]
		if (yt > t)
		{
			which = 1;
			t = yt;
		}
		if (zt > t)
		{
			which = 2;
			t = zt;
		}
		switch (which)
		{
		case 0: //与yz平面相交
		{
			float y = p1.y + pp.y * t;
			if (y < -m_size.y || y > m_size.y) return -1.0f;
			float z = p1.z + pp.z * t;
			if (z < -m_size.z || z > m_size.z) return -1.0f;

			if (normal != nullptr)
				*normal = { xn, 0.0f, 0.0f };
		}
		break;

		case 1: //与xz平面相交
		{
			float x = p1.x + pp.x * t;
			if (x < -m_size.x || x > m_size.x) return -1.0f;
			float z = p1.z + pp.z * t;
			if (z < -m_size.z || z > m_size.z) return -1.0f;

			if (normal != nullptr)
				*normal = { 0.0f, yn, 0.0f };
		}
		break;

		case 2: //与xy平面相交
		{
			float x = p1.x + pp.x * t;
			if (x < -m_size.x || x > m_size.x) return -1.0f;
			float y = p1.y + pp.y * t;
			if (y < -m_size.y || y > m_size.y) return -1.0f;

			if (normal != nullptr)
				*normal = { 0.0f, 0.0f, zn };
		}
		break;

		default:
			break;
		}

		return t;
	}

	/*
	std::vector<glm::vec3> b3_BoxShape::GetVerticesAll() const
	{
		std::vector<glm::vec3> vertices;
		vertices.resize(26);
		vertices[0]  = glm::vec3( m_size.x,  m_size.y,  m_size.z);
		vertices[1]  = glm::vec3( m_size.x,  m_size.y, -m_size.z);
		vertices[2]  = glm::vec3( m_size.x,  m_size.y,  0.0f);
		vertices[3]  = glm::vec3( m_size.x, -m_size.y,  m_size.z);
		vertices[4]  = glm::vec3( m_size.x, -m_size.y, -m_size.z);
		vertices[5]  = glm::vec3( m_size.x, -m_size.y,  0.0f);
		vertices[6]  = glm::vec3( m_size.x,  0.0f,      m_size.z);
		vertices[7]  = glm::vec3( m_size.x,  0.0f,     -m_size.z);
		vertices[8]  = glm::vec3( m_size.x,  0.0f,      0.0f);
		vertices[9]  = glm::vec3(-m_size.x,  m_size.y,  m_size.z);
		vertices[10] = glm::vec3(-m_size.x,  m_size.y, -m_size.z);
		vertices[11] = glm::vec3(-m_size.x,  m_size.y,  0.0f);
		vertices[12] = glm::vec3(-m_size.x, -m_size.y,  m_size.z);
		vertices[13] = glm::vec3(-m_size.x, -m_size.y, -m_size.z);
		vertices[14] = glm::vec3(-m_size.x, -m_size.y,  0.0f);
		vertices[15] = glm::vec3(-m_size.x,  0.0f,      m_size.z);
		vertices[16] = glm::vec3(-m_size.x,  0.0f,     -m_size.z);
		vertices[17] = glm::vec3(-m_size.x,  0.0f,      0.0f);
		vertices[18] = glm::vec3(0.0f,       m_size.y,  m_size.z);
		vertices[19] = glm::vec3(0.0f,       m_size.y, -m_size.z);
		vertices[20] = glm::vec3(0.0f,       m_size.y,  0.0f);
		vertices[21] = glm::vec3(0.0f,      -m_size.y,  m_size.z);
		vertices[22] = glm::vec3(0.0f,      -m_size.y, -m_size.z);
		vertices[23] = glm::vec3(0.0f,      -m_size.y,  0.0f);
		vertices[24] = glm::vec3(0.0f,       0.0f,      m_size.z);
		vertices[25] = glm::vec3(0.0f,       0.0f,     -m_size.z);
		return vertices;
	}
	*/

	glm::vec3 b3_BoxShape::GetNormal(int index) const
	{
		return m_normals[index / 6];
	}

	bool b3_BoxShape::RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input, const b3_Transform& transform, int childIndex) const
	{
		(void)(childIndex); // not used

		// 将光线放入箱的参照系(frame of reference)中
		glm::vec3 p1 = b3_MultiplyT(transform.rotation, input.p1 - transform.position);
		glm::vec3 p2 = b3_MultiplyT(transform.rotation, input.p2 - transform.position);

		float lower = 0.0f, upper = input.maxFraction;

		glm::vec3 pt = p1 + upper * (p2 - p1); // 线段终点的点pt，maxFraction不一定等于1，t点可能在线段p1p2前中后任何位置

		lower = RayBoxIntersect(p1, pt, &(output->normal));
		if (lower < 0.0f)
			return false;

		assert(0.0f <= lower && lower <= input.maxFraction);

		output->fraction = lower;
		output->normal = b3_Multiply(transform.rotation, output->normal);
		return true;
	}

	void b3_BoxShape::ComputeAABB(b3_AABB* aabb, const b3_Transform& transform, int childIndex) const
	{
		(void)(childIndex); // not used
		
		glm::vec3 lower = b3_Multiply(transform, m_vertices[0]);
		glm::vec3 upper = lower;
		for (int i = 1; i < getArrayLength(m_vertices); ++i)
		{
			glm::vec3 v = b3_Multiply(transform, m_vertices[i]);
			lower = glm::min(lower, v);
			upper = glm::max(upper, v);
		}

		glm::vec3 r(m_radius);
		aabb->lowerBound = lower - r;
		aabb->upperBound = upper + r;
	}

	void b3_BoxShape::ComputeMass(b3_MassData* massData, float density) const
	{
		float volume = 8.0f * m_size.x * m_size.y * m_size.z;
		massData->mass = density * volume;

		// Center of mass
		assert(volume > b3_Epsilon);
		massData->center = m_position;

		massData->I = (1.0f / 3.0f) * massData->mass * glm::mat3(
		          m_size.y * m_size.y + m_size.z * m_size.z, 0.0f, 0.0f,
			      0.0f,                                      m_size.x * m_size.x + m_size.z * m_size.z, 0.0f,
			      0.0f,                                      0.0f,                                      m_size.x * m_size.x + m_size.y * m_size.y);

		// Shift to center of mass then to original body origin.
		massData->I = ComputeInertia(massData->I, massData->mass, massData->center);
	}

	bool b3_BoxShape::Set(const glm::vec3* size)
	{
		if (size->x >= 0.0f && size->y >= 0.0f && size->z >= 0.0f)
		{
			m_size = *size;

			glm::vec3 vertices[] = {
				{ -m_size.x, -m_size.y,  m_size.z },
				{  m_size.x, -m_size.y,  m_size.z },
				{  m_size.x,  m_size.y,  m_size.z },
				{ -m_size.x,  m_size.y,  m_size.z },
				{  m_size.x, -m_size.y, -m_size.z },
				{ -m_size.x, -m_size.y, -m_size.z },
				{ -m_size.x,  m_size.y, -m_size.z },
				{  m_size.x,  m_size.y, -m_size.z }
			};
			for (int i = 0; i < 8; i++)
				m_vertices[i] = vertices[i];

			return true;
		}
		return false;
	}

	glm::vec3 b3_BoxShape::GetClosestPoint(const glm::vec3& point) const
	{
		// 球体原点不在box内部
		glm::vec3 closestPoint = point; // box到球的最近点
		if (closestPoint.x < -m_size.x)
			closestPoint.x = -m_size.x;
		else if (closestPoint.x > m_size.x)
			closestPoint.x = m_size.x;

		if (closestPoint.y < -m_size.y)
			closestPoint.y = -m_size.y;
		else if (closestPoint.y > m_size.y)
			closestPoint.y = m_size.y;

		if (closestPoint.z < -m_size.z)
			closestPoint.z = -m_size.z;
		else if (closestPoint.z > m_size.z)
			closestPoint.z = m_size.z;

		return closestPoint;
	}
}
