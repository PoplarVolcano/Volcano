#include "volpch.h"

#include "b3_Distance.h"
#include "b3_Body.h"
#include "b3_Shape.h"
#include "Collision/b3_SphereShape.h"
#include "Collision/b3_BoxShape.h"

namespace Volcano {

	// GJK使用Voronoi区域(regions)（Christer-Ericson）和质心坐标(Barycentric coordinates)。
	int b3_GjkCalls, b3_GjkIters, b3_GjkMaxIters;

	void b3_DistanceProxy::Set(const b3_Shape* shape, int index)
	{
		switch (shape->GetType())
		{
		case b3_Shape::e_sphere:
		{
			const b3_SphereShape* sphere = static_cast<const b3_SphereShape*>(shape);
			m_vertices = &sphere->m_position;
			m_count = 1;
			m_radius = sphere->m_radius;
		}
		break;

		case b3_Shape::e_box:
		{
			const b3_BoxShape* box = static_cast<const b3_BoxShape*>(shape);
			m_vertices = &box->m_position;
			m_count = 8;
			m_radius = box->m_radius;
		}
		break;

		/*
		case b3_Shape::e_polygon:
		{
			const b3_PolygonShape* polygon = static_cast<const b3_PolygonShape*>(shape);
			m_vertices = polygon->m_vertices;
			m_count = polygon->m_count;
			m_radius = polygon->m_radius;
		}
		break;

		case b3_Shape::e_chain:
		{
			const b3_ChainShape* chain = static_cast<const b3_ChainShape*>(shape);
			assert(0 <= index && index < chain->m_count);

			m_buffer[0] = chain->m_vertices[index];
			if (index + 1 < chain->m_count)
			{
				m_buffer[1] = chain->m_vertices[index + 1];
			}
			else
			{
				m_buffer[1] = chain->m_vertices[0];
			}

			m_vertices = m_buffer;
			m_count = 2;
			m_radius = chain->m_radius;
		}
		break;

		case b3_Shape::e_edge:
		{
			const b3_EdgeShape* edge = static_cast<const b3_EdgeShape*>(shape);
			m_vertices = &edge->m_vertex1;
			m_count = 2;
			m_radius = edge->m_radius;
		}
		break;
		*/
		default:
			assert(false);
		}
	}

	void b3_DistanceProxy::Set(const glm::vec3* vertices, int count, float radius)
	{
		m_vertices = vertices;
		m_count = count;
		m_radius = radius;
	}

	struct b3_SimplexVertex
	{
		glm::vec3 wA;		// support point in proxyA
		glm::vec3 wB;		// support point in proxyB
		glm::vec3 w;		// wB - wA
		float a;		    // 最近点的重心(barycentric)坐标 
		int indexA;         // wA index
		int indexB;         // wB index
	};

	struct b3_Simplex
	{
		void ReadCache(const b3_SimplexCache* cache,
			const b3_DistanceProxy* proxyA, const b3_Transform& transformA,
			const b3_DistanceProxy* proxyB, const b3_Transform& transformB)
		{
			assert(cache->count <= 3);

			// Copy data from cache.
			m_count = cache->count;
			b3_SimplexVertex* vertices = &m_v1;
			for (int i = 0; i < m_count; ++i)
			{
				b3_SimplexVertex* v = vertices + i;
				v->indexA = cache->indexA[i];
				v->indexB = cache->indexB[i];
				glm::vec3 wALocal = proxyA->GetVertex(v->indexA);
				glm::vec3 wBLocal = proxyB->GetVertex(v->indexB);
				v->wA = b3_Multiply(transformA, wALocal);
				v->wB = b3_Multiply(transformB, wBLocal);
				v->w = v->wB - v->wA;
				v->a = 0.0f;
			}

			// 计算新的单纯形度量(metric)，如果它与旧度量有很大不同，则刷新单纯形。
			if (m_count > 1)
			{
				float metric1 = cache->metric;
				float metric2 = GetMetric();
				if (metric2 < 0.5f * metric1 || 2.0f * metric1 < metric2 || metric2 < b3_Epsilon)
				{
					// Reset the simplex.
					m_count = 0;
				}
			}

			// 如果缓存为空或无效。。。 
			if (m_count == 0)
			{
				b3_SimplexVertex* v = vertices + 0;
				v->indexA = 0;
				v->indexB = 0;
				glm::vec3 wALocal = proxyA->GetVertex(0);
				glm::vec3 wBLocal = proxyB->GetVertex(0);
				v->wA = b3_Multiply(transformA, wALocal);
				v->wB = b3_Multiply(transformB, wBLocal);
				v->w = v->wB - v->wA;
				v->a = 1.0f;
				m_count = 1;
			}
		}

