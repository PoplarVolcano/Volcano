#pragma once

#include "b3_Math.h"

namespace Volcano {

	class b3_Shape;
	class b3_SphereShape;
	class b3_BoxShape;
	class b3_EdgeShape;

	// 相交(intersect)形成的交点的特征(Feature)  这必须是4个字节或更少。
	struct b3_ContactFeature
	{
		enum Type
		{
			e_vertex = 0,
			e_face = 1
		};

		unsigned char indexA;		// shapeA上的特征索引
		unsigned char indexB;		// shapeB上的特征索引
		unsigned char typeA;		// shapeA上的特征类型
		unsigned char typeB;		// shapeB上的特征类型
	};


	// Contact ID以促进热启动。Contact ids to facilitate warm starting.
	union b3_ContactID
	{
		b3_ContactFeature cf;
		uint32_t key;				// 用于快速比较contactID
	};

	/*
	    一个manifold点是属于contact manifold的交点。它保存了与接触点的几何形状和动力学相关的详细信息。 
        局部点的使用取决于manifold类型： 
        -e_circles：圆B的局部中心 
        -e_faceA：圆B的局部中心或多边形B的剪裁点 
        -e_faceB：多边形A的剪裁点 
        这个结构是跨时间步长存储的，所以我们保持它很小。 
        注意：冲量用于内部缓存，可能无法提供可靠的contact力，特别是在高速碰撞时。
	*/
	struct b3_ManifoldPoint
	{
		glm::vec3 localPoint;	// 另一个被碰撞物体的交点位置，单个点或三角形中的某点
		float normalImpulse;	// 非穿透(non-penetration)冲量, 在b3_ContactSolver.StoreImpulses方法中被赋值
		float tangentImpulse;	// 摩擦冲量
		b3_ContactID id;		// 唯一标识两个形状之间的交点
	};

	/*
	    当一个物体与另一个物体发生碰撞时，碰撞检测模块会生成一个Manifold，这个Manifold包含了碰撞的具体信息，如接触点、法线、切线等
        Box3D支持多种类型的接触(contact)： 
        -剪裁点与具有半径的平面 (clip point versus plane with radius)
        -点与半径点（圆） 
        local point 的使用取决于manifold类型： 
        -e_circles：圆A的局部中点 
        -e_faceA：faceA的中点 
        -e_faceB: faceB的中点 
        类似地，local normal 使用： 
        -e_circles：未使用 
        -e_faceA：boxA上的法线 
        -e_faceB：boxB上的法线 
        我们以这种方式存储交点，以便位置校正(correction)可以解释运动，这对连续物理学至关重要(critical)。 
        所有contact方案(scenarios)都必须用其中一种类型表示。 
        这个结构是跨时间步长(time steps)存储的，所以我们保持它很小。
	*/
	struct b3_Manifold
	{
		enum Type
		{
			e_circles,
			e_faceA,
			e_faceB
		};

		b3_ManifoldPoint points[b3_MaxManifoldPoints];	// 发生碰撞的另一个body的中点
		glm::vec3 localNormal;							// e_circles：未使用，e_faceA：碰撞交点或边或面的法向量，单位向量
		glm::vec3 localPoint;							// e_circles：发生碰撞的body的中点, e_faceA：碰撞交点
		Type type;
		int pointCount;								    // 交点数量
	};

	// 这用于计算contact manifold的当前状态。
	struct b3_WorldManifold
	{
		// 使用提供的transforms对manifold进行评估(Evaluate)。这假设(assumes)了从原始状态开始的适度(modest)运动。
		// 这不会改变点的数量、冲量等。半径必须来自生成manifold的shape。
		// 计算bodyA的bodyB的A->B单位向量normal，交点和交点到边界的距离差，对于两个球体，交点为(A+rA*normal+B-rB*normal)/2
		void Initialize(const b3_Manifold* manifold,
			const b3_Transform& transformA, float radiusA,
			const b3_Transform& transformB, float radiusB);

		glm::vec3 normal;							// 交点法向量，单位向量
		glm::vec3 points[b3_MaxManifoldPoints];		// 交点列表
		float separations[b3_MaxManifoldPoints];	// 负值表示(indicates)重叠，单位为米
	};

	// 这用于确定(determining)接触点的状态
	enum b3_PointState
	{
		e_nullState,	// point does not exist
		e_addState,		// point was added in the update
		e_persistState,	// point 在整个更新过程中持续存在 persisted across the update
		e_removeState	// point was removed in the update
	};

	// 计算给定两个manifolds的点状态。这些状态与从manifold1到manifold2的过渡(transition)有关(pertain)。
	// 因此，state1要么持久化(persist)要么删除(remove)，而state2要么添加(add)要么持久化(persist)。
	//void b3_GetPointStates(b3_PointState state1[b3_MaxManifoldPoints], b3_PointState state2[b3_MaxManifoldPoints], const b3_Manifold* manifold1, const b3_Manifold* manifold2);

	// 用于计算contact manifolds.
	struct b3_ClipVertex
	{
		glm::vec3 vertex;  // 碰撞边顶点的世界坐标
		b3_ContactID id;
	};

	// 射线检测输入数据，光线从 p1 延伸到 p1 + maxFraction * (p2 - p1)。
	struct b3_RayCastInput
	{
		glm::vec3 p1, p2;
		float maxFraction;
	};

	// 射线检测输出数据，光线在 p1 + fraction * (p2 - p1) 处相交，其中p1和p2来自b3_RayCastInput。
	struct b3_RayCastOutput
	{
		glm::vec3 normal;  // 交点世界坐标法向量
		float fraction;    // 交点的分数
	};

	// 轴对齐边界框(axis aligned bounding box)
	struct b3_AABB
	{
		// 获取AABB的中心点 0.5f * (lowerBound + upperBound)
		glm::vec3 GetCenter() const { return 0.5f * (lowerBound + upperBound); }

		// 获取AABB的范围(extents)  0.5f * (upperBound - lowerBound).
		glm::vec3 GetExtents() const { return 0.5f * (upperBound - lowerBound); }

		// 获取周长 Get the perimeter length
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

		// 验证边界是否已排序。Verify that the bounds are sorted.
		bool IsValid() const;

		// 是否包含(contain)aabb
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

	// 确定(Determine)两个通用(generic)shapes是否重叠。
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
	
	// 光线与AABB相交测试, @return -1 不相交；[0,1] 参数化交点
	inline float b3_RayAABBIntersect(const glm::vec3& p1, const glm::vec3& p2, b3_AABB aabb)
	{
		glm::vec3 pp = p2 - p1;
		// 光线与AABB相交测试
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
			if (y < aabb.lowerBound.y || y > aabb.upperBound.y) return -1.0f;
			float z = p1.z + pp.z * t;
			if (z < aabb.lowerBound.z || z > aabb.upperBound.z) return -1.0f;
		}
		break;

		case 1: //与xz平面相交
		{
			float x = p1.x + pp.x * t;
			if (x < aabb.lowerBound.x || x > aabb.upperBound.x) return -1.0f;
			float z = p1.z + pp.z * t;
			if (z < aabb.lowerBound.z || z > aabb.upperBound.z) return -1.0f;
		}
		break;

		case 2: //与xy平面相交
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

	// 点p是否在aabb内
	static bool b3_PointInsideAABB(const glm::vec3& p, b3_AABB aabb)
	{
		if (p.x < aabb.lowerBound.x || p.x > aabb.upperBound.x || 
			p.y < aabb.lowerBound.y || p.y > aabb.upperBound.y || 
			p.z < aabb.lowerBound.z || p.z > aabb.upperBound.z)
			return false;
		return true;
	}

}