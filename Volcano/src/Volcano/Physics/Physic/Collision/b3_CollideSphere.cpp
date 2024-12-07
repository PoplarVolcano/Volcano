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
		float separation = -FLT_MAX / 2.0f;
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
			if (glm::dot(cLocal - v1, cLocal - v1) > radius * radius)
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
			if (glm::dot(cLocal - v2, cLocal - v2) > radius * radius)
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
			if (glm::dot(cLocal - v3, cLocal - v3) > radius * radius)
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
	/*
	static float b3_FindMaxSeparation(std::vector<int>& vertexIndices,
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

		std::vector<int> bestIndex;
		float maxSeparation = -FLT_MAX / 2.0f;
		// 遍历boxA的顶点，获取投影最大的点
		for (int i = 0; i < count1; ++i)
		{
			// boxA的顶点和法向量在boxB的局部坐标
			glm::vec normal = boxA->GetNormal(i);
			glm::vec vertex = verticesA[indicesA[i]];
			glm::vec3 nromalA = b3_Multiply(transform.rotation, normal);
			glm::vec3 vertexA = b3_Multiply(transform, vertex);

			// 找到法线i的最深点。 Find deepest point for normal i.
			float si = FLT_MAX / 2.0f;
			for (int j = 0; j < count2; ++j)
			{
				// 向量顶点A->B在对应法线上的投影，遍历boxB的顶点获取投影最小的点的投影
				float sij = glm::dot(nromalA, verticesB[indicesB[j]] - vertexA);
				if (sij < si)
					si = sij;
			}

			if ((si - maxSeparation)> b3_LinearSlop)
			{
				maxSeparation = si;
				bestIndex.clear();
				bestIndex.push_back(i);
			}
			else if(glm::abs(maxSeparation - si) < b3_LinearSlop)
				bestIndex.push_back(i);
		}

		vertexIndices = bestIndex;
		return maxSeparation;
	}
	*/

	//获取碰撞三角，将顶点注入clipTriangle
	static void b3_FindIncidentTriangle(std::vector<std::array<b3_ClipVertex, 3>>& clipTriangles,
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
		std::vector<int> bestIndex;
		float minDot = FLT_MAX / 2.0f;
		// 遍历boxB的法线，找到与normalA夹角最接近-PI的法线
		for (int i = 0; i < count2; ++i)
		{
			float dot = glm::dot(normalA, boxB->GetNormal(i));
			if (glm::abs(dot - minDot) > b3_LinearSlop)
			{
				minDot = dot;
				bestIndex.clear();
				bestIndex.push_back(i);
			}
			else if(glm::abs(dot - minDot) < b3_LinearSlop)
				bestIndex.push_back(i);
		}

		// 为碰撞边构建交点。 Build the clip vertices for the incident edge.
		for (int i = 0; i < bestIndex.size(); i += 3)
		{
			VOL_ASSERT(i + 2 < bestIndex.size());
			int triangleIndex = bestIndex[i] - bestIndex[i] % 3;
			int index1 = bestIndex[i];
			int index2 = (bestIndex[i] + 1) % 3 + triangleIndex;
			int index3 = (bestIndex[i] + 2) % 3 + triangleIndex;

			std::array<b3_ClipVertex, 3> clipTriangle;
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

			clipTriangles.push_back(clipTriangle);
		}
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
		if (distance0 <= b3_LinearSlop) vOut[count++] = vIn[0];
		if (distance1 <= b3_LinearSlop) vOut[count++] = vIn[1];

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

		return count;

	}

	// 合并三角形
	std::list<int> CombineTriangle(const int* indices, const std::vector<int>& indicesIndex)
	{
		if (indicesIndex.size() % 3 != 0 || indicesIndex.size() < 3)
			return std::list<int>();

		std::list<int> combineIndices;
		combineIndices.push_back(indicesIndex[0]);
		combineIndices.push_back(indicesIndex[1]);
		combineIndices.push_back(indicesIndex[2]);

		// 将插入三角形(i,i-1)中与合并点集(j,j+1重合的边消掉
		for (int i = 3; i < indicesIndex.size(); i+=3)
		{
			int points[] = { indicesIndex[i], indicesIndex[i + 1], indicesIndex[i + 2] };
			int flag = false;
			for (int j = 0; j < 3; j++)
			{
				// 遍历combineIndices的边，是否有和points的边重合的，如果有，将points合并进combineIndices，否则3个点都加入combineIndices
				for (auto it = combineIndices.begin(); it != combineIndices.end(); it++)
				{
					if (indices[*it] == indices[points[j]])
					{
						auto itNext = it; 
						itNext++;
						if (itNext == combineIndices.end())
							itNext = combineIndices.begin();

						if (indices[*(itNext)] == indices[points[(j + 2) % 3]])
						{
							combineIndices.push_back(points[(j + 1) % 3]);
							flag = true;
							break;
						}
					}
				}
				if (flag)
					break;
			}
			if (!flag)
			{
				combineIndices.push_back(points[0]);
				combineIndices.push_back(points[1]);
				combineIndices.push_back(points[2]);
			}
		}

		// combineIndices自查
		for (auto it1 = combineIndices.begin(); it1 != combineIndices.end(); it1++)
		{
			auto it1Next = it1; 
			it1Next++;
			if (it1Next == combineIndices.end())
				it1Next = combineIndices.begin();

			auto itSecond = it1; itSecond++;
			for (auto it2 = itSecond; it2 != combineIndices.end(); it2++)
			{
				if (indices[*(it2)] == indices[*(it1)])
				{
					auto it2Prev = it2; 
					if (it2Prev == combineIndices.begin())
						it2Prev = combineIndices.end();
					it2Prev--;

					if (indices[*(it1Next)] == indices[*(it2Prev)])
					{
						it2Prev = combineIndices.erase(it2);
						if (it2Prev == combineIndices.begin())
							it2Prev = combineIndices.end();
						it2Prev--;
						combineIndices.erase(it2Prev);

						it1 = combineIndices.begin();

						break;
					}
				}
			}
		}
		return combineIndices;
	}

	float b3_FindMaxSeparation(std::vector<int>& vertexIndicesA, std::vector<int>& vertexIndicesB,
		const b3_BoxShape* boxA, const b3_Transform& transformA,
		const b3_BoxShape* boxB, const b3_Transform& transformB)
	{
		const glm::vec3 posA = b3_Multiply(transformA, boxA->m_position);
		const glm::vec3 posB = b3_Multiply(transformB, boxB->m_position);
		const glm::vec3 pp = glm::normalize(posA - posB); // B中点到A中点的单位向量，世界坐标;

		const glm::vec3* verticesA = boxA->m_vertices;
		const int* indicesA = boxA->m_indices;
		const glm::vec3* verticesB = boxB->m_vertices;
		const int* indicesB = boxB->m_indices;
		int countA = getArrayLength(boxA->m_indices);
		int countB = getArrayLength(boxB->m_indices);

		glm::vec3 normal;
		std::vector<int> indexA;
		std::vector<int> indexB;
		
		// 获取boxB与boxA中点连线与boxA相交的点及其法向量
		float t = boxA->RayBoxIntersect(b3_MultiplyT(transformA, posB), boxA->m_position, &normal);

		// 只有boxB的中点在boxA的内部时才会出现t=FLT_MAX的情况
		if (t == FLT_MAX)
			return FLT_MAX;

		for (int i = 0; i < countA; ++i)
			if (boxA->GetNormal(i) == normal)
				indexA.push_back(i);

		glm::vec3 ppLocalA = b3_MultiplyT(transformA.rotation, pp);

		float maxSeparationA = -FLT_MAX / 2.0f;
		for (int i = 0; i < indexA.size(); i++)
		{
			float si = glm::dot(-ppLocalA, verticesA[indicesA[indexA[i]]] - boxA->m_position);
			if (si > maxSeparationA)
				maxSeparationA = si;
		}


		t = boxB->RayBoxIntersect(b3_MultiplyT(transformB, posA), boxB->m_position, &normal);

		if (t == FLT_MAX)
			return FLT_MAX;

		for (int i = 0; i < countB; ++i)
			if (boxB->GetNormal(i) == normal)
				indexB.push_back(i);

		glm::vec3 ppLocalB = b3_MultiplyT(transformB.rotation, pp);

		float maxSeparationB = -FLT_MAX / 2.0f;
		for (int i = 0; i < indexB.size(); i++)
		{
			float si = glm::dot(ppLocalB, verticesB[indicesB[indexB[i]]] - boxB->m_position);
			if (si > maxSeparationB)
				maxSeparationB = si;
		}

		vertexIndicesA = indexA;
		vertexIndicesB = indexB;
		return glm::length(posA - posB) - maxSeparationB - maxSeparationA;
	}

	void b3_CollideBoxs(b3_Manifold* manifold, const b3_BoxShape* boxA, const b3_Transform& transformA, const b3_BoxShape* boxB, const b3_Transform& transformB)
	{
		manifold->pointCount = 0;

		float totalRadius = boxA->m_radius + boxB->m_radius;

		std::vector<int> indexA;
		std::vector<int> indexB;
		float separation = b3_FindMaxSeparation(indexA, indexB, boxA, transformA, boxB, transformB);
		if (separation == FLT_MAX)
			return;
		if (separation > totalRadius)
			return;

		const b3_BoxShape* boxRef;	// 参考系box reference box
		const b3_BoxShape* boxInc;	// 碰撞体box incident box
		b3_Transform transformRef, transformInc;
		std::vector<int> indexRef;
		std::vector<int> indexInc;
		bool flip;
		const float k_tol = 0.1f * b3_LinearSlop; // 公差

		boxRef = boxA;
		boxInc = boxB;
		transformRef = transformA;
		transformInc = transformB;
		indexRef = indexA;
		indexInc = indexB;
		manifold->type = b3_Manifold::e_faceA;
		flip = false;

		const glm::vec3* verticesRef = boxRef->m_vertices;
		const int* indicesRef = boxRef->m_indices;

		if (indexRef.size() == 0)
			return;

		if (indexRef.size() % 3 != 0)
		{
			int index = indexRef[0] - indexRef[0] % 3;
			int index1 = indexRef[0];
			int index2 = index + (indexRef[0] + 1) % 3;
			int index3 = index + (indexRef[0] + 2) % 3;
			indexRef.clear();
			indexRef.push_back(index1);
			indexRef.push_back(index2);
			indexRef.push_back(index3);
		}
		std::list<int> planePoints = CombineTriangle(indicesRef, indexRef);

		// 被碰撞面 refPlane
		std::vector<glm::vec3> refPlane;
		for (auto it = planePoints.begin(); it != planePoints.end(); it++)
			refPlane.push_back(verticesRef[indicesRef[*(it)]]);
		
		// refPlane重心, 局部坐标
		glm::vec3 planePoint = glm::vec3(0.0f);
		for (int i = 0; i < refPlane.size(); i++) planePoint += refPlane[i];
		planePoint = planePoint / float(refPlane.size());

		glm::vec3 localNormal = boxRef->GetNormal(indexRef[0]);// 法向量

		// 用于裁剪的面的切向量
		std::vector<glm::vec3> normals;
		normals.resize(refPlane.size());
		for (int i = 0; i < normals.size(); i++)
		{
			normals[i] = glm::normalize(glm::cross(localNormal, refPlane[i] - refPlane[(i + 1) % refPlane.size()]));
			normals[i] = b3_Multiply(transformRef.rotation, normals[i]);
		}

		glm::vec3 normal = b3_Multiply(transformRef.rotation, localNormal);// 法向量，世界坐标
		normals.push_back(normal);

		for (int i = 0; i < refPlane.size(); i++)
			refPlane[i] = b3_Multiply(transformRef, refPlane[i]);

		// 碰撞三角形：三个顶点，世界坐标
		std::vector<std::array<b3_ClipVertex, 3>> incTriangles;
		for (int i = 0; i < indexInc.size(); i += 3)
		{
			int triangleIndex = indexInc[i] - indexInc[i] % 3;
			int index1 = indexInc[i];
			int index2 = (indexInc[i] + 1) % 3 + triangleIndex;
			int index3 = (indexInc[i] + 2) % 3 + triangleIndex;

			std::array<b3_ClipVertex, 3> triangle;
			triangle[0].vertex = b3_Multiply(transformB, boxB->m_vertices[boxB->m_indices[index1]]);
			triangle[0].id.cf.indexA = (unsigned char)indexA[0];
			triangle[0].id.cf.indexB = (unsigned char)index1;
			triangle[0].id.cf.typeA = b3_ContactFeature::e_face;
			triangle[0].id.cf.typeB = b3_ContactFeature::e_vertex;

			triangle[1].vertex = b3_Multiply(transformB, boxB->m_vertices[boxB->m_indices[index2]]);
			triangle[1].id.cf.indexA = (unsigned char)indexA[0];
			triangle[1].id.cf.indexB = (unsigned char)index2;
			triangle[1].id.cf.typeA = b3_ContactFeature::e_face;
			triangle[1].id.cf.typeB = b3_ContactFeature::e_vertex;

			triangle[2].vertex = b3_Multiply(transformB, boxB->m_vertices[boxB->m_indices[index3]]);
			triangle[2].id.cf.indexA = (unsigned char)indexA[0];
			triangle[2].id.cf.indexB = (unsigned char)index3;
			triangle[2].id.cf.typeA = b3_ContactFeature::e_face;
			triangle[2].id.cf.typeB = b3_ContactFeature::e_vertex;
			incTriangles.push_back(triangle);
		}

		// 裁剪incTriangle，扩增为6个点，每2个点代表一条边，共6条边
		std::vector<std::array<b3_ClipVertex, 6>> clipTriangles;
		clipTriangles.resize(incTriangles.size());
		for(int i = 0; i < incTriangles.size(); i++)
			for (int j = 0; j < 6; j++)
				clipTriangles[i][j] = incTriangles[i][j >> 1];

		// 偏移，立方体蒙皮(skin)厚度延伸。 Side offsets, extended by polytope skin thickness.
		std::vector<float> offset;
		for (int i = 0; i < normals.size() - 1; i++)
			offset.push_back(glm::dot(normals[i], refPlane[i]) + totalRadius);
		offset.push_back(glm::dot(normals[normals.size() - 1], refPlane[0]) + totalRadius);

		b3_ClipVertex clipPoints[2];
		b3_ClipVertex incidentTriangleEdge[2];

		int np;
		int index;
		for(int i = 0; i < normals.size(); i++)
		{
			for (int j = 0; j < clipTriangles.size(); j++)
			{
				index = -1;
				for (int k = 0; k < 6; k++)
				{
					// debug
					float temp = glm::dot(normals[i], clipTriangles[j][k].vertex) - offset[i];
					if (temp <= 0.1f * b3_LinearSlop)
					{
						index = k;
						break;
					}
				}

				// 所有点在一个面外，不相交，剔除该三角形
				if (index == -1)
				{
					clipTriangles.erase(clipTriangles.begin() + j);
					j--;
					continue;
				}

				incidentTriangleEdge[0] = clipTriangles[j][index];
				incidentTriangleEdge[1] = clipTriangles[j][(index + 1) % 6];
				np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normals[i], offset[i], indexRef[0]);
				// 已知distance0 <= 0，np不可能为0，np为1的情况只有一种，distance0 = 0， distance1 > 0
				if (np == 1)
					clipTriangles[j][(index + 1) % 6].vertex = clipPoints[0].vertex;
				else if (np == 2)
					clipTriangles[j][(index + 1) % 6].vertex = clipPoints[1].vertex;
				else VOL_ASSERT(false);


				incidentTriangleEdge[0] = clipTriangles[j][(index + 1) % 6];
				incidentTriangleEdge[1] = clipTriangles[j][(index + 2) % 6];
				np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normals[i], offset[i], indexRef[0]);
				if (np == 1)
					clipTriangles[j][(index + 2) % 6].vertex = clipPoints[0].vertex;
				else if (np == 2)
					clipTriangles[j][(index + 2) % 6].vertex = clipPoints[1].vertex;
				else VOL_ASSERT(false);


				incidentTriangleEdge[0] = clipTriangles[j][index];
				incidentTriangleEdge[1] = clipTriangles[j][(index + 5) % 6];
				np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normals[i], offset[i], indexRef[0]);
				if (np == 1)
					clipTriangles[j][(index + 5) % 6].vertex = clipPoints[0].vertex;
				else if (np == 2)
					clipTriangles[j][(index + 5) % 6].vertex = clipPoints[1].vertex;
				else VOL_ASSERT(false);

				// 预防34边跨面，先将34边裁剪，34边不清楚两点情况，np为2正常裁剪，np不为2则不动，等后续裁剪
				incidentTriangleEdge[0] = clipTriangles[j][(index + 3) % 6];
				incidentTriangleEdge[1] = clipTriangles[j][(index + 4) % 6];
				np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normals[i], offset[i], indexRef[0]);
				if (np == 2)
				{
					clipTriangles[j][(index + 3) % 6].vertex = clipPoints[0].vertex;
					clipTriangles[j][(index + 4) % 6].vertex = clipPoints[1].vertex;
				}

				incidentTriangleEdge[0] = clipTriangles[j][(index + 2) % 6];
				incidentTriangleEdge[1] = clipTriangles[j][(index + 3) % 6];
				np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normals[i], offset[i], indexRef[0]);
				if (np == 1)
					clipTriangles[j][(index + 3) % 6].vertex = clipPoints[0].vertex;
				else if (np == 2)
					clipTriangles[j][(index + 3) % 6].vertex = clipPoints[1].vertex;
				else VOL_ASSERT(false);


				incidentTriangleEdge[0] = clipTriangles[j][(index + 5) % 6];
				incidentTriangleEdge[1] = clipTriangles[j][(index + 4) % 6];
				np = b3_ClipSegmentToLine(clipPoints, incidentTriangleEdge, normals[i], offset[i], indexRef[0]);
				if (np == 1)
					clipTriangles[j][(index + 4) % 6].vertex = clipPoints[0].vertex;
				else if (np == 2)
					clipTriangles[j][(index + 4) % 6].vertex = clipPoints[1].vertex;
				else VOL_ASSERT(false);

			}
		}

		if (clipTriangles.size() == 0)
			return;

		/*
		//glm::vec3 incCenter = b3_Multiply(transformInc, boxInc->m_position);
		glm::vec3 incPoint = glm::vec3(0.0f);
		for (int i = 0; i < clipTriangles.size(); i++)
			for (int j = 0; j < 6; j++)
				incPoint += clipTriangles[i][j].vertex;
		incPoint /= (float)clipTriangles.size() * 6.0f;

		manifold->localTangent = glm::cross(localNormal, b3_MultiplyT(transformRef, incPoint) - boxRef->m_position);
		if (glm::abs(manifold->localTangent) < glm::vec3(b3_LinearSlop))
			manifold->localTangent = glm::vec3(0.0f);
		else
			manifold->localTangent = glm::normalize(manifold->localTangent);
			*/
		manifold->localNormal = localNormal;
		manifold->localPoint = planePoint;

		int pointCount = 0;
		for (int i = 0; i < clipTriangles.size(); i++)
		{
			for (int j = 0; j < 6; j++)
			{
				VOL_ASSERT(clipTriangles[i][j].vertex != glm::vec3(FLT_MAX));

				glm::vec3 point = b3_MultiplyT(transformInc, clipTriangles[i][j].vertex);

				b3_ManifoldPoint* cp = manifold->points + pointCount;
				cp->localPoint = point;
				cp->id = clipTriangles[i][j].id;
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
		}

		//glm::vec3 size = boxRef->m_size;
		//VOL_TRACE("boxRef.size: " + std::to_string(size.x) + ", " + std::to_string(size.y) + ", " + std::to_string(size.z));

		for (int i = 0; i < pointCount; i++)
		{
			glm::vec3 point = (manifold->points + i)->localPoint;
			VOL_TRACE("point" + std::to_string(i) + ": " + 
				std::to_string(point.x) + ", " +  std::to_string(point.y) + ", " + std::to_string(point.z));
		}

		if (pointCount > b3_MaxManifoldPoints)
			return;

		manifold->pointCount = pointCount;
	}

}