		void WriteCache(b3_SimplexCache* cache) const
		{
			cache->metric = GetMetric();
			cache->count = unsigned short(m_count);
			const b3_SimplexVertex* vertices = &m_v1;
			for (int i = 0; i < m_count; ++i)
			{
				cache->indexA[i] = unsigned char(vertices[i].indexA);
				cache->indexB[i] = unsigned char(vertices[i].indexB);
			}
		}

		glm::vec3 GetSearchDirection() const
		{
			return glm::vec3();
			/*
			switch (m_count)
			{
			case 1:
				return -m_v1.w;

			case 2:
			{
				glm::vec3 e12 = m_v2.w - m_v1.w;
				float sgn = glm::cross(e12, -m_v1.w);
				if (sgn > 0.0f)
				{
					// Origin is left of e12.
					return glm::cross(1.0f, e12);
				}
				else
				{
					// Origin is right of e12.
					return glm::cross(e12, 1.0f);
				}
			}

			default:
				assert(false);
				return glm::vec3();
			}
			*/
		}

		glm::vec3 GetClosestPoint() const
		{
			switch (m_count)
			{
			case 0:
				assert(false);
				return glm::vec3();

			case 1:
				return m_v1.w;

			case 2:
				return m_v1.a * m_v1.w + m_v2.a * m_v2.w;

			case 3:
				return glm::vec3();

			default:
				assert(false);
				return glm::vec3();
			}
		}

		void GetWitnessPoints(glm::vec3* pA, glm::vec3* pB) const
		{
			switch (m_count)
			{
			case 0:
				assert(false);
				break;

			case 1:
				*pA = m_v1.wA;
				*pB = m_v1.wB;
				break;

			case 2:
				*pA = m_v1.a * m_v1.wA + m_v2.a * m_v2.wA;
				*pB = m_v1.a * m_v1.wB + m_v2.a * m_v2.wB;
				break;

			case 3:
				*pA = m_v1.a * m_v1.wA + m_v2.a * m_v2.wA + m_v3.a * m_v3.wA;
				*pB = *pA;
				break;

			default:
				assert(false);
				break;
			}
		}

		float GetMetric() const
		{
			switch (m_count)
			{
			case 0:
				assert(false);
				return 0.0f;

			case 1:
				return 0.0f;

			case 2:
				return glm::distance(m_v1.w, m_v2.w);

			case 3:
				return glm::length(glm::cross(m_v2.w - m_v1.w, m_v3.w - m_v1.w));

			default:
				assert(false);
				return 0.0f;
			}
		}

		void Solve2();
		void Solve3();

		b3_SimplexVertex m_v1, m_v2, m_v3;
		int m_count;
	};


	// Solve a line segment using barycentric coordinates.
	//
	// p = a1 * w1 + a2 * w2
	// a1 + a2 = 1
	//
	// The vector from the origin to the closest point on the line is
	// perpendicular to the line.
	// e12 = w2 - w1
	// dot(p, e) = 0
	// a1 * dot(w1, e) + a2 * dot(w2, e) = 0
	//
	// 2-by-2 linear system
	// [1      1     ][a1] = [1]
	// [w1.e12 w2.e12][a2] = [0]
	//
	// Define
	// d12_1 =  dot(w2, e12)
	// d12_2 = -dot(w1, e12)
	// d12 = d12_1 + d12_2
	//
	// Solution
	// a1 = d12_1 / d12
	// a2 = d12_2 / d12
	void b3_Simplex::Solve2()
	{
		glm::vec3 w1 = m_v1.w;
		glm::vec3 w2 = m_v2.w;
		glm::vec3 e12 = w2 - w1;

		// w1 region
		float d12_2 = -glm::dot(w1, e12);
		if (d12_2 <= 0.0f)
		{
			// a2 <= 0, so we clamp it to 0
			m_v1.a = 1.0f;
			m_count = 1;
			return;
		}

		// w2 region
		float d12_1 = glm::dot(w2, e12);
		if (d12_1 <= 0.0f)
		{
			// a1 <= 0, so we clamp it to 0
			m_v2.a = 1.0f;
			m_count = 1;
			m_v1 = m_v2;
			return;
		}

		// Must be in e12 region.
		float inv_d12 = 1.0f / (d12_1 + d12_2);
		m_v1.a = d12_1 * inv_d12;
		m_v2.a = d12_2 * inv_d12;
		m_count = 2;
	}

