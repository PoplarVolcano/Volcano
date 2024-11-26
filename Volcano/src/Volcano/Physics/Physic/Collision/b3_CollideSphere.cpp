#include "volpch.h"

#include "b3_CollideSphere.h"
#include "b3_SphereShape.h"
#include "b3_BoxShape.h"

namespace Volcano {

	void b3_CollideSpheres(b3_Manifold* manifold, 
		const b3_SphereShape* sphereA, const b3_Transform& transformA, 
		const b3_SphereShape* sphereB, const b3_Transform& transformB)
	{
		manifold->pointCount = 0;

		glm::vec3 pA = b3_Multiply(transformA, sphereA->m_position);
		glm::vec3 pB = b3_Multiply(transformB, sphereB->m_position);

		glm::vec3 d = pB - pA;
		float distSqr = glm::dot(d, d);
		float rA = sphereA->m_radius, rB = sphereB->m_radius;
		float radius = rA + rB;
		if (distSqr > radius * radius)
		{
			return;
		}

		manifold->type = b3_Manifold::e_circles;
		manifold->localPoint = sphereA->m_position;
		manifold->localNormal = { 0.0f, 0.0f, 0.0f };
		manifold->pointCount = 1;

		manifold->points[0].localPoint = sphereB->m_position;
		manifold->points[0].id.key = 0;
	}

	void b3_CollideBoxAndSphere(b3_Manifold* manifold, 
		const b3_BoxShape* boxA, const b3_Transform& transformA, 
		const b3_SphereShape* sphereB, const b3_Transform& transformB)
	{

		manifold->pointCount = 0;

		// 计算立方体框架中的球的位置
		glm::vec3 c = b3_Multiply(transformB, sphereB->m_position);
		glm::vec3 cLocal = b3_MultiplyT(transformA, c);

		// 找到最小间隔的三角形
		int normalIndex = 0;
		float separation = -FLT_MAX;
		float radius = boxA->m_radius + sphereB->m_radius;
		int vertexCount = getArrayLength(boxA->m_indices);
		const glm::vec3* vertices = boxA->m_vertices;
		const int* indices = boxA->m_indices;

		for (int i = 0; i < vertexCount; ++i)
		{
			float s = glm::dot(boxA->GetNormal(i), cLocal - vertices[indices[i]]);

			if (s > radius)
			{
				return;
			}

			if (s > separation)
			{
				separation = s;
				normalIndex = i;
			}
		}

		// 碰撞面的顶点
		int triangleIndex = normalIndex - normalIndex % 3;
		int index1 = normalIndex;
		int index2 = (normalIndex + 1) % 3 + triangleIndex;
		int index3 = (normalIndex + 2) % 3 + triangleIndex;

		glm::vec3 v1 = vertices[indices[index1]];
		glm::vec3 v2 = vertices[indices[index2]];
		glm::vec3 v3 = vertices[indices[index3]];


		// 如果球心在立方体内
		if (separation < b3_Epsilon)
		{
			manifold->pointCount = 1;
			manifold->type = b3_Manifold::e_faceA;
			manifold->localNormal = boxA->GetNormal(normalIndex);
			manifold->localPoint = (v1 + v2 + v3) / 3.0f;
			manifold->points[0].localPoint = sphereB->m_position;
			manifold->points[0].id.key = 0;
			return;
		}

		// 计算重心(barycentric)坐标
		float u1 = glm::dot(cLocal - v1, (v1 + v2 + v3) / 3.0f - v1);
		float u2 = glm::dot(cLocal - v2, (v1 + v2 + v3) / 3.0f - v2);
		float u3 = glm::dot(cLocal - v3, (v1 + v2 + v3) / 3.0f - v3);
		if (u1 <= 0.0f)
		{
			if (glm::dot(cLocal - v1) > radius * radius)
			{
				return;
			}

			manifold->pointCount = 1;
			manifold->type = b3_Manifold::e_faceA;
			manifold->localNormal = glm::normalize(cLocal - v1);
			manifold->localPoint = v1;
			manifold->points[0].localPoint = sphereB->m_position;
			manifold->points[0].id.key = 0;
		}
		else if (u2 <= 0.0f)
		{
			if (glm::dot(cLocal - v2) > radius * radius)
			{
				return;
			}

			manifold->pointCount = 1;
			manifold->type = b3_Manifold::e_faceA;
			manifold->localNormal = glm::normalize(cLocal - v2);
			manifold->localPoint = v2;
			manifold->points[0].localPoint = sphereB->m_position;
			manifold->points[0].id.key = 0;
		}
		else if (u3 <= 0.0f)
		{
			if (glm::dot(cLocal - v3) > radius * radius)
			{
				return;
			}

			manifold->pointCount = 1;
			manifold->type = b3_Manifold::e_faceA;
			manifold->localNormal = glm::normalize(cLocal - v3);
			manifold->localPoint = v3;
			manifold->points[0].localPoint = sphereB->m_position;
			manifold->points[0].id.key = 0;
		}
		else
		{
			glm::vec3 faceCenter = (v1 + v2 + v3) / 3.0f;
			float s = glm::dot(cLocal - faceCenter, boxA->GetNormal(index1));
			if (s > radius)
			{
				return;
			}

			manifold->pointCount = 1;
			manifold->type = b3_Manifold::e_faceA;
			manifold->localNormal = boxA->GetNormal(index1);
			manifold->localPoint = faceCenter;
			manifold->points[0].localPoint = sphereB->m_position;
			manifold->points[0].id.key = 0;
		}
	}

