#pragma once

#include "b3_Math.h"

namespace Volcano {

	class b3_Shape;
	class b3_SphereShape;
	class b3_BoxShape;
	class b3_EdgeShape;

	// �ཻ(intersect)�γɵĽ��������(Feature)  �������4���ֽڻ���١�
	struct b3_ContactFeature
	{
		enum Type
		{
			e_vertex = 0,
			e_face = 1
		};

		unsigned char indexA;		// shapeA�ϵ���������
		unsigned char indexB;		// shapeB�ϵ���������
		unsigned char typeA;		// shapeA�ϵ���������
		unsigned char typeB;		// shapeB�ϵ���������
	};


	// Contact ID�Դٽ���������Contact ids to facilitate warm starting.
	union b3_ContactID
	{
		b3_ContactFeature cf;
		uint32_t key;				// ���ڿ��ٱȽ�contactID
	};

	/*
	    һ��manifold��������contact manifold�Ľ��㡣����������Ӵ���ļ�����״�Ͷ���ѧ��ص���ϸ��Ϣ�� 
        �ֲ����ʹ��ȡ����manifold���ͣ� 
        -e_circles��ԲB�ľֲ����� 
        -e_faceA��ԲB�ľֲ����Ļ�����B�ļ��õ� 
        -e_faceB�������A�ļ��õ� 
        ����ṹ�ǿ�ʱ�䲽���洢�ģ��������Ǳ�������С�� 
        ע�⣺���������ڲ����棬�����޷��ṩ�ɿ���contact�����ر����ڸ�����ײʱ��
	*/
	struct b3_ManifoldPoint
	{
		glm::vec3 localPoint;	// ��һ������ײ����Ľ���λ�ã���������������е�ĳ��
		float normalImpulse;	// �Ǵ�͸(non-penetration)����, ��b3_ContactSolver.StoreImpulses�����б���ֵ
		float tangentImpulse;	// Ħ������
		b3_ContactID id;		// Ψһ��ʶ������״֮��Ľ���
	};

	/*
	    ��һ����������һ�����巢����ײʱ����ײ���ģ�������һ��Manifold�����Manifold��������ײ�ľ�����Ϣ����Ӵ��㡢���ߡ����ߵ�
        Box3D֧�ֶ������͵ĽӴ�(contact)�� 
        -���õ�����а뾶��ƽ�� (clip point versus plane with radius)
        -����뾶�㣨Բ�� 
        local point ��ʹ��ȡ����manifold���ͣ� 
        -e_circles��ԲA�ľֲ��е� 
        -e_faceA��faceA���е� 
        -e_faceB: faceB���е� 
        ���Ƶأ�local normal ʹ�ã� 
        -e_circles��δʹ�� 
        -e_faceA��boxA�ϵķ��� 
        -e_faceB��boxB�ϵķ��� 
        ���������ַ�ʽ�洢���㣬�Ա�λ��У��(correction)���Խ����˶��������������ѧ������Ҫ(critical)�� 
        ����contact����(scenarios)������������һ�����ͱ�ʾ�� 
        ����ṹ�ǿ�ʱ�䲽��(time steps)�洢�ģ��������Ǳ�������С��
	*/
	struct b3_Manifold
	{
		enum Type
		{
			e_circles,
			e_faceA,
			e_faceB
		};

		b3_ManifoldPoint points[b3_MaxManifoldPoints];	// ������ײ����һ��body���е�
		glm::vec3 localNormal;							// e_circles��δʹ�ã�e_faceA����ײ�����߻���ķ���������λ����
		glm::vec3 localPoint;							// e_circles��������ײ��body���е�, e_faceA����ײ����
		Type type;
		int pointCount;								    // ��������
	};

	// �����ڼ���contact manifold�ĵ�ǰ״̬��
	struct b3_WorldManifold
	{
		// ʹ���ṩ��transforms��manifold��������(Evaluate)�������(assumes)�˴�ԭʼ״̬��ʼ���ʶ�(modest)�˶���
		// �ⲻ��ı��������������ȡ��뾶������������manifold��shape��
		// ����bodyA��bodyB��A->B��λ����normal������ͽ��㵽�߽�ľ��������������壬����Ϊ(A+rA*normal+B-rB*normal)/2
		void Initialize(const b3_Manifold* manifold,
			const b3_Transform& transformA, float radiusA,
			const b3_Transform& transformB, float radiusB);

		glm::vec3 normal;							// ���㷨��������λ����
		glm::vec3 points[b3_MaxManifoldPoints];		// �����б�
		float separations[b3_MaxManifoldPoints];	// ��ֵ��ʾ(indicates)�ص�����λΪ��
	};

	// ������ȷ��(determining)�Ӵ����״̬
	enum b3_PointState
	{
		e_nullState,	// point does not exist
		e_addState,		// point was added in the update
		e_persistState,	// point ���������¹����г������� persisted across the update
		e_removeState	// point was removed in the update
	};