	// Possible regions:
	// - points[2]
	// - edge points[0]-points[2]
	// - edge points[1]-points[2]
	// - inside the triangle
	void b3_Simplex::Solve3()
	{
		glm::vec3 w1 = m_v1.w;
		glm::vec3 w2 = m_v2.w;
		glm::vec3 w3 = m_v3.w;

		// Edge12
		// [1      1     ][a1] = [1]
		// [w1.e12 w2.e12][a2] = [0]
		// a3 = 0
		glm::vec3 e12 = w2 - w1;
		float w1e12 = glm::dot(w1, e12);
		float w2e12 = glm::dot(w2, e12);
		float d12_1 = w2e12;
		float d12_2 = -w1e12;

		// Edge13
		// [1      1     ][a1] = [1]
		// [w1.e13 w3.e13][a3] = [0]
		// a2 = 0
		glm::vec3 e13 = w3 - w1;
		float w1e13 = glm::dot(w1, e13);
		float w3e13 = glm::dot(w3, e13);
		float d13_1 = w3e13;
		float d13_2 = -w1e13;

		// Edge23
		// [1      1     ][a2] = [1]
		// [w2.e23 w3.e23][a3] = [0]
		// a1 = 0
		glm::vec3 e23 = w3 - w2;
		float w2e23 = glm::dot(w2, e23);
		float w3e23 = glm::dot(w3, e23);
		float d23_1 = w3e23;
		float d23_2 = -w2e23;

		// Triangle123
		float n123 = glm::length(glm::cross(e12, e13));

		float d123_1 = n123 * glm::length(glm::cross(w2, w3));
		float d123_2 = n123 * glm::length(glm::cross(w3, w1));
		float d123_3 = n123 * glm::length(glm::cross(w1, w2));

		// w1 region
		if (d12_2 <= 0.0f && d13_2 <= 0.0f)
		{
			m_v1.a = 1.0f;
			m_count = 1;
			return;
		}

		// e12
		if (d12_1 > 0.0f && d12_2 > 0.0f && d123_3 <= 0.0f)
		{
			float inv_d12 = 1.0f / (d12_1 + d12_2);
			m_v1.a = d12_1 * inv_d12;
			m_v2.a = d12_2 * inv_d12;
			m_count = 2;
			return;
		}

		// e13
		if (d13_1 > 0.0f && d13_2 > 0.0f && d123_2 <= 0.0f)
		{
			float inv_d13 = 1.0f / (d13_1 + d13_2);
			m_v1.a = d13_1 * inv_d13;
			m_v3.a = d13_2 * inv_d13;
			m_count = 2;
			m_v2 = m_v3;
			return;
		}

		// w2 region
		if (d12_1 <= 0.0f && d23_2 <= 0.0f)
		{
			m_v2.a = 1.0f;
			m_count = 1;
			m_v1 = m_v2;
			return;
		}

		// w3 region
		if (d13_1 <= 0.0f && d23_1 <= 0.0f)
		{
			m_v3.a = 1.0f;
			m_count = 1;
			m_v1 = m_v3;
			return;
		}

		// e23
		if (d23_1 > 0.0f && d23_2 > 0.0f && d123_1 <= 0.0f)
		{
			float inv_d23 = 1.0f / (d23_1 + d23_2);
			m_v2.a = d23_1 * inv_d23;
			m_v3.a = d23_2 * inv_d23;
			m_count = 2;
			m_v1 = m_v3;
			return;
		}

		// Must be in triangle123
		float inv_d123 = 1.0f / (d123_1 + d123_2 + d123_3);
		m_v1.a = d123_1 * inv_d123;
		m_v2.a = d123_2 * inv_d123;
		m_v3.a = d123_3 * inv_d123;
		m_count = 3;
	}