	// 通过boxA的法线获取boxA和boxB的间隔
	static float b3_FindMaxSeparation(int* vertexIndex,
		const b3_BoxShape* boxA, const b3_Transform& transform1,
		const b3_BoxShape* boxB, const b3_Transform& transform2)
	{

		const glm::vec3* verticesA = boxA->m_vertices;
		const int* indicesA = boxA->m_indices;
		const glm::vec3* verticesB = boxB->m_vertices;
		const int* indicesB = boxB->m_indices;
		int count1 = getArrayLength(boxA->m_indices);
		int count2 = getArrayLength(boxB->m_indices);
		b3_Transform transform = b3_MultiplyT(transform2, transform1);

		int bestIndex = 0;
		float maxSeparation = -FLT_MAX;
		// 遍历boxA的顶点，获取投影最大的点
		for (int i = 0; i < count1; ++i)
		{
			// boxA的顶点和法向量在boxB的局部坐标
			glm::vec normal = boxA->GetNormal(i);
			glm::vec vertex = verticesA[indicesA[i]];
			glm::vec3 nromalA = b3_Multiply(transform.rotation, normal);
			glm::vec3 vertexA = b3_Multiply(transform, vertex);

			// 找到法线i的最深点。 Find deepest point for normal i.
			float si = FLT_MAX;
			for (int j = 0; j < count2; ++j)
			{
				// 向量顶点A->B在对应法线上的投影，遍历boxB的顶点获取投影最小的点的投影
				float sij = glm::dot(nromalA, verticesB[indicesB[j]] - vertexA);
				if (sij < si)
					si = sij;
			}

			if (si > maxSeparation)
			{
				maxSeparation = si;
				bestIndex = i;
			}
		}

		*vertexIndex = bestIndex;
		return maxSeparation;
	}