	// �����������manifolds�ĵ�״̬����Щ״̬���manifold1��manifold2�Ĺ���(transition)�й�(pertain)��
	// ��ˣ�state1Ҫô�־û�(persist)Ҫôɾ��(remove)����state2Ҫô���(add)Ҫô�־û�(persist)��
	//void b3_GetPointStates(b3_PointState state1[b3_MaxManifoldPoints], b3_PointState state2[b3_MaxManifoldPoints], const b3_Manifold* manifold1, const b3_Manifold* manifold2);

	// ���ڼ���contact manifolds.
	struct b3_ClipVertex
	{
		glm::vec3 vertex;  // ��ײ�߶������������
		b3_ContactID id;
	};

	// ���߼���������ݣ����ߴ� p1 ���쵽 p1 + maxFraction * (p2 - p1)��
	struct b3_RayCastInput
	{
		glm::vec3 p1, p2;
		float maxFraction;
	};

	// ���߼��������ݣ������� p1 + fraction * (p2 - p1) ���ཻ������p1��p2����b3_RayCastInput��
	struct b3_RayCastOutput
	{
		glm::vec3 normal;  // �����������귨����
		float fraction;    // ����ķ���
	};

	// �����߽��(axis aligned bounding box)
	struct b3_AABB
	{
		// ��ȡAABB�����ĵ� 0.5f * (lowerBound + upperBound)
		glm::vec3 GetCenter() const { return 0.5f * (lowerBound + upperBound); }

		// ��ȡAABB�ķ�Χ(extents)  0.5f * (upperBound - lowerBound).
		glm::vec3 GetExtents() const { return 0.5f * (upperBound - lowerBound); }

		// ��ȡ�ܳ� Get the perimeter length
		float GetPerimeter() const
		{
			float wx = upperBound.x - lowerBound.x;
			float wy = upperBound.y - lowerBound.y;
			float wz = upperBound.z - lowerBound.z;
			return 4.0f * (wx + wy + wz);
		}

		// Combine an AABB into this one.
		void Combine(const b3_AABB& aabb)
		{
			lowerBound = glm::min(lowerBound, aabb.lowerBound);
			upperBound = glm::max(upperBound, aabb.upperBound);
		}

		// Combine two AABBs into this one.
		void Combine(const b3_AABB& aabb1, const b3_AABB& aabb3_)
		{
			lowerBound = glm::min(aabb1.lowerBound, aabb3_.lowerBound);
			upperBound = glm::max(aabb1.upperBound, aabb3_.upperBound);
		}

		// ��֤�߽��Ƿ�������Verify that the bounds are sorted.
		bool IsValid() const;

		// �Ƿ����(contain)aabb
		bool Contains(const b3_AABB& aabb) const
		{
			bool result = true;
			result = result && lowerBound.x <= aabb.lowerBound.x;
			result = result && lowerBound.y <= aabb.lowerBound.y;
			result = result && lowerBound.z <= aabb.lowerBound.z;
			result = result && aabb.upperBound.x <= upperBound.x;
			result = result && aabb.upperBound.y <= upperBound.y;
			result = result && aabb.upperBound.z <= upperBound.z;
			return result;
		}

		bool RayCast(b3_RayCastOutput* output, const b3_RayCastInput& input) const;

		glm::vec3 lowerBound;	// the lower vertex
		glm::vec3 upperBound;	// the upper vertex
	};


	/*
	// Compute the collision manifold between two spheres.
	void b3_CollideSpheres(b3_Manifold* manifold,
		const b3_SphereShape* sphereA, const b3_Transform& transformA,
		const b3_SphereShape* sphereB, const b3_Transform& transformB);

	// Compute the collision manifold between a box and a sphere.
	void b3_CollideBoxAndSphere(b3_Manifold* manifold,
		const b3_BoxShape* boxA, const b3_Transform& transformA,
		const b3_SphereShape* sphereB, const b3_Transform& transformB);

	// Compute the collision manifold between two boxs.
	void b3_CollideBoxs(b3_Manifold* manifold,
		const b3_BoxShape* boxA, const b3_Transform& transformA,
		const b3_BoxShape* boxB, const b3_Transform& transformB);

	// Compute the collision manifold between an edge and a sphere.
	void b3_CollideEdgeAndSphere(b3_Manifold* manifold,
		const b3_EdgeShape* boxA, const b3_Transform& transformA,
		const b3_SphereShape* sphereB, const b3_Transform& transformB);

	// Compute the collision manifold between an edge and a box.
	void b3_CollideEdgeAndBox(b3_Manifold* manifold,
		const b3_EdgeShape* edgeA, const b3_Transform& transformA,
		const b3_BoxShape* boxB, const b3_Transform& transformB);

	// Clipping for contact manifolds.
	int b3_ClipSegmentToLine(b3_ClipVertex vOut[2], const b3_ClipVertex vIn[2],
		const glm::vec3& normal, float offset, int vertexIndexA);
		*/