	void b3_Distance(b3_DistanceOutput* output, b3_SimplexCache* cache, const b3_DistanceInput* input)
	{
		++b3_GjkCalls;

		const b3_DistanceProxy* proxyA = &input->proxyA;
		const b3_DistanceProxy* proxyB = &input->proxyB;

		b3_Transform transformA = input->transformA;
		b3_Transform transformB = input->transformB;

		// 初始化单纯形。 Initialize the simplex.
		b3_Simplex simplex;
		simplex.ReadCache(cache, proxyA, transformA, proxyB, transformB);

		// Get simplex vertices as an array.
		b3_SimplexVertex* vertices = &simplex.m_v1;
		const int k_maxIters = 20;

		// These store the vertices of the last simplex so that we
		// can check for duplicates and prevent cycling.
		int saveA[3], saveB[3];
		int saveCount = 0;

		// Main iteration loop.
		int iter = 0;
		while (iter < k_maxIters)
		{
			// Copy simplex so we can identify duplicates.
			saveCount = simplex.m_count;
			for (int i = 0; i < saveCount; ++i)
			{
				saveA[i] = vertices[i].indexA;
				saveB[i] = vertices[i].indexB;
			}

			switch (simplex.m_count)
			{
			case 1:
				break;

			case 2:
				simplex.Solve2();
				break;

			case 3:
				simplex.Solve3();
				break;

			default:
				assert(false);
			}

			// If we have 3 points, then the origin is in the corresponding triangle.
			if (simplex.m_count == 3)
			{
				break;
			}

			// Get search direction.
			glm::vec3 d = simplex.GetSearchDirection();

			// Ensure the search direction is numerically fit.
			if (glm::dot(d, d) < b3_Epsilon * b3_Epsilon)
			{
				// The origin is probably contained by a line segment
				// or triangle. Thus the shapes are overlapped.

				// We can't return zero here even though there may be overlap.
				// In case the simplex is a point, segment, or triangle it is difficult
				// to determine if the origin is contained in the CSO or very close to it.
				break;
			}

			// Compute a tentative new simplex vertex using support points.
			b3_SimplexVertex* vertex = vertices + simplex.m_count;
			vertex->indexA = proxyA->GetSupport(b3_MultiplyT(transformA.rotation, -d));
			vertex->wA = b3_Multiply(transformA, proxyA->GetVertex(vertex->indexA));
			vertex->indexB = proxyB->GetSupport(b3_MultiplyT(transformB.rotation, d));
			vertex->wB = b3_Multiply(transformB, proxyB->GetVertex(vertex->indexB));
			vertex->w = vertex->wB - vertex->wA;

			// Iteration count is equated to the number of support point calls.
			++iter;
			++b3_GjkIters;

			// Check for duplicate support points. This is the main termination criteria.
			bool duplicate = false;
			for (int i = 0; i < saveCount; ++i)
			{
				if (vertex->indexA == saveA[i] && vertex->indexB == saveB[i])
				{
					duplicate = true;
					break;
				}
			}

			// If we found a duplicate support point we must exit to avoid cycling.
			if (duplicate)
			{
				break;
			}

			// New vertex is ok and needed.
			++simplex.m_count;
		}

		b3_GjkMaxIters = glm::max(b3_GjkMaxIters, iter);

		// Prepare output.
		simplex.GetWitnessPoints(&output->pointA, &output->pointB);
		output->distance = glm::distance(output->pointA, output->pointB);
		output->iterations = iter;

		// Cache the simplex.
		simplex.WriteCache(cache);

		// Apply radii if requested
		if (input->useRadii)
		{
			if (output->distance < b3_Epsilon)
			{
				// Shapes are too close to safely compute normal
				glm::vec3 p = 0.5f * (output->pointA + output->pointB);
				output->pointA = p;
				output->pointB = p;
				output->distance = 0.0f;
			}
			else
			{
				// Keep closest points on perimeter even if overlapped, this way
				// the points move smoothly.
				float rA = proxyA->m_radius;
				float rB = proxyB->m_radius;
				glm::vec3 normal = output->pointB - output->pointA;
				glm::normalize(normal);
				output->distance = glm::max(0.0f, output->distance - rA - rB);
				output->pointA += rA * normal;
				output->pointB -= rB * normal;
			}
		}
	}

