#include "volpch.h"

#include "b3_Collision.h"
//#include "b3_Distance.h"

namespace Volcano {
	
	bool b3_TestOverlap(const b3_Shape* shapeA, int indexA, const b3_Shape* shapeB, int indexB,
		const b3_Transform& transformA, const b3_Transform& transformB)
	{
		/*
		b3_DistanceInput input;
		input.proxyA.Set(shapeA, indexA);
		input.proxyB.Set(shapeB, indexB);
		input.transformA = transformA;
		input.transformB = transformB;
		input.useRadii = true;

		b3_SimplexCache cache;
		cache.count = 0;

		b3_DistanceOutput output;

		b3_Distance(&output, &cache, &input);

		return output.distance < 10.0f * b3_Epsilon;
		*/
		return false;
	}
	

	void b3_WorldManifold::Initialize(const b3_Manifold* manifold, const b3_Transform& transformA, float radiusA, const b3_Transform& transformB, float radiusB)
	{
		if (manifold->pointCount == 0)
		{
			return;
		}

		switch (manifold->type)
		{
		case b3_Manifold::e_circles:
		{
			normal = { 1.0f, 0.0f, 0.0f };
			glm::vec3 pointA = b3_Multiply(transformA, manifold->localPoint);
			glm::vec3 pointB = b3_Multiply(transformB, manifold->points[0].localPoint);
			if (glm::dot(pointB - pointA, pointB - pointA) > b3_Epsilon * b3_Epsilon)
			{
				normal = glm::normalize(pointB - pointA);
			}

			glm::vec3 cA = pointA + radiusA * normal;
			glm::vec3 cB = pointB - radiusB * normal;
			points[0] = 0.5f * (cA + cB);
			separations[0] = glm::dot(cB - cA, normal);
		}
		break;
		
		case b3_Manifold::e_faceA:
		{
			normal = b3_Multiply(transformA.rotation, manifold->localNormal);
			glm::vec3 planePoint = b3_Multiply(transformA, manifold->localPoint);  // Ωªµ„Œª÷√

			for (int i = 0; i < manifold->pointCount; ++i)
			{
				glm::vec3 clipPoint = b3_Multiply(transformB, manifold->points[i].localPoint);
				glm::vec3 cA = clipPoint + (radiusA - glm::dot(clipPoint - planePoint, normal)) * normal;
				glm::vec3 cB = clipPoint - radiusB * normal;
				points[i] = 0.5f * (cA + cB);
				separations[i] = glm::dot(cB - cA, normal);
			}
		}
		break;

		case b3_Manifold::e_faceB:
		{
			normal = b3_Multiply(transformB.rotation, manifold->localNormal);
			glm::vec3 planePoint = b3_Multiply(transformB, manifold->localPoint);

			for (int i = 0; i < manifold->pointCount; ++i)
			{
				glm::vec3 clipPoint = b3_Multiply(transformA, manifold->points[i].localPoint);
				glm::vec3 cB = clipPoint + (radiusB - glm::dot(clipPoint - planePoint, normal)) * normal;
				glm::vec3 cA = clipPoint - radiusA * normal;
				points[i] = 0.5f * (cA + cB);
				separations[i] = glm::dot(cA - cB, normal);
			}

			// Ensure normal points from A to B.
			normal = -normal;
		}
		break;
		
		}
	}

}