#include "volpch.h"

#include "b3_TimeOfImpact.h"
#include <Volcano/Core/Timer.h>

namespace Volcano {

	float b3_TOITime, b3_TOIMaxTime;
	int b3_TOICalls, b3_TOIIters, b3_TOIMaxIters;  // 调用TOI的次数
	int b3_TOIRootIters, b3_TOIMaxRootIters;

	//
	struct b3_SeparationFunction
	{
		enum Type
		{
			e_points,
			e_faceA,
			e_faceB
		};

		// TODO_ERIN might not need to return the separation

		float Initialize(const b3_SimplexCache* cache,
			const b3_DistanceProxy* proxyA, const b3_Sweep& sweepA,
			const b3_DistanceProxy* proxyB, const b3_Sweep& sweepB,
			float t1)
		{
			m_proxyA = proxyA;
			m_proxyB = proxyB;
			int count = cache->count;
			assert(0 < count && count < 3);

			m_sweepA = sweepA;
			m_sweepB = sweepB;

			b3_Transform transformA, transformB;
			m_sweepA.GetTransform(&transformA, t1);
			m_sweepB.GetTransform(&transformB, t1);

			if (count == 1)
			{
				m_type = e_points;
				glm::vec3 localPointA = m_proxyA->GetVertex(cache->indexA[0]);
				glm::vec3 localPointB = m_proxyB->GetVertex(cache->indexB[0]);
				glm::vec3 pointA = b3_Multiply(transformA, localPointA);
				glm::vec3 pointB = b3_Multiply(transformB, localPointB);
				m_axis = pointB - pointA;
				float s = glm::length(m_axis);
				m_axis = glm::normalize(m_axis);
				return s;
			}
			else if (cache->indexA[0] == cache->indexA[1])
			{
				// Two points on B and one on A.
				m_type = e_faceB;
				glm::vec3 localPointB1 = proxyB->GetVertex(cache->indexB[0]);
				glm::vec3 localPointb3_ = proxyB->GetVertex(cache->indexB[1]);

				m_axis = b3_Tangent(localPointb3_ - localPointB1);
				m_axis = glm::normalize(m_axis);
				glm::vec3 normal = b3_Multiply(transformB.rotation, m_axis);

				m_localPoint = 0.5f * (localPointB1 + localPointb3_);
				glm::vec3 pointB = b3_Multiply(transformB, m_localPoint);

				glm::vec3 localPointA = proxyA->GetVertex(cache->indexA[0]);
				glm::vec3 pointA = b3_Multiply(transformA, localPointA);

				float s = glm::dot(pointA - pointB, normal);
				if (s < 0.0f)
				{
					m_axis = -m_axis;
					s = -s;
				}
				return s;
			}
			else
			{
				// Two points on A and one or two points on B.
				m_type = e_faceA;
				glm::vec3 localPointA1 = m_proxyA->GetVertex(cache->indexA[0]);
				glm::vec3 localPointA2 = m_proxyA->GetVertex(cache->indexA[1]);

				m_axis = b3_Tangent(localPointA2 - localPointA1);
				m_axis = glm::normalize(m_axis);
				glm::vec3 normal = b3_Multiply(transformA.rotation, m_axis);

				m_localPoint = 0.5f * (localPointA1 + localPointA2);
				glm::vec3 pointA = b3_Multiply(transformA, m_localPoint);

				glm::vec3 localPointB = m_proxyB->GetVertex(cache->indexB[0]);
				glm::vec3 pointB = b3_Multiply(transformB, localPointB);

				float s = glm::dot(pointB - pointA, normal);
				if (s < 0.0f)
				{
					m_axis = -m_axis;
					s = -s;
				}
				return s;
			}
		}