	// ȷ��(Determine)����ͨ��(generic)shapes�Ƿ��ص���
	bool b3_TestOverlap(const b3_Shape* shapeA, int indexA, const b3_Shape* shapeB, int indexB, const b3_Transform& transformA, const b3_Transform& transformB);


	// ---------------- Inline Functions ------------------------------------------

	inline bool b3_AABB::IsValid() const
	{
		glm::vec3 d = upperBound - lowerBound;
		bool valid = d.x >= 0.0f && d.y >= 0.0f;
		valid = valid && isfinite(lowerBound.x) && isfinite(lowerBound.y) && isfinite(lowerBound.z) && 
			             isfinite(upperBound.x) && isfinite(upperBound.y) && isfinite(upperBound.z);
		return valid;
	}

	inline bool b3_TestOverlap(const b3_AABB& a, const b3_AABB& b)
	{
		if (a.lowerBound.x >= b.upperBound.x) return false;
		if (a.upperBound.x <= b.lowerBound.x) return false;
		if (a.lowerBound.y >= b.upperBound.y) return false;
		if (a.upperBound.y <= b.lowerBound.y) return false;
		if (a.lowerBound.z >= b.upperBound.z) return false;
		if (a.upperBound.z <= b.lowerBound.z) return false;
		return true;
	}
	
	// ������AABB�ཻ����, @return -1 ���ཻ��[0,1] ����������
	inline float b3_RayAABBIntersect(const glm::vec3& p1, const glm::vec3& p2, b3_AABB aabb)
	{
		glm::vec3 pp = p2 - p1;
		// ������AABB�ཻ����
		bool inside = true;
		float xt, xn;
		if (p1.x < aabb.lowerBound.x)
		{
			xt = aabb.lowerBound.x - p1.x;
			if (xt > pp.x) return -1.0f;
			xt /= pp.x;
			inside = false;
			xn = -1.0f;
		}
		else if (p1.x > aabb.upperBound.x)
		{
			xt = aabb.upperBound.x - p1.x;
			if (xt < pp.x) return -1.0f;
			xt /= pp.x;
			inside = false;
			xn = 1.0f;
		}
		else
			xt = -1.0f;

		float yt, yn;
		if (p1.y < aabb.lowerBound.y)
		{
			yt = aabb.lowerBound.y - p1.y;
			if (yt > pp.y) return -1.0f;
			yt /= pp.y;
			inside = false;
			yn = -1.0f;
		}
		else if (p1.y > aabb.upperBound.y)
		{
			yt = aabb.upperBound.y - p1.y;
			if (yt < pp.y) return -1.0f;
			yt /= pp.y;
			inside = false;
			yn = 1.0f;
		}
		else
			yt = -1.0f;

		float zt, zn;
		if (p1.z < aabb.lowerBound.z)
		{
			zt = aabb.lowerBound.z - p1.z;
			if (zt > pp.z) return -1.0f;
			zt /= pp.z;
			inside = false;
			zn = -1.0f;
		}
		else if (p1.z > aabb.upperBound.z)
		{
			zt = aabb.upperBound.z - p1.z;
			if (zt < pp.z) return -1.0f;
			zt /= pp.z;
			inside = false;
			zn = 1.0f;
		}
		else
			zt = -1.0f;

		int which = 0;
		float t = xt;   // ���������� [0,1]
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
		case 0: //��yzƽ���ཻ
		{
			float y = p1.y + pp.y * t;
			if (y < aabb.lowerBound.y || y > aabb.upperBound.y) return -1.0f;
			float z = p1.z + pp.z * t;
			if (z < aabb.lowerBound.z || z > aabb.upperBound.z) return -1.0f;
		}
		break;

		case 1: //��xzƽ���ཻ
		{
			float x = p1.x + pp.x * t;
			if (x < aabb.lowerBound.x || x > aabb.upperBound.x) return -1.0f;
			float z = p1.z + pp.z * t;
			if (z < aabb.lowerBound.z || z > aabb.upperBound.z) return -1.0f;
		}
		break;

		case 2: //��xyƽ���ཻ
		{
			float x = p1.x + pp.x * t;
			if (x < aabb.lowerBound.x || x > aabb.upperBound.x) return -1.0f;
			float y = p1.y + pp.y * t;
			if (y < aabb.lowerBound.y || y > aabb.upperBound.y) return -1.0f;
		}
		break;

		default:
			break;
		}

		return t;
	}

	// ��p�Ƿ���aabb��
	static bool b3_PointInsideAABB(const glm::vec3& p, b3_AABB aabb)
	{
		if (p.x < aabb.lowerBound.x || p.x > aabb.upperBound.x || 
			p.y < aabb.lowerBound.y || p.y > aabb.upperBound.y || 
			p.z < aabb.lowerBound.z || p.z > aabb.upperBound.z)
			return false;
		return true;
	}

}