	//获取碰撞三角，将顶点注入clipTriangle
	static void b3_FindIncidentTriangle(b3_ClipVertex clipTriangle[3],
		const b3_BoxShape* boxA, const b3_Transform& transformA, int indexA,
		const b3_BoxShape* boxB, const b3_Transform& transformB)
	{
		const int* indicesA = boxA->m_indices;
		const glm::vec3* verticesB = boxB->m_vertices;
		const int* indicesB = boxB->m_indices;
		int count2 = getArrayLength(boxB->m_indices);

		assert(0 <= indexA && indexA < getArrayLength(boxA->m_indices));

		// 获取boxB坐标系中参考box的法向量
		glm::vec3 normalA = b3_MultiplyT(transformB.rotation, b3_Multiply(transformA.rotation, boxA->GetNormal(indexA)));

		// 在boxB上找到碰撞法线点
		int index = 0;
		float minDot = FLT_MAX;
		// 遍历boxB的法线，找到与normalA夹角最接近-PI的法线
		for (int i = 0; i < count2; ++i)
		{
			float dot = glm::dot(normalA, boxB->GetNormal(i));
			if (dot < minDot)
			{
				minDot = dot;
				index = i;
			}
		}

		// 为碰撞边构建交点。 Build the clip vertices for the incident edge.
		int triangleIndex = index - index % 3;
		int index1 = index;
		int index2 = (index + 1) % 3 + triangleIndex;
		int index3 = (index + 2) % 3 + triangleIndex;

		clipTriangle[0].vertex = b3_Multiply(transformB, verticesB[indicesB[index1]]);
		clipTriangle[0].id.cf.indexA = (unsigned char)indexA;
		clipTriangle[0].id.cf.indexB = (unsigned char)index1;
		clipTriangle[0].id.cf.typeA = b3_ContactFeature::e_face;
		clipTriangle[0].id.cf.typeB = b3_ContactFeature::e_vertex;

		clipTriangle[1].vertex = b3_Multiply(transformB, verticesB[indicesB[index2]]);
		clipTriangle[1].id.cf.indexA = (unsigned char)indexA;
		clipTriangle[1].id.cf.indexB = (unsigned char)index2;
		clipTriangle[1].id.cf.typeA = b3_ContactFeature::e_face;
		clipTriangle[1].id.cf.typeB = b3_ContactFeature::e_vertex;

		clipTriangle[2].vertex = b3_Multiply(transformB, verticesB[indicesB[index3]]);
		clipTriangle[2].id.cf.indexA = (unsigned char)indexA;
		clipTriangle[2].id.cf.indexB = (unsigned char)index3;
		clipTriangle[2].id.cf.typeA = b3_ContactFeature::e_face;
		clipTriangle[2].id.cf.typeB = b3_ContactFeature::e_vertex;
	}

	float RayTriangleIntersect(const glm::vec3& rayOrg, const glm::vec3& rayDelta,
		const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2
	) {
		const float kNoIntersection = FLT_MAX;

		//计算逆时针矢量
		glm::vec3 e1 = p1 - p0;
		glm::vec3 e2 = p2 - p1;

		glm::vec3 n = glm::cross(e1, e2);

		//计算梯度(Gradient)，梯度将提示接触三角形正面的坡度，光线在三角形垂线上的投影，负值
		float dot = glm::dot(n, rayDelta);

		// 检测光线是否和三角形平行，或者未指向三角形正面
		if (!(dot < 0.0f))
			return kNoIntersection;

		// 计算平面公式的d值，将使用d在右侧的平面公式，Ax+By+cz=d
		float d = glm::dot(n, p0);

		// 计算和平面相交的参数化点，光线起点在三角形垂线上的投影，负值或0
		float t = d - glm::dot(n, rayOrg); 

		// 光线原点是否在多边形的背面
		if (!(t <= 0.0f))
			return kNoIntersection;
		
		t /= dot;
		assert(t >= 0.0f);

		// 交点
		glm::vec3 p = rayOrg + rayDelta * t;

		// 查找主轴以选择要投影到其上的平面，计算uv
		float u0, u1, u2;
		float v0, v1, v2;
		if (fabs(n.x) > fabs(n.y)) {
			if (fabs(n.x) > fabs(n.z)) {
				u0 = p.y - p0.y;
				u1 = p1.y - p0.y;
				u2 = p2.y - p0.y;

				v0 = p.z - p0.z;
				v1 = p1.z - p0.z;
				v2 = p2.z - p0.z;
			}
			else {
				u0 = p.x - p0.x;
				u1 = p1.x - p0.x;
				u2 = p2.x - p0.x;

				v0 = p.y - p0.y;
				v1 = p1.y - p0.y;
				v2 = p2.y - p0.y;
			}
		}
		else {
			if (fabs(n.y) > fabs(n.z)) {
				u0 = p.x - p0.x;
				u1 = p1.x - p0.x;
				u2 = p2.x - p0.x;

				v0 = p.z - p0.z;
				v1 = p1.z - p0.z;
				v2 = p2.z - p0.z;
			}
			else {
				u0 = p.x - p0.x;
				u1 = p1.x - p0.x;
				u2 = p2.x - p0.x;

				v0 = p.y - p0.y;
				v1 = p1.y - p0.y;
				v2 = p2.y - p0.y;
			}
		}

		// 计算分母，检查是否合法
		float temp = u1 * v2 - v1 * u2;
		if (!(temp != 0.0f))
			return kNoIntersection;
		temp = 1.0f / temp;

		// 计算重心坐标，在每个步骤检查是否查出范围
		float alpha = (u0 * v2 - v0 * u2) * temp;
		if (!(alpha >= 0.0f))
			return kNoIntersection;

		float beta = (u1 * v0 - v1 * u0) * temp;
		if (!(beta >= 0.0f))
			return kNoIntersection;

		float gamma = 1 - alpha - beta;
		if (!(gamma >= 0.0f))
			return kNoIntersection;

		return t;

	}