		//
		float FindMinSeparation(int* indexA, int* indexB, float t) const
		{
			b3_Transform transformA, transformB;
			m_sweepA.GetTransform(&transformA, t);
			m_sweepB.GetTransform(&transformB, t);

			switch (m_type)
			{
			case e_points:
			{
				glm::vec3 axisA = b3_MultiplyT(transformA.rotation, m_axis);
				glm::vec3 axisB = b3_MultiplyT(transformB.rotation, -m_axis);

				*indexA = m_proxyA->GetSupport(axisA);
				*indexB = m_proxyB->GetSupport(axisB);

				glm::vec3 localPointA = m_proxyA->GetVertex(*indexA);
				glm::vec3 localPointB = m_proxyB->GetVertex(*indexB);

				glm::vec3 pointA = b3_Multiply(transformA, localPointA);
				glm::vec3 pointB = b3_Multiply(transformB, localPointB);

				float separation = glm::dot(pointB - pointA, m_axis);
				return separation;
			}

			case e_faceA:
			{
				glm::vec3 normal = b3_Multiply(transformA.rotation, m_axis);
				glm::vec3 pointA = b3_Multiply(transformA, m_localPoint);

				glm::vec3 axisB = b3_MultiplyT(transformB.rotation, -normal);

				*indexA = -1;
				*indexB = m_proxyB->GetSupport(axisB);

				glm::vec3 localPointB = m_proxyB->GetVertex(*indexB);
				glm::vec3 pointB = b3_Multiply(transformB, localPointB);

				float separation = glm::dot(pointB - pointA, normal);
				return separation;
			}

			case e_faceB:
			{
				glm::vec3 normal = b3_Multiply(transformB.rotation, m_axis);
				glm::vec3 pointB = b3_Multiply(transformB, m_localPoint);

				glm::vec3 axisA = b3_MultiplyT(transformA.rotation, -normal);

				*indexB = -1;
				*indexA = m_proxyA->GetSupport(axisA);

				glm::vec3 localPointA = m_proxyA->GetVertex(*indexA);
				glm::vec3 pointA = b3_Multiply(transformA, localPointA);

				float separation = glm::dot(pointA - pointB, normal);
				return separation;
			}

			default:
				assert(false);
				*indexA = -1;
				*indexB = -1;
				return 0.0f;
			}
		}

		//
		float Evaluate(int indexA, int indexB, float t) const
		{
			b3_Transform transformA, transformB;
			m_sweepA.GetTransform(&transformA, t);
			m_sweepB.GetTransform(&transformB, t);

			switch (m_type)
			{
			case e_points:
			{
				glm::vec3 localPointA = m_proxyA->GetVertex(indexA);
				glm::vec3 localPointB = m_proxyB->GetVertex(indexB);

				glm::vec3 pointA = b3_Multiply(transformA, localPointA);
				glm::vec3 pointB = b3_Multiply(transformB, localPointB);
				float separation = glm::dot(pointB - pointA, m_axis);

				return separation;
			}

			case e_faceA:
			{
				glm::vec3 normal = b3_Multiply(transformA.rotation, m_axis);
				glm::vec3 pointA = b3_Multiply(transformA, m_localPoint);

				glm::vec3 localPointB = m_proxyB->GetVertex(indexB);
				glm::vec3 pointB = b3_Multiply(transformB, localPointB);

				float separation = glm::dot(pointB - pointA, normal);
				return separation;
			}

			case e_faceB:
			{
				glm::vec3 normal = b3_Multiply(transformB.rotation, m_axis);
				glm::vec3 pointB = b3_Multiply(transformB, m_localPoint);

				glm::vec3 localPointA = m_proxyA->GetVertex(indexA);
				glm::vec3 pointA = b3_Multiply(transformA, localPointA);

				float separation = glm::dot(pointA - pointB, normal);
				return separation;
			}

			default:
				assert(false);
				return 0.0f;
			}
		}

		const b3_DistanceProxy* m_proxyA;
		const b3_DistanceProxy* m_proxyB;
		b3_Sweep m_sweepA, m_sweepB;
		Type m_type;
		glm::vec3 m_localPoint;
		glm::vec3 m_axis;
	};