	// GJK-raycast
	// Algorithm by Gino van den Bergen.
	// "Smooth Mesh Contacts with GJK" in Game Physics Pearls. 2010
	bool b3_ShapeCast(b3_ShapeCastOutput* output, const b3_ShapeCastInput* input)
	{
		output->iterations = 0;
		output->lambda = 1.0f;
		output->normal = { 0.0f, 0.0f, 0.0f };
		output->point = { 0.0f, 0.0f, 0.0f };

		const b3_DistanceProxy* proxyA = &input->proxyA;
		const b3_DistanceProxy* proxyB = &input->proxyB;

		float radiusA = glm::max(proxyA->m_radius, b3_BoxRadius);
		float radiusB = glm::max(proxyB->m_radius, b3_BoxRadius);
		float radius = radiusA + radiusB;

		b3_Transform transformA = input->transformA;
		b3_Transform transformB = input->transformB;

		glm::vec3 r = input->translationB;
		glm::vec3 n(0.0f, 0.0f, 0.0f);
		float lambda = 0.0f;

		// Initial simplex
		b3_Simplex simplex;
		simplex.m_count = 0;

		// Get simplex vertices as an array.
		b3_SimplexVertex* vertices = &simplex.m_v1;

		// Get support point in -r direction
		int indexA = proxyA->GetSupport(b3_MultiplyT(transformA.rotation, -r));
		glm::vec3 wA = b3_Multiply(transformA, proxyA->GetVertex(indexA));
		int indexB = proxyB->GetSupport(b3_MultiplyT(transformB.rotation, r));
		glm::vec3 wB = b3_Multiply(transformB, proxyB->GetVertex(indexB));
		glm::vec3 v = wA - wB;

		// Sigma is the target distance between polygons
		float sigma = glm::max(b3_BoxRadius, radius - b3_BoxRadius);
		const float tolerance = 0.5f * b3_LinearSlop;

		// Main iteration loop.
		const int k_maxIters = 20;
		int iter = 0;
		while (iter < k_maxIters && glm::length(v) - sigma > tolerance)
		{
			assert(simplex.m_count < 3);

			output->iterations += 1;

			// Support in direction -v (A - B)
			indexA = proxyA->GetSupport(b3_MultiplyT(transformA.rotation, -v));
			wA = b3_Multiply(transformA, proxyA->GetVertex(indexA));
			indexB = proxyB->GetSupport(b3_MultiplyT(transformB.rotation, v));
			wB = b3_Multiply(transformB, proxyB->GetVertex(indexB));
			glm::vec3 p = wA - wB;

			// -v is a normal at p
			n = glm::normalize(n);

			// Intersect ray with plane
			float vp = glm::dot(v, p);
			float vr = glm::dot(v, r);
			if (vp - sigma > lambda * vr)
			{
				if (vr <= 0.0f)
				{
					return false;
				}

				lambda = (vp - sigma) / vr;
				if (lambda > 1.0f)
				{
					return false;
				}

				n = -v;
				simplex.m_count = 0;
			}

			// Reverse simplex since it works with B - A.
			// Shift by lambda * r because we want the closest point to the current clip point.
			// Note that the support point p is not shifted because we want the plane equation
			// to be formed in unshifted space.
			b3_SimplexVertex* vertex = vertices + simplex.m_count;
			vertex->indexA = indexB;
			vertex->wA = wB + lambda * r;
			vertex->indexB = indexA;
			vertex->wB = wA;
			vertex->w = vertex->wB - vertex->wA;
			vertex->a = 1.0f;
			simplex.m_count += 1;

			switch (simplex.m_count)
			{
			case 1:
				break;

			case 2:
				simplex.Solve2();
				break;

			case 3:
				simplex.Solve3();
				break;

			default:
				assert(false);
			}

			// If we have 3 points, then the origin is in the corresponding triangle.
			if (simplex.m_count == 3)
			{
				// Overlap
				return false;
			}

			// Get search direction.
			v = simplex.GetClosestPoint();

			// Iteration count is equated to the number of support point calls.
			++iter;
		}

		if (iter == 0)
		{
			// Initial overlap
			return false;
		}

		// Prepare output.
		glm::vec3 pointA, pointB;
		simplex.GetWitnessPoints(&pointB, &pointA);

		if (glm::dot(v, v) > 0.0f)
		{
			n = -v;
			n = glm::normalize(n);
		}

		output->point = pointA + radiusA * n;
		output->normal = n;
		output->lambda = lambda;
		output->iterations = iter;
		return true;
	}


}