	// 萨瑟兰·霍奇曼剪辑。 Sutherland-Hodgman clipping.
	// 可能的情况，distance0 > 0, distance1 > 0, count == 0;
	//             distance0 = 0, distance1 > 0, count == 1;
	//             distance0 < 0, distance1 > 0, count == 2;
	//             distance0 > 0, distance1 = 0, count == 1;
	//             distance0 = 0, distance1 = 0, count == 2;
	//             distance0 < 0, distance1 = 0, count == 2;
	//             distance0 > 0, distance1 < 0, count == 2;
	//             distance0 = 0, distance1 < 0, count == 2;
	//             distance0 < 0, distance1 < 0, count == 2;
	int b3_ClipSegmentToLine(b3_ClipVertex vOut[2], const b3_ClipVertex vIn[2],
		const glm::vec3& normal, float offset, int indexRef)
	{

		// Start with no output points
		int count = 0;

		// 计算端点到直线的距离 Calculate the distance of end points to the line
		float projection1 = glm::dot(normal, vIn[0].vertex);
		float projection2 = glm::dot(normal, vIn[1].vertex);
		float distance0 = glm::dot(normal, vIn[0].vertex) - offset;
		float distance1 = glm::dot(normal, vIn[1].vertex) - offset;


		// 如果这些点在平面后面 If the points are behind the plane
		if (distance0 <= 0.1f * b3_LinearSlop) vOut[count++] = vIn[0];
		if (distance1 <= 0.1f * b3_LinearSlop) vOut[count++] = vIn[1];

		// 如果这些点位于平面的不同侧面，将点裁剪 If the points are on different sides of the plane
		if (distance0 * distance1 < 0.0f && count == 1)
		{
			// 找到边和平面的交点 Find intersection point of edge and plane
			float interp = distance0 / (distance0 - distance1);
			vOut[count].vertex = vIn[0].vertex + interp * (vIn[1].vertex - vIn[0].vertex);

			// 顶点A碰到边B。 VertexA is hitting edgeB.
			vOut[count].id.cf.indexA = static_cast<unsigned char>(indexRef);
			vOut[count].id.cf.indexB = vIn[0].id.cf.indexB;
			vOut[count].id.cf.typeA = b3_ContactFeature::e_vertex;
			vOut[count].id.cf.typeB = b3_ContactFeature::e_face;
			++count;

			assert(count == 2);
		}
		if (count == 0)
			VOL_TRACE("Debug");
		return count;

	}

	void b3_CollideBoxs(b3_Manifold* manifold, const b3_BoxShape* boxA, const b3_Transform& transformA, const b3_BoxShape* boxB, const b3_Transform& transformB)
	{
		manifold->pointCount = 0;

		float totalRadius = boxA->m_radius + boxB->m_radius;

		int indexA = 0;
		float separationA = b3_FindMaxSeparation(&indexA, boxA, transformA, boxB, transformB);
		if (separationA > totalRadius)
			return;

		int indexB = 0;
		float separationB = b3_FindMaxSeparation(&indexB, boxB, transformB, boxA, transformA);
		if (separationB > totalRadius)
			return;

		const b3_BoxShape* boxRef;	// 参考系box reference box
		const b3_BoxShape* boxInc;	// 碰撞体box incident box
		b3_Transform transformRef, transformInc;
		int indexRef;
		bool flip;
		const float k_tol = 0.1f * b3_LinearSlop; // 公差

		if (separationB > separationA + k_tol)
		{
			boxRef = boxB;
			boxInc = boxA;
			transformRef = transformB;
			transformInc = transformA;
			indexRef = indexB;
			manifold->type = b3_Manifold::e_faceB;
			flip = true;
		}
		else
		{
			boxRef = boxA;
			boxInc = boxB;
			transformRef = transformA;
			transformInc = transformB;
			indexRef = indexA;
			manifold->type = b3_Manifold::e_faceA;
			flip = false;
		}

		const glm::vec3* verticesRef = boxRef->m_vertices;
		const int* indicesRef = boxRef->m_indices;

		int triangleIndex = indexRef - indexRef % 3;
		int index1 = indexRef;
		int index2 = (indexRef + 1) % 3 + triangleIndex;
		int index3 = (indexRef + 2) % 3 + triangleIndex;

		// 被碰撞三角形 refTriangle
		glm::vec3 v11 = verticesRef[indicesRef[index1]];
		glm::vec3 v12 = verticesRef[indicesRef[index2]];
		glm::vec3 v13 = verticesRef[indicesRef[index3]];

		glm::vec3 planePoint = (v11 + v12 + v13) / 3.0f;// 三角形重心, 局部坐标

		glm::vec3 localNormal = boxRef->GetNormal(indexRef);// 法向量
		glm::vec3 localTangent1 = glm::normalize(glm::cross(localNormal, v12 - v13)); // v1切向量,方向v1->平面v2v3
		glm::vec3 localTangent2 = glm::normalize(glm::cross(localNormal, v13 - v11)); // v2切向量,方向v2->平面v3v1
		glm::vec3 localTangent3 = glm::normalize(glm::cross(localNormal, v11 - v12)); // v3切向量,方向v3->平面v1v2

		glm::vec3 normal[4];
		normal[0] = b3_Multiply(transformRef.rotation, localNormal);  // 法向量，世界坐标
		normal[1] = b3_Multiply(transformRef.rotation, localTangent1);// v1切向量，世界坐标
		normal[2] = b3_Multiply(transformRef.rotation, localTangent2);// v2切向量，世界坐标
		normal[3] = b3_Multiply(transformRef.rotation, localTangent3);// v3切向量，世界坐标

		v11 = b3_Multiply(transformRef, v11);
		v12 = b3_Multiply(transformRef, v12);
		v13 = b3_Multiply(transformRef, v13);

		// 碰撞三角形：三个顶点，世界坐标
		b3_ClipVertex incTriangle[3];
		b3_FindIncidentTriangle(incTriangle, boxRef, transformRef, indexRef, boxInc, transformInc);

		// 裁剪incTriangle，扩增为6个点，每2个点代表一条边，共6条边
		b3_ClipVertex clipTriangle[6];
		for (int i = 0; i < 6; i++)
			clipTriangle[i] = incTriangle[i >> 1];

		// 偏移，立方体蒙皮(skin)厚度延伸。 Side offsets, extended by polytope skin thickness.
		float offset[4];
		offset[0] = glm::dot(normal[0], v11) - totalRadius;
		offset[1] = glm::dot(normal[1], v12) - totalRadius;
		offset[2] = glm::dot(normal[2], v13) - totalRadius;
		offset[3] = glm::dot(normal[3], v11) - totalRadius;

		b3_ClipVertex clipPoints[2];
		b3_ClipVertex incidentTriangleEdge[2];

		int np;
		int index;
		for(int i = 0; i < 4; i++)
		{
			index = -1;
			for (int j = 0; j < 6; j++)
				if (glm::dot(normal[i], clipTriangle[j].vertex) - offset[i] <= b3_Epsilon)
				{
					index = j;
					break;
				}

			// 所有点在一个面外，不相交
			if (index == -1)
				return;

			incidentTriangleEdge[0] = clipTriangle[index];
			incidentTriangleEdge[1] = clipTriangle[(index + 1) % 6];
			np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normal[i], offset[i], indexRef);
			// 已知distance0 <= 0，np不可能为0，np为1的情况只有一种，distance0 = 0， distance1 > 0
			if (np == 1)
				clipTriangle[(index + 1) % 6].vertex = clipPoints[0].vertex;
			else if (np == 2)
				clipTriangle[(index + 1) % 6].vertex = clipPoints[1].vertex;
			else VOL_ASSERT(false);


			incidentTriangleEdge[0] = clipTriangle[(index + 1) % 6];
			incidentTriangleEdge[1] = clipTriangle[(index + 2) % 6];
			np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normal[i], offset[i], indexRef);
			if (np == 1)
				clipTriangle[(index + 2) % 6].vertex = clipPoints[0].vertex;
			else if (np == 2)
				clipTriangle[(index + 2) % 6].vertex = clipPoints[1].vertex;
			else VOL_ASSERT(false);