	// CCD通过(via)局部分离轴法。这通过计算保持分离的最大时间来寻求进步(progression)。
	void b3_TimeOfImpact(b3_TOIOutput* output, const b3_TOIInput* input)
	{
		Timer timer;

		++b3_TOICalls;

		output->state = b3_TOIOutput::e_unknown;
		output->t = input->tMax;

		const b3_DistanceProxy* proxyA = &input->proxyA;
		const b3_DistanceProxy* proxyB = &input->proxyB;

		b3_Sweep sweepA = input->sweepA;
		b3_Sweep sweepB = input->sweepB;

		// 大的旋转会使根查找器(finder)失败，因此我们对sweep角度进行归一化。
		sweepA.Normalize();
		sweepB.Normalize();

		float tMax = input->tMax;

		float totalRadius = proxyA->m_radius + proxyB->m_radius;  // AB的半径和
		float target = glm::max(b3_LinearSlop, totalRadius - 3.0f * b3_LinearSlop);
		float tolerance = 0.25f * b3_LinearSlop; // 公差
		assert(target > tolerance);

		float t1 = 0.0f;
		const int k_maxIterations = 20;	// TODO_ERIN b3_Settings
		int iter = 0;

		// 准备距离查询的输入。
		b3_SimplexCache cache;
		cache.count = 0;
		b3_DistanceInput distanceInput;
		distanceInput.proxyA = input->proxyA;
		distanceInput.proxyB = input->proxyB;
		distanceInput.useRadii = false;

		// 外环逐步尝试计算新的分离轴。 当轴重复时（没有进展），此循环终止。
		for (;;)
		{
			b3_Transform transformA, transformB;
			sweepA.GetTransform(&transformA, t1);
			sweepB.GetTransform(&transformB, t1);

			// 获取shapes之间的距离。我们还可以使用结果来获得分离轴。
			distanceInput.transformA = transformA;
			distanceInput.transformB = transformB;
			b3_DistanceOutput distanceOutput;
			b3_Distance(&distanceOutput, &cache, &distanceInput);

			// 如果shapes重叠，我们就放弃连续碰撞。
			if (distanceOutput.distance <= 0.0f)
			{
				// Failure!
				output->state = b3_TOIOutput::e_overlapped;
				output->t = 0.0f;
				break;
			}

			if (distanceOutput.distance < target + tolerance)
			{
				// Victory!
				output->state = b3_TOIOutput::e_touching;
				output->t = t1;
				break;
			}

			// 初始化分离轴。 
			b3_SeparationFunction fcn;
			fcn.Initialize(&cache, proxyA, sweepA, proxyB, sweepB, t1);

			// 计算分离轴上的TOI。我们通过连续(successively)求解最深点来实现这一点。此循环受顶点数量的限制。
			bool done = false;
			float t2 = tMax;
			int pushBackIter = 0;
			for (;;)
			{
				// 在t2处找到最深点。存储见证(witness)点索引。
				int indexA, indexB;
				float s2 = fcn.FindMinSeparation(&indexA, &indexB, t2);

				// 最终配置(configuration)是否分开？
				if (s2 > target + tolerance)
				{
					// Victory!
					output->state = b3_TOIOutput::e_separated;
					output->t = tMax;
					done = true;
					break;
				}

				// 这种分离是否已经达到了公差？ Has the separation reached tolerance?
				if (s2 > target - tolerance)
				{
					// 推进(Advance)sweeps
					t1 = t2;
					break;
				}

				// Compute the initial separation of the witness points.
				float s1 = fcn.Evaluate(indexA, indexB, t1);

				// Check for initial overlap. This might happen if the root finder
				// runs out of iterations.
				if (s1 < target - tolerance)
				{
					output->state = b3_TOIOutput::e_failed;
					output->t = t1;
					done = true;
					break;
				}

				// Check for touching
				if (s1 <= target + tolerance)
				{
					// Victory! t1 should hold the TOI (could be 0.0).
					output->state = b3_TOIOutput::e_touching;
					output->t = t1;
					done = true;
					break;
				}

				// Compute 1D root of: f(x) - target = 0
				int rootIterCount = 0;
				float a1 = t1, a2 = t2;
				for (;;)
				{
					// Use a mix of the secant rule and bisection.
					float t;
					if (rootIterCount & 1)
					{
						// Secant rule to improve convergence.
						t = a1 + (target - s1) * (a2 - a1) / (s2 - s1);
					}
					else
					{
						// Bisection to guarantee progress.
						t = 0.5f * (a1 + a2);
					}

					++rootIterCount;
					++b3_TOIRootIters;

					float s = fcn.Evaluate(indexA, indexB, t);

					if (glm::abs(s - target) < tolerance)
					{
						// t2 holds a tentative value for t1
						t2 = t;
						break;
					}

					// Ensure we continue to bracket the root.
					if (s > target)
					{
						a1 = t;
						s1 = s;
					}
					else
					{
						a2 = t;
						s2 = s;
					}

					if (rootIterCount == 50)
					{
						break;
					}
				}

				b3_TOIMaxRootIters = glm::max(b3_TOIMaxRootIters, rootIterCount);

				++pushBackIter;

				if (pushBackIter == b3_MaxBoxVertices)
				{
					break;
				}
			}

			++iter;
			++b3_TOIIters;

			if (done)
			{
				break;
			}

			if (iter == k_maxIterations)
			{
				// Root finder got stuck. Semi-victory.
				output->state = b3_TOIOutput::e_failed;
				output->t = t1;
				break;
			}
		}

		b3_TOIMaxIters = glm::max(b3_TOIMaxIters, iter);

		float time = timer.ElapsedMillis();
		b3_TOIMaxTime = glm::max(b3_TOIMaxTime, time);
		b3_TOITime += time;
	}

}