			incidentTriangleEdge[0] = clipTriangle[index];
			incidentTriangleEdge[1] = clipTriangle[(index + 5) % 6];
			np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normal[i], offset[i], indexRef);
			if (np == 1)
				clipTriangle[(index + 5) % 6].vertex = clipPoints[0].vertex;
			else if (np == 2)
				clipTriangle[(index + 5) % 6].vertex = clipPoints[1].vertex;
			else VOL_ASSERT(false);

			incidentTriangleEdge[0] = clipTriangle[(index + 2) % 6];
			incidentTriangleEdge[1] = clipTriangle[(index + 3) % 6];
			np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normal[i], offset[i], indexRef);
			if (np == 1)
				clipTriangle[(index + 3) % 6].vertex = clipPoints[0].vertex;
			else if (np == 2)
				clipTriangle[(index + 3) % 6].vertex = clipPoints[1].vertex;
			else VOL_ASSERT(false);


			incidentTriangleEdge[0] = clipTriangle[(index + 5) % 6];
			incidentTriangleEdge[1] = clipTriangle[(index + 4) % 6];
			np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normal[i], offset[i], indexRef);
			if (np == 1)
				clipTriangle[(index + 4) % 6].vertex = clipPoints[0].vertex;
			else if (np == 2)
				clipTriangle[(index + 4) % 6].vertex = clipPoints[1].vertex;
			else VOL_ASSERT(false);
		}

		// 现在clipTriangle包含剪切的点
		manifold->localNormal = localNormal;
		manifold->localPoint = planePoint;

		int pointCount = 0;
		for (int i = 0; i < 6; i++)
		{
			VOL_ASSERT(clipTriangle[i].vertex != glm::vec3(FLT_MAX));

			glm::vec3 point = b3_MultiplyT(transformInc, clipTriangle[i].vertex);

			b3_ManifoldPoint* cp = manifold->points + pointCount;
			cp->localPoint = point;
			cp->id = clipTriangle[i].id;
			if (flip)
				{
					// Swap features
					b3_ContactFeature cf = cp->id.cf;
					cp->id.cf.indexA = cf.indexB;
					cp->id.cf.indexB = cf.indexA;
					cp->id.cf.typeA = cf.typeB;
					cp->id.cf.typeB = cf.typeA;
				}
			++pointCount;
		}
		//assert(pointCount >= 3);
		if (pointCount < 6)
		{
			if(pointCount > 0)
				VOL_TRACE("0 < pointCount < 6");
			return;
		}

		for (int i = 0; i < pointCount; i++)
		{
			glm::vec3 point = (manifold->points + i)->localPoint;
			VOL_TRACE("point" + std::to_string(i) + ": " + 
				std::to_string(point.x) + ", " +  std::to_string(point.y) + ", " + std::to_string(point.z));
		}

		manifold->pointCount = pointCount;
	}

}