#include "volpch.h"

#include "b3_ContactSolver.h"
#include "b3_StackAllocator.h"
#include "b3_Fixture.h"
#include "b3_Contact.h"
#include "b3_Math.h"

namespace Volcano {

	bool g_blockSolve = true;

	// contact位置约束
	struct b3_ContactPositionConstraint
	{
		glm::vec3 localPoints[b3_MaxManifoldPoints];// 另一个被碰撞物体的位置
		glm::vec3 localNormal;  // e_circles：未使用，e_faceA：碰撞交点或边或面的法向量，单位向量
		glm::vec3 localPoint;	// e_circles：发生碰撞的body的中点, e_faceA：碰撞交点
		int indexA;             // bodyA的岛上索引
		int indexB;             // bodyB的岛上索引
		float invMassA, invMassB;
		glm::vec3 localCenterA, localCenterB; //质心局部坐标
		glm::mat3 invIA, invIB;
		b3_Manifold::Type type;
		float radiusA, radiusB;
		int pointCount;
	};

	b3_ContactSolver::b3_ContactSolver(b3_ContactSolverDef* def)
	{
		m_step = def->step;
		m_allocator = def->allocator;
		m_count = def->count;
		m_positionConstraints = (b3_ContactPositionConstraint*)m_allocator->Allocate(m_count * sizeof(b3_ContactPositionConstraint));
		m_velocityConstraints = (b3_ContactVelocityConstraint*)m_allocator->Allocate(m_count * sizeof(b3_ContactVelocityConstraint));
		m_positions = def->positions;
		m_velocities = def->velocities;
		m_contacts = def->contacts;

		// 初始化约束中与位置无关的部分(independent portions)。为每一个contact构建速度约束和位置约束
		// 遍历contact，初始化速度约束和位置约束
		for (int i = 0; i < m_count; ++i)
		{
			b3_Contact* contact = m_contacts[i];

			b3_Fixture* fixtureA = contact->m_fixtureA;
			b3_Fixture* fixtureB = contact->m_fixtureB;
			b3_Shape* shapeA = fixtureA->GetShape();
			b3_Shape* shapeB = fixtureB->GetShape();
			float radiusA = shapeA->m_radius;
			float radiusB = shapeB->m_radius;
			b3_Body* bodyA = fixtureA->GetBody();
			b3_Body* bodyB = fixtureB->GetBody();
			b3_Manifold* manifold = contact->GetManifold();

			int pointCount = manifold->pointCount;
			assert(pointCount > 0);

			b3_ContactVelocityConstraint* velocityConstraint = m_velocityConstraints + i;
			velocityConstraint->friction     = contact->m_friction;
			velocityConstraint->restitution  = contact->m_restitution;
			velocityConstraint->threshold    = contact->m_restitutionThreshold;
			velocityConstraint->tangentSpeed = contact->m_tangentSpeed;
			velocityConstraint->indexA       = bodyA->m_islandIndex;
			velocityConstraint->indexB       = bodyB->m_islandIndex;
			velocityConstraint->invMassA     = bodyA->m_invMass;
			velocityConstraint->invMassB     = bodyB->m_invMass;
			velocityConstraint->invIA        = bodyA->m_invI;
			velocityConstraint->invIB        = bodyB->m_invI;
			velocityConstraint->contactIndex = i;
			velocityConstraint->pointCount   = pointCount;

			b3_ContactPositionConstraint* positionConstraint = m_positionConstraints + i;
			positionConstraint->indexA       = bodyA->m_islandIndex;
			positionConstraint->indexB       = bodyB->m_islandIndex;
			positionConstraint->invMassA     = bodyA->m_invMass;
			positionConstraint->invMassB     = bodyB->m_invMass;
			positionConstraint->localCenterA = bodyA->m_sweep.localCenter;
			positionConstraint->localCenterB = bodyB->m_sweep.localCenter;
			positionConstraint->invIA        = bodyA->m_invI;
			positionConstraint->invIB        = bodyB->m_invI;
			positionConstraint->localNormal  = manifold->localNormal;
			positionConstraint->localPoint   = manifold->localPoint;
			positionConstraint->pointCount   = pointCount;
			positionConstraint->radiusA      = radiusA;
			positionConstraint->radiusB      = radiusB;
			positionConstraint->type         = manifold->type;

			// 遍历Manifold的点，将点对应的速度约束初始化
			for (int j = 0; j < pointCount; ++j)
			{
				b3_ManifoldPoint* mp = manifold->points + j;
				b3_VelocityConstraintPoint* velocityConstraintPoint = velocityConstraint->points + j;

				if (m_step.warmStarting)
				{
					velocityConstraintPoint->normalImpulse = m_step.deltaTimeRatio * mp->normalImpulse;
					velocityConstraintPoint->tangentImpulse = m_step.deltaTimeRatio * mp->tangentImpulse;
				}
				else
				{
					velocityConstraintPoint->normalImpulse = 0.0f;
					velocityConstraintPoint->tangentImpulse = 0.0f;
				}

				velocityConstraintPoint->rA = { 0.0f, 0.0f, 0.0f };
				velocityConstraintPoint->rB = { 0.0f, 0.0f, 0.0f };
				velocityConstraintPoint->normalMass = 0.0f;
				velocityConstraintPoint->tangentMass = 0.0f;
				velocityConstraintPoint->velocityBias = 0.0f;

				positionConstraint->localPoints[j] = mp->localPoint;
			}
		}
	}
	b3_ContactSolver::~b3_ContactSolver()
	{
		m_allocator->Free(m_velocityConstraints);
		m_allocator->Free(m_positionConstraints);
	}

	void b3_ContactSolver::InitializeVelocityConstraints()
	{
		for (int contactIndex = 0; contactIndex < m_count; ++contactIndex)
		{
			b3_ContactVelocityConstraint* velocityConstraint = m_velocityConstraints + contactIndex;
			b3_ContactPositionConstraint* positionConstraint = m_positionConstraints + contactIndex;

			b3_Manifold* manifold = m_contacts[velocityConstraint->contactIndex]->GetManifold();

			int indexA = velocityConstraint->indexA;
			int indexB = velocityConstraint->indexB;

			float imA = velocityConstraint->invMassA;
			float imB = velocityConstraint->invMassB;
			glm::mat3 iIA = velocityConstraint->invIA;
			glm::mat3 iIB = velocityConstraint->invIB;
			glm::vec3 localCenterA = positionConstraint->localCenterA;  // 质心A局部坐标
			glm::vec3 localCenterB = positionConstraint->localCenterB;  // 质心B局部坐标

			glm::vec3 cA = m_positions[indexA].position;	     // bodyA质心世界坐标
			glm::vec3 aA = m_positions[indexA].rotation;         // bodyA的rotation
			glm::vec3 vA = m_velocities[indexA].linearVelocity;  // 线速度A
			glm::vec3 wA = m_velocities[indexA].angularVelocity; // 角速度A

			glm::vec3 cB = m_positions[indexB].position;         // bodyB质心世界坐标
			glm::vec3 aB = m_positions[indexB].rotation;         // bodyB的rotation
			glm::vec3 vB = m_velocities[indexB].linearVelocity;  // 线速度B
			glm::vec3 wB = m_velocities[indexB].angularVelocity; // 角速度B

			assert(manifold->pointCount > 0);
			
			float radiusA = positionConstraint->radiusA;
			float radiusB = positionConstraint->radiusB;
			b3_Transform transformA, transformB;
			transformA.rotation.Set(aA);
			transformB.rotation.Set(aB);
			transformA.position = cA - b3_Multiply(transformA.rotation, localCenterA);
			transformB.position = cB - b3_Multiply(transformB.rotation, localCenterB);

			b3_WorldManifold worldManifold;
			worldManifold.Initialize(manifold, transformA, radiusA, transformB, radiusB);

			velocityConstraint->normal = worldManifold.normal;

			// 遍历worldManifold所有交点，初始化速度约束
			int pointCount = velocityConstraint->pointCount;
			for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex)
			{
				b3_VelocityConstraintPoint* velocityConstraintPoint = velocityConstraint->points + pointIndex;

				velocityConstraintPoint->rA = worldManifold.points[pointIndex] - cA;
				velocityConstraintPoint->rB = worldManifold.points[pointIndex] - cB;

				glm::vec3 rA_cross_n = glm::cross(velocityConstraintPoint->rA, velocityConstraint->normal);
				glm::vec3 rB_cross_n = glm::cross(velocityConstraintPoint->rB, velocityConstraint->normal);

				// 约束 C(p(t))    p(t)：位置position随时间变化，整体速度  dp/dt = v + w×r, rb:矢径，锚点到接触点的距离
				// dC/dt = J * v(t) = 0 (J为雅可比矩阵，每一行为约束对质点影响，每一列为质点所受所有约束影响)
				// v = vi + dt(Fe+Fc)/M     Fe：外力，Fc：约束力
				// 定义约束力向量λf = [va, wa, vb, wb];
				// Fc = J^Tλf
				// v = vi + dt Fc/M = vi + J^T(dtλf) / M = vi + J^Tλimp / M
				// Jv = 0  =>  J(vi + J^Tλimp / M) = 0  => J*J^Tλimp / M = -Jvi
				// 动量p=mv，角动量L=Iw(转动惯量I,角速度w)
				// M^-1 = [ma^-1  0     0      0    ]
				//        [0     Ia^-1  0      0    ]
				//        [0     0      mb^-1  0    ]
				//        [0     0      0      Ib^-1]
				// C = length(pa - pb) - L
				// u = pa - pb; uu = u·u; n = normalize(u)
				// C = sqrt(uu) - L
				// dC/dt = 1/(2sqrt(uu)) * d(uu) / dt - 0 = 1/(2sqrt(uu)) * (u*du/dt+du/dt*u) = ndu/dt
				// du/dt = d(pa-pb) = dpa/dt -dpB/dt = va+wa×ra - vb - wb×rb
				// w×r =  |i  j  k  | = (wyrz-wzry, wzrx-wxrz, wxry-wyrx) =  | 0   rz -ry|
				// 		   |wx wy wz |										  |-rz  0   rx|
				// 		   |rx ry rz |										  | ry -rx  0 |
				// 
				// dC/dt = n(va+Rsawa-vb-Rsbwb) = n[I3x3  Rsa -I3x3 -Rsb][va wa vb wb]^T  I3x3是3x3的单位矩阵
				// 
				// J = dC/dt / v = [n^T n^TRsa -n^T -n^TRsb]   // 雅可比矩阵
				// 
				// JM^-1J^T = [n^T n^TRsa -n^T -n^TRsb][ma^-1  0     0      0    ][ n     ]
				//									   [0     Ia^-1  0      0    ][ nRsa^T]
				//									   [0     0      mb^-1  0    ][-n     ]
				//									   [0     0      0      Ib^-1][-nRsb^T]
				//          = n^T * ma^-1 * n + n^TRsa * Ia^-1 * nRsa^T + -n^T * mb^-1 * (-n) + -n^TRsb * Ib^-1 * -nRsb^T
				//          = ma^-1 + Ia^-1 * n^T * Rsa * nRsa^T * n + mb^-1 + Ib^-1 * n^T * Rsb * nRsb^T * n 
				// n^T*Rs*Rs^T*n = (nxrz-nxry + -nyrz+nyrx + nzry - nzrx)(-nyrz + nzry + nxrz - nzrx -nxry + nyrx)
				//               = (n×r)^2
				// JM^-1J^T = ma^-1 + Ia^-1(n×ra)^2 + mb^-1 + Ib^-1(n×rb)^2

				//有效质量 M effective
				float kNormal = imA + glm::dot(iIA * rA_cross_n * rA_cross_n) + imB + glm::dot(iIB * rB_cross_n * rB_cross_n);
				velocityConstraintPoint->normalMass = kNormal > 0.0f ? 1.0f / kNormal : 0.0f;

				glm::vec3 tangent = b3_Tangent(velocityConstraint->normal);


				glm::vec3 rA_cross_t = glm::cross(velocityConstraintPoint->rA, tangent);
				glm::vec3 rB_cross_t = glm::cross(velocityConstraintPoint->rB, tangent);

				float kTangent = imA + glm::dot(iIA * rA_cross_t * rA_cross_t) + imB + glm::dot(iIB * rB_cross_t * rB_cross_t);

				velocityConstraintPoint->tangentMass = kTangent > 0.0f ? 1.0f / kTangent : 0.0f;

				// 质量为m1,m2的物体碰撞前后速度与恢复系数(弹性)的关系:
				// v1’ = (m1v1 + m2v2 - em2(v2 - v1)) / (m1 + m2)
				// v2’ = (m1v1 + m2v2 - em1(v1 - v2)) / (m1 + m2)
				// 
				// 设置恢复速度偏差(velocity bias for restitution)。 
				velocityConstraintPoint->velocityBias = 0.0f;
				float vRel = glm::dot(velocityConstraint->normal, vB + glm::cross(wB, velocityConstraintPoint->rB) - vA - glm::cross(wA, velocityConstraintPoint->rA));  // v2-v1
				if (vRel < -velocityConstraint->threshold)
				{
					velocityConstraintPoint->velocityBias = -velocityConstraint->restitution * vRel;  
				}
			}

			// 如果大于2个点，准备块结算器 If we have two points, then prepare the block solver.
			if (velocityConstraint->pointCount > 1 && velocityConstraint->pointCount % 6 == 0 && g_blockSolve)
			{
				VOL_ASSERT(velocityConstraint->pointCount <= b3_MaxManifoldPoints);

				for (int triangleIndex = 0; triangleIndex < velocityConstraint->pointCount / 6; triangleIndex++)
				{

					std::array<b3_VelocityConstraintPoint*, 6> vcp;
					for (int i = 0; i < 6; i++)
						vcp[i] = velocityConstraint->points + triangleIndex * 6 + i;

					std::array<std::pair<glm::vec3, glm::vec3>, 6> r_cross_n;
					for (int i = 0; i < 6; i++)
					{
						r_cross_n[i].first  = glm::cross(vcp[i]->rA, velocityConstraint->normal);
						r_cross_n[i].second = glm::cross(vcp[i]->rB, velocityConstraint->normal);
					}

					glm::vec3& n = velocityConstraint->normal;
					float k[6][6];
					for (int i = 0; i < 6; i++)
					{
						for (int j = 0; j < i; j++)
						{
							k[i][j] = k[j][i];
						}
						for (int j = i; j < 6; j++)
						{
							k[i][j] = imA + glm::dot(iIA * r_cross_n[i].first * r_cross_n[j].first) + imB + glm::dot(iIB * r_cross_n[i].second * r_cross_n[j].second);
//								imA + glm::dot(n, iIA * glm::cross(r_cross_n[i].first, vcp[j]->rA)) +
//								imB + glm::dot(n, iIB * glm::cross(r_cross_n[i].second, vcp[j]->rB));
						}
					}

					float delta = 0.0f;
					for (int i = 0; i < 6; i++)
					{
						delta = delta + 
							k[0][i] * k[1][(i + 1) % 6] * k[2][(i + 2) % 6] * k[3][(i + 3) % 6] * k[4][(i + 4) % 6] * k[5][(i + 5) % 6] -
							k[0][i] * k[1][(i + 5) % 6] * k[2][(i + 4) % 6] * k[3][(i + 3) % 6] * k[4][(i + 2) % 6] * k[5][(i + 1) % 6];
					}

					if (delta != 0)
					{
						// K可以安全地反转(invert)
						mat K;
						K.resize(6);
						for (int i = 0; i < 6; i++)
							K[i].resize(6);
						for (int i = 0; i < 6; i++)
							for (int j = 0; j < 6; j++)
							{
								velocityConstraint->K[triangleIndex][i][j] = k[i][j];
								K[i][j] = k[i][j];
							}
						mat normalMass = inverse(K);

						for (int i = 0; i < 6; i++)
							for (int j = 0; j < 6; j++)
							{
								velocityConstraint->normalMass[triangleIndex][i][j] = normalMass[i][j];
								if (!std::isfinite(normalMass[i][j]))
								{
									VOL_TRACE("DEBUG");
								}
							}
					}
				}
			}
			else
			{
				// 约束是多余的(redundant)，只需使用一个。 TODO_ERIN 使用最深(deepest)？
				velocityConstraint->pointCount = 1;
			}
		}
	}
	
	void b3_ContactSolver::WarmStart()
	{
		// Warm start.
		for (int i = 0; i < m_count; ++i)
		{
			b3_ContactVelocityConstraint* velocityConstraint = m_velocityConstraints + i;

			int indexA     = velocityConstraint->indexA;
			int indexB     = velocityConstraint->indexB;
			float imA      = velocityConstraint->invMassA;
			glm::mat3 iIA  = velocityConstraint->invIA;
			float imB      = velocityConstraint->invMassB;
			glm::mat3 iIB  = velocityConstraint->invIB;
			int pointCount = velocityConstraint->pointCount;

			glm::vec3 vA = m_velocities[indexA].linearVelocity;
			glm::vec3 wA = m_velocities[indexA].angularVelocity;
			glm::vec3 vB = m_velocities[indexB].linearVelocity;
			glm::vec3 wB = m_velocities[indexB].angularVelocity;

			glm::vec3 normal  = velocityConstraint->normal;
			glm::vec3 tangent = b3_Tangent(normal);


			for (int j = 0; j < pointCount; ++j)
			{
				b3_VelocityConstraintPoint* vcp = velocityConstraint->points + j;
				glm::vec3 P = vcp->normalImpulse * normal + vcp->tangentImpulse * tangent;
				// L = Iw
				wA -= iIA * glm::cross(vcp->rA, P);
				vA -= imA * P;
				wB += iIB * glm::cross(vcp->rB, P);
				vB += imB * P;
			}

			m_velocities[indexA].linearVelocity  = vA;
			m_velocities[indexA].angularVelocity = wA;
			m_velocities[indexB].linearVelocity  = vB;
			m_velocities[indexB].angularVelocity = wB;
		}
	}

	void b3_ContactSolver::SolveVelocityConstraints()
	{
		for (int vcIndex = 0; vcIndex < m_count; ++vcIndex)
		{
			b3_ContactVelocityConstraint* velocityConstraint = m_velocityConstraints + vcIndex;

			int indexA     = velocityConstraint->indexA;
			int indexB     = velocityConstraint->indexB;
			float imA      = velocityConstraint->invMassA;
			glm::mat3 iIA  = velocityConstraint->invIA;
			float imB      = velocityConstraint->invMassB;
			glm::mat3 iIB  = velocityConstraint->invIB;
			int pointCount = velocityConstraint->pointCount;

			glm::vec3 vA = m_velocities[indexA].linearVelocity;  // linearVelocity
			glm::vec3 wA = m_velocities[indexA].angularVelocity; // angularVelocity
			glm::vec3 vB = m_velocities[indexB].linearVelocity;  // linearVelocity
			glm::vec3 wB = m_velocities[indexB].angularVelocity; // angularVelocity

			glm::vec3 normal = velocityConstraint->normal;      // 交点法向量
			glm::vec3 tangent = b3_Tangent(normal);
			float friction = velocityConstraint->friction;   // 摩擦力

			//assert(pointCount == 1 || pointCount == 2);

			// 首先解决切线约束，因为非穿透(non-penetration)比摩擦更重要。
			for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex)
			{
				b3_VelocityConstraintPoint* velocityConstraintPoint = velocityConstraint->points + pointIndex;

				// VelocityConstraint, contact物体间的相对速度
				glm::vec3 dv = vB + glm::cross(wB, velocityConstraintPoint->rB) - vA - glm::cross(wA, velocityConstraintPoint->rA);

				float vt = glm::dot(dv, tangent) - velocityConstraint->tangentSpeed; // 相对速度在切线方向的速度
				float lambda = velocityConstraintPoint->tangentMass * (-vt);  // 切线方向的冲量

				// Sequential Impulse 

				// b3_Clamp累加冲量
				float maxFriction = friction * velocityConstraintPoint->normalImpulse; // 上一帧的法线冲量计算摩擦力
				float newImpulse = b3_Clamp(velocityConstraintPoint->tangentImpulse + lambda, -maxFriction, maxFriction);
				lambda = newImpulse - velocityConstraintPoint->tangentImpulse;
				velocityConstraintPoint->tangentImpulse = newImpulse;

				// 结算contact冲量
				glm::vec3 P = lambda * tangent;

				vA -= imA * P;
				wA -= iIA * glm::cross(velocityConstraintPoint->rA, P);

				vB += imB * P;
				wB += iIB * glm::cross(velocityConstraintPoint->rB, P);
			}
			// 结算法线约束
			if (pointCount == 1 || g_blockSolve == false)
			{
				for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex)
				{
					b3_VelocityConstraintPoint* velocityConstraintPoint = velocityConstraint->points + pointIndex;

					// 相对速度
					glm::vec3 dv = vB + glm::cross(wB, velocityConstraintPoint->rB) - vA - glm::cross(wA, velocityConstraintPoint->rA);

					// 相对速度在法向量的投影
					float vn = glm::dot(dv, normal);
					float lambda = velocityConstraintPoint->normalMass * -(vn - velocityConstraintPoint->velocityBias); // 法线方向的冲量

					// Sequential Impulse 

					// 获得上一步的累加冲量(accumulated impulse)，并将累加结果限制在大于 0 的范围，防止施加反冲
					float newImpulse = glm::max(velocityConstraintPoint->normalImpulse + lambda, 0.0f);
					lambda = newImpulse - velocityConstraintPoint->normalImpulse;
					velocityConstraintPoint->normalImpulse = newImpulse;

					// 结算contact冲量
					glm::vec3 P = lambda * normal;
					vA -= imA * P;
					wA -= iIA * glm::cross(velocityConstraintPoint->rA, P);

					vB += imB * P;
					wB += iIB * glm::cross(velocityConstraintPoint->rB, P);
				}
			}
			else
			{
				// Block solver
				// 
				// 由上面的注释中的计算得到公式
				// J = dC/dt / v = [n^T n^TRsa -n^T -n^TRsb]
				// Jn M^(-1) Jn^Tλn = -JnV
				// J M^(-1) J^T = ma^-1 + Ia^-1(n×ra)^2 + mb^-1 + Ib^-1(n×rb)^2
				// 
				// M(v'-v)=Jn1λn1+Jn2λn2   <=   冲量作用到物体导致速度变化
				// 
				//   => v' = v + M^(-1) Jn1λn1 +  M^(-1) Jn2λn2
				// 
				//   => Jn1V' = Jn1V + Jn1M^(-1)Jn1^Tλn1 + Jn1M^(-1)Jn2^Tλn2
				//      Jn2V' = Jn2V + Jn1M^(-1)Jn1^Tλn1 + Jn2M^(-1)Jn2^Tλn2
				// 
				// 	 => [Jn1M^(-1)Jn1^T  Jn1M^(-1)Jn2^T][λn1]+[Jn1V]=[Jn1V']
				//      [Jn2M^(-1)Jn1^T  Jn2M^(-1)Jn2^T][λn2]+[Jn2V]=[Jn2V']
				//                  A                     x       b      y
				//  
				// 如果求得了一组接触冲量的大小λn1,λn2 ，并按碰撞法向施加给给刚体后，我们希望最终的速度大于等于0 。
				// 
				//   => A x + b = y
				// 
				// 接触冲量的大小为非负数：x>=0
				// 最终的相对速度沿碰撞法向必须是相互分离的：y >= 0
				// 如果两个刚体要相互穿透，那就产生接触冲量，使两者分离：x >= 0 => y = 0 \
				// 如果两个刚体已经不往穿透方向运动，那就不产生接触冲量：y >= 0 => x = 0 /   x^T * y = 0
				// 
				// 构建LCP(Linear Complementarity Problem, 线性互补问题)
				//
				// 如果矩阵A是正定的，可以把LCP转化为QP问题
				// 转置性质：(A * B)^T = B^T * A^T => 矩阵A为实对称矩阵
				// 
				// 对于任意x，有y=J^T*x,其中[Jn1^T, Jn2^T]^T,故x^TJM^(-1)J^Tx = y^TM^(-1)y
				// 质量矩阵的逆M^(-1)为对角矩阵且均为非负数(地面的m正无穷，逆为0),所以y^TM^(-1)y >= 0, JM^(-1)J^T为半正定矩阵
				// 如果严格要求JM^(-1)J^T为正定，地面的m可设为很小的数。
				// 
				// 
				// 
				// vn = A * x + b,    vn >= 0, x >= 0, vn_i * x_i = 0 with i = 1..2
				//
				// A = J * W * JT and J = ( -n, -r1 x n, n, r2 x n )  
				// b = vn0 - velocityBias
				//
				// 该系统使用“总枚举法(Total enumeration method)”求解。
				// 互补(complementary)约束vn_i*x_i意味着我们在任何解中都必须有vn_i=0或x_i=0。
				// 因此，对于2D碰撞问题，需要测试情况vn1=0和vn2=0，x1=0和x2=0，x1=0和vn2=0，x2=0和vn1=0。选择满足问题的第一个有效解。
				// 
				// 为了记录累积冲量“a”（由于求解器的迭代性质，它只需要对累积冲量进行箝位(clamped)，而不是增加冲量），我们改变了冲量变量（x_i）。
				// 
				// Substitute:
				// 
				// x = a + d
				// 
				// a := old total impulse  上一次迭代的法向冲量
				// x := new total impulse  这次迭代的法向冲量
				// d := incremental impulse   法向冲量增量
				//
				// 对于当前迭代，我们扩展了增加冲量的公式来计算新的总冲量：
				//
				// vn = A * d + b
				//    = A * (x - a) + b
				//    = A * x + b - A * a
				//    = A * x + b'
				// b' = b - A * a;


				std::vector<glm::vec3> vATemp; vATemp.resize(pointCount / 6); for (int i = 0; i < vATemp.size(); i++) vATemp[i] = glm::vec3(0.0f);
				std::vector<glm::vec3> wATemp; wATemp.resize(pointCount / 6); for (int i = 0; i < wATemp.size(); i++) wATemp[i] = glm::vec3(0.0f);
				std::vector<glm::vec3> vBTemp; vBTemp.resize(pointCount / 6); for (int i = 0; i < vBTemp.size(); i++) vBTemp[i] = glm::vec3(0.0f);
				std::vector<glm::vec3> wBTemp; wBTemp.resize(pointCount / 6); for (int i = 0; i < wBTemp.size(); i++) wBTemp[i] = glm::vec3(0.0f);

				if (pointCount % 6 != 0)
					break;

				for (int triangleIndex = 0; triangleIndex < pointCount / 6; triangleIndex++)
				{
					b3_VelocityConstraintPoint* constraintPoints[6];
					float a[6];       // 法向冲量normalImpulse (inverse A)， 上一轮结算的冲量
					glm::vec3 dv[6];  // 相对速度 vB+wB×rB-vA-wA×rA
					float vn[6];      // 法向速度 n(vB+wB×rB-vA-wA×rA)   Jnv  

					for (int i = 0; i < 6; i++)
					{
						constraintPoints[i] = velocityConstraint->points + triangleIndex * 6 + i;

						a[i] = constraintPoints[i]->normalImpulse;
						assert(a[i] >= 0.0f);

						dv[i] = vB + glm::cross(wB, constraintPoints[i]->rB) - vA - glm::cross(wA, constraintPoints[i]->rA);

						vn[i] = glm::dot(dv[i], normal);
					}

					float b[6];  // b = Jnv = n(vB+wB×rB-vA-wA×rA)
					for (int i = 0; i < 6; i++)
					{
						b[i] = vn[i] - constraintPoints[i]->velocityBias;
						float sum = 0;
						for (int j = 0; j < 6; j++)
							sum += velocityConstraint->K[triangleIndex][i][j] * a[j];
						b[i] -= sum;  // b' = b - A * a
					}

					const float k_errorTol = 1e-3f;
					(void)(k_errorTol);  // not used

					int flag = -1;
					std::array<float, 6> x;

					while (true)
					{
						// 14种情况：
						// 1、全向内 
						// 2、全向外
						// 3、p12 36向内，p45向外 
						// 4、p12 36向外，p45向内 
						// 5、p34 52向内，p16向外 
						// 6、p34 53向外，p16向内 
						// 7、p56 14向内，p23向外 
						// 8、p56 14向外，p23向内 
						// 9、p12 向内，p3456向外 
						// 10、p12 向外，p3456向内 
						// 11、p34 向内，p1256向外 
						// 12、p34 向外，p1256向内 
						// 13、p56 向内，p1234向外 
						// 14、p56 向外，p1234向内 


						{
							//
							// case 1: vn = 0
							//
							// 0 = A * x + b'
							//
							// 冲量x: λn
							// x = - inv(A) * b'    
							// x = - inv(A) *(n(vB+wB×rB-vA-wA×rA) - A * λ)

							for (int i = 0; i < 6; i++)
							{
								float sum = 0;
								for (int j = 0; j < 6; j++)
									sum += velocityConstraint->normalMass[triangleIndex][i][j] * b[j];
								x[i] = -sum;
							}

							if (x[0] >= 0.0f && x[1] >= 0.0f && x[2] >= 0.0f && x[3] >= 0.0f && x[4] >= 0.0f && x[5] >= 0.0f)
							{
								flag = 1;
								break;
							}
						}

						{
							// case 2: x = 0
							//
							// vn1 = b1'
							// vn2 = b2'
							// vn3 = b3'
							// vn4 = b4'
							// vn5 = b5'
							// vn6 = b6'

							for (int i = 0; i < 6; i++)
							{
								x[i] = 0.0f;
								vn[i] = b[i];
							}
							if (vn[0] >= 0.0f && vn[1] >= 0.0f && vn[2] >= 0.0f && vn[3] >= 0.0f && vn[4] >= 0.0f && vn[5] >= 0.0f)
							{
								flag = 2;
								break;
							}
						}


						{
							//
							// case 3: vn1 = vn2 = vn3 = λn4 = λn5 = vn6 = 0
							// x= {λn1, λn2, λn3, 0, 0, λn6 };
							//
							//   0 = a11 * x1 + a12 * x2 + a13 * x3 + a14 * 0 + a15 * 0 + a16 * x6 + b1' 
							//   0 = a12 * x1 + a22 * x2 + a23 * x3 + a24 * 0 + a25 * 0 + a26 * x6 + b2' 
							// vn3 = a13 * x1 + a23 * x2 + a33 * x3 + a34 * 0 + a35 * 0 + a36 * x6 + b3' 
							// vn4 = a14 * x1 + a24 * x2 + a34 * x3 + a44 * 0 + a45 * 0 + a46 * x6 + b4' 
							// vn5 = a15 * x1 + a25 * x2 + a35 * x3 + a45 * 0 + a55 * 0 + a56 * x6 + b5' 
							// vn6 = a16 * x1 + a26 * x2 + a36 * x3 + a46 * 0 + a55 * 0 + a66 * x6 + b6' 
							//

							x[0] = -constraintPoints[0]->normalMass * b[0];
							x[1] = -constraintPoints[1]->normalMass * b[1];
							x[2] = -constraintPoints[2]->normalMass * b[2];
							x[3] = 0.0f;
							x[4] = 0.0f;
							x[5] = -constraintPoints[5]->normalMass * b[5];


							vn[0] = 0.0f;
							vn[1] = 0.0f;
							vn[2] = 0.0f;
							vn[3] = velocityConstraint->K[triangleIndex][3][0] * x[0] + velocityConstraint->K[triangleIndex][3][1] * x[1] + velocityConstraint->K[triangleIndex][3][2] * x[2] + velocityConstraint->K[triangleIndex][3][5] * x[5] + b[3];
							vn[4] = velocityConstraint->K[triangleIndex][4][0] * x[0] + velocityConstraint->K[triangleIndex][4][1] * x[1] + velocityConstraint->K[triangleIndex][4][2] * x[2] + velocityConstraint->K[triangleIndex][4][5] * x[5] + b[4];
							vn[5] = 0.0f;
							if (x[0] >= 0.0f && x[1] >= 0.0f && x[2] >= 0.0f && vn[3] >= 0.0f && vn[4] >= 0.0f && x[5] >= 0.0f)
							{
								flag = 3;
								break;
							}
						}

						{
							//
							// case 4: λn1 = λn2 = λn3 = vn4 = vn5 = λn6 = 0
							// x= {0, 0, 0, λn4, λn5, 0 };
							//
							// vn1 = a11 * 0 + a12 * 0 + a13 * 0 + a14 * x4 + a15 * x5 + a16 * 0 + b1' 
							// vn2 = a12 * 0 + a22 * 0 + a23 * 0 + a24 * x4 + a25 * x5 + a26 * 0 + b2' 
							// vn3 = a13 * 0 + a23 * 0 + a33 * 0 + a34 * x4 + a35 * x5 + a36 * 0 + b3' 
							//   0 = a14 * 0 + a24 * 0 + a34 * 0 + a44 * x4 + a45 * x5 + a46 * 0 + b4' 
							//   0 = a15 * 0 + a25 * 0 + a35 * 0 + a45 * x4 + a55 * x5 + a56 * 0 + b5' 
							// vn6 = a16 * 0 + a26 * 0 + a36 * 0 + a46 * x4 + a55 * x5 + a66 * 0 + b6' 
							//

							x[0] = 0.0f;
							x[1] = 0.0f;
							x[2] = 0.0f;
							x[3] = -constraintPoints[3]->normalMass * b[3];
							x[4] = -constraintPoints[4]->normalMass * b[4];
							x[5] = 0.0f;

							vn[0] = velocityConstraint->K[triangleIndex][0][3] * x[3] + velocityConstraint->K[triangleIndex][0][4] * x[4] + b[0];
							vn[1] = velocityConstraint->K[triangleIndex][1][3] * x[3] + velocityConstraint->K[triangleIndex][1][4] * x[4] + b[1];
							vn[2] = velocityConstraint->K[triangleIndex][2][3] * x[3] + velocityConstraint->K[triangleIndex][2][4] * x[4] + b[2];
							vn[3] = 0.0f;
							vn[4] = 0.0f;
							vn[5] = velocityConstraint->K[triangleIndex][5][3] * x[3] + velocityConstraint->K[triangleIndex][5][4] * x[4] + b[5];
							if (vn[0] >= 0.0f && vn[1] >= 0.0f && vn[2] >= 0.0f && x[3] >= 0.0f && x[4] >= 0.0f && vn[5] >= 0.0f)
							{
								flag = 4;
								break;
							}
						}

						{
							//
							// case 5: λn1 = vn2 = vn3 = vn4  = vn5 = λn6= 0
							// x= {0, λn2, λn3, λn4, λn5, 0 };
							// 
							//
							// vn1 = a11 * 0 + a12 * x2 + a13 * x3 + a14 * x4 + a15 * x5 + a16 * 0 + b1' 
							//   0 = a12 * 0 + a22 * x2 + a23 * x3 + a24 * x4 + a25 * x5 + a26 * 0 + b2' 
							//   0 = a13 * 0 + a23 * x2 + a33 * x3 + a34 * x4 + a35 * x5 + a36 * 0 + b3' 
							//   0 = a14 * 0 + a24 * x2 + a34 * x3 + a44 * x4 + a45 * x5 + a46 * 0 + b4' 
							//   0 = a15 * 0 + a25 * x2 + a35 * x3 + a45 * x4 + a55 * x5 + a56 * 0 + b5' 
							// vn6 = a16 * 0 + a26 * x2 + a36 * x3 + a46 * x4 + a55 * x5 + a66 * 0 + b6' 
							//

							x[0] = 0.0f;
							x[1] = -constraintPoints[1]->normalMass * b[1];
							x[2] = -constraintPoints[2]->normalMass * b[2];
							x[3] = -constraintPoints[3]->normalMass * b[3];
							x[4] = -constraintPoints[4]->normalMass * b[4];
							x[5] = 0.0f;

							vn[0] = velocityConstraint->K[triangleIndex][0][1] * x[1] + velocityConstraint->K[triangleIndex][0][2] * x[2] + velocityConstraint->K[triangleIndex][0][3] * x[3] + velocityConstraint->K[triangleIndex][0][4] * x[4] + b[0];
							vn[1] = 0.0f;
							vn[2] = 0.0f;
							vn[3] = 0.0f;
							vn[4] = 0.0f;
							vn[5] = velocityConstraint->K[triangleIndex][5][1] * x[1] + velocityConstraint->K[triangleIndex][5][2] * x[2] + velocityConstraint->K[triangleIndex][5][3] * x[3] + velocityConstraint->K[triangleIndex][5][4] * x[4] + b[5];
							if (vn[0] >= 0.0f && x[1] >= 0.0f && x[2] >= 0.0f && x[3] >= 0.0f && x[4] >= 0.0f && vn[5] >= 0.0f)
							{
								flag = 5;
								break;
							}
						}

						{
							//
							// case 6: vn1 = λn2 = λn3 = λn4 = λn5 = vn6 = 0
							// x= {λn1, 0, 0, 0, 0, λn6 };
							// 
							//   0 = a11 * x1 + a12 * 0 + a13 * 0 + a14 * 0 + a15 * 0 + a16 * x6 + b1' 
							// vn2 = a12 * x1 + a22 * 0 + a23 * 0 + a24 * 0 + a25 * 0 + a26 * x6 + b2' 
							// vn3 = a13 * x1 + a23 * 0 + a33 * 0 + a34 * 0 + a35 * 0 + a36 * x6 + b3' 
							// vn4 = a14 * x1 + a24 * 0 + a34 * 0 + a44 * 0 + a45 * 0 + a46 * x6 + b4' 
							// vn5 = a15 * x1 + a25 * 0 + a35 * 0 + a45 * 0 + a55 * 0 + a56 * x6 + b5' 
							//   0 = a16 * x1 + a26 * 0 + a36 * 0 + a46 * 0 + a55 * 0 + a66 * x6 + b6' 
							//

							x[0] = -constraintPoints[0]->normalMass * b[0];
							x[1] = 0.0f;
							x[2] = 0.0f;
							x[3] = 0.0f;
							x[4] = 0.0f;
							x[5] = -constraintPoints[5]->normalMass * b[5];

							vn[0] = 0.0f;
							vn[1] = velocityConstraint->K[triangleIndex][1][0] * x[0] + velocityConstraint->K[triangleIndex][1][5] * x[5] + b[1];
							vn[2] = velocityConstraint->K[triangleIndex][2][0] * x[0] + velocityConstraint->K[triangleIndex][2][5] * x[5] + b[2];
							vn[3] = velocityConstraint->K[triangleIndex][3][0] * x[0] + velocityConstraint->K[triangleIndex][3][5] * x[5] + b[3];
							vn[4] = velocityConstraint->K[triangleIndex][4][0] * x[0] + velocityConstraint->K[triangleIndex][4][5] * x[5] + b[4];
							vn[5] = 0.0f;
							if (x[0] >= 0.0f && vn[1] >= 0.0f && vn[2] >= 0.0f && vn[3] >= 0.0f && vn[4] >= 0.0f && x[5] >= 0.0f)
							{
								flag = 6;
								break;
							}
						}

						{
							// case 7: vn1 = λn2 = λn3 = vn4 = vn5 = vn6 = 0
							// x= {λn1, 0, 0, λn4, λn5, λn6 };
							// 
							//   0 = a11 * x1 + a12 * 0 + a13 * 0 + a14 * x4 + a15 * x5 + a16 * x6 + b1' 
							// vn2 = a12 * x1 + a22 * 0 + a23 * 0 + a24 * x4 + a25 * x5 + a26 * x6 + b2' 
							// vn3 = a13 * x1 + a23 * 0 + a33 * 0 + a34 * x4 + a35 * x5 + a36 * x6 + b3' 
							//   0 = a14 * x1 + a24 * 0 + a34 * 0 + a44 * x4 + a45 * x5 + a46 * x6 + b4' 
							//   0 = a15 * x1 + a25 * 0 + a35 * 0 + a45 * x4 + a55 * x5 + a56 * x6 + b5' 
							//   0 = a16 * x1 + a26 * 0 + a36 * 0 + a46 * x4 + a55 * x5 + a66 * x6 + b6' 
							//

							x[0] = -constraintPoints[0]->normalMass * b[0];
							x[1] = 0.0f;
							x[2] = 0.0f;
							x[3] = -constraintPoints[3]->normalMass * b[3];
							x[4] = -constraintPoints[4]->normalMass * b[4];
							x[5] = -constraintPoints[5]->normalMass * b[5];

							vn[0] = 0.0f;
							vn[1] = velocityConstraint->K[triangleIndex][1][0] * x[0] + velocityConstraint->K[triangleIndex][1][3] * x[3] + velocityConstraint->K[triangleIndex][1][4] * x[4] + velocityConstraint->K[triangleIndex][1][5] * x[5] + b[1];
							vn[2] = velocityConstraint->K[triangleIndex][2][0] * x[0] + velocityConstraint->K[triangleIndex][2][3] * x[3] + velocityConstraint->K[triangleIndex][2][4] * x[4] + velocityConstraint->K[triangleIndex][2][5] * x[5] + b[2];
							vn[3] = 0.0f;
							vn[4] = 0.0f;
							vn[5] = 0.0f;
							if (x[0] >= 0.0f && vn[1] >= 0.0f && vn[2] >= 0.0f && x[3] >= 0.0f && x[4] >= 0.0f && x[5] >= 0.0f)
							{
								flag = 7;
								break;
							}
						}

						{
							// case 8: λn1 = vn2 = vn3 = λn4 = λn5 = λn6 = 0
							// x= {0, λn2, λn3, 0, 0, 0 };
							// 
							// vn1 = a11 * 0 + a12 * x2 + a13 * x3 + a14 * 0 + a15 * 0 + a16 * 0 + b1' 
							//   0 = a12 * 0 + a22 * x2 + a23 * x3 + a24 * 0 + a25 * 0 + a26 * 0 + b2' 
							//   0 = a13 * 0 + a23 * x2 + a33 * x3 + a34 * 0 + a35 * 0 + a36 * 0 + b3' 
							// vn4 = a14 * 0 + a24 * x2 + a34 * x3 + a44 * 0 + a45 * 0 + a46 * 0 + b4' 
							// vn5 = a15 * 0 + a25 * x2 + a35 * x3 + a45 * 0 + a55 * 0 + a56 * 0 + b5' 
							// vn6 = a16 * 0 + a26 * x2 + a36 * x3 + a46 * 0 + a55 * 0 + a66 * 0 + b6' 
							//

							x[0] = 0.0f;
							x[1] = -constraintPoints[1]->normalMass * b[1];
							x[2] = -constraintPoints[2]->normalMass * b[2];
							x[3] = 0.0f;
							x[4] = 0.0f;
							x[5] = 0.0f;

							vn[0] = velocityConstraint->K[triangleIndex][0][1] * x[1] + velocityConstraint->K[triangleIndex][0][2] * x[2] + b[0];
							vn[1] = 0.0f;
							vn[2] = 0.0f;
							vn[3] = velocityConstraint->K[triangleIndex][3][1] * x[1] + velocityConstraint->K[triangleIndex][3][2] * x[2] + b[3];
							vn[4] = velocityConstraint->K[triangleIndex][4][1] * x[1] + velocityConstraint->K[triangleIndex][4][2] * x[2] + b[4];
							vn[5] = velocityConstraint->K[triangleIndex][5][1] * x[1] + velocityConstraint->K[triangleIndex][5][2] * x[2] + b[5];
							if (vn[0] >= 0.0f && x[1] >= 0.0f && x[2] >= 0.0f && vn[3] >= 0.0f && vn[4] >= 0.0f && vn[5] >= 0.0f)
							{
								flag = 8;
								break;
							}
						}

						{
							//
							// case 9: vn1 = vn2 = λn3 = λn4 = λn5 = λn6 = 0
							// x= {λn1, λn2, 0, 0, 0, 0 };
							//
							//   0 = a11 * x1 + a12 * x2 + a13 * 0 + a14 * 0 + a15 * 0 + a16 * 0 + b1' 
							//   0 = a12 * x1 + a22 * x2 + a23 * 0 + a24 * 0 + a25 * 0 + a26 * 0 + b2' 
							// vn3 = a13 * x1 + a23 * x2 + a33 * 0 + a34 * 0 + a35 * 0 + a36 * 0 + b3' 
							// vn4 = a14 * x1 + a24 * x2 + a34 * 0 + a44 * 0 + a45 * 0 + a46 * 0 + b4' 
							// vn5 = a15 * x1 + a25 * x2 + a35 * 0 + a45 * 0 + a55 * 0 + a56 * 0 + b5' 
							// vn6 = a16 * x1 + a26 * x2 + a36 * 0 + a46 * 0 + a55 * 0 + a66 * 0 + b6' 
							//

							x[0] = -constraintPoints[0]->normalMass * b[0];
							x[1] = -constraintPoints[1]->normalMass * b[1];
							x[2] = 0.0f;
							x[3] = 0.0f;
							x[4] = 0.0f;
							x[5] = 0.0f;


							vn[0] = 0.0f;
							vn[1] = 0.0f;
							vn[2] = velocityConstraint->K[triangleIndex][2][0] * x[0] + velocityConstraint->K[triangleIndex][2][1] * x[1] + b[2];
							vn[3] = velocityConstraint->K[triangleIndex][3][0] * x[0] + velocityConstraint->K[triangleIndex][3][1] * x[1] + b[3];
							vn[4] = velocityConstraint->K[triangleIndex][4][0] * x[0] + velocityConstraint->K[triangleIndex][4][1] * x[1] + b[4];
							vn[5] = velocityConstraint->K[triangleIndex][5][0] * x[0] + velocityConstraint->K[triangleIndex][5][1] * x[1] + b[5];
							if (x[0] >= 0.0f && x[1] >= 0.0f && vn[2] >= 0.0f && vn[3] >= 0.0f && vn[4] >= 0.0f && vn[5] >= 0.0f)
							{
								flag = 9;
								break;
							}
						}

						{
							//
							// case 10: λn1 = λn2 = vn3 = vn4 = vn5 = vn6 = 0
							// x= {0, 0, λn3, λn4, λn5, λn6 };
							//
							// vn1 = a11 * 0 + a12 * 0 + a13 * x3 + a14 * x4 + a15 * x5 + a16 * x6 + b1' 
							// vn2 = a12 * 0 + a22 * 0 + a23 * x3 + a24 * x4 + a25 * x5 + a26 * x6 + b2' 
							//   0 = a13 * 0 + a23 * 0 + a33 * x3 + a34 * x4 + a35 * x5 + a36 * x6 + b3' 
							//   0 = a14 * 0 + a24 * 0 + a34 * x3 + a44 * x4 + a45 * x5 + a46 * x6 + b4' 
							//   0 = a15 * 0 + a25 * 0 + a35 * x3 + a45 * x4 + a55 * x5 + a56 * x6 + b5' 
							//   0 = a16 * 0 + a26 * 0 + a36 * x3 + a46 * x4 + a55 * x5 + a66 * x6 + b6' 
							//

							x[0] = 0.0f;
							x[1] = 0.0f;
							x[2] = -constraintPoints[2]->normalMass * b[2];
							x[3] = -constraintPoints[3]->normalMass * b[3];
							x[4] = -constraintPoints[4]->normalMass * b[4];
							x[5] = -constraintPoints[5]->normalMass * b[5];

							vn[0] = velocityConstraint->K[triangleIndex][0][2] * x[2] + velocityConstraint->K[triangleIndex][0][3] * x[3] + velocityConstraint->K[triangleIndex][0][4] * x[4] + velocityConstraint->K[triangleIndex][0][5] * x[5] + b[0];
							vn[1] = velocityConstraint->K[triangleIndex][1][2] * x[2] + velocityConstraint->K[triangleIndex][1][3] * x[3] + velocityConstraint->K[triangleIndex][1][4] * x[4] + velocityConstraint->K[triangleIndex][1][5] * x[5] + b[1];
							vn[2] = 0.0f;
							vn[3] = 0.0f;
							vn[4] = 0.0f;
							vn[5] = 0.0f;
							if (vn[0] >= 0.0f && vn[1] >= 0.0f && x[2] >= 0.0f && x[3] >= 0.0f && x[4] >= 0.0f && x[5] >= 0.0f)
							{
								flag = 10;
								break;
							}
						}

						{
							//
							// case 11: λn1 = λn2 = vn3 = vn4 = λn5 = λn6 = 0
							// x= {0, 0, λn3, λn4, 0, 0 };
							// 
							//
							// vn1 = a11 * 0 + a12 * 0 + a13 * x3 + a14 * x4 + a15 * 0 + a16 * 0 + b1' 
							// vn2 = a12 * 0 + a22 * 0 + a23 * x3 + a24 * x4 + a25 * 0 + a26 * 0 + b2' 
							//   0 = a13 * 0 + a23 * 0 + a33 * x3 + a34 * x4 + a35 * 0 + a36 * 0 + b3' 
							//   0 = a14 * 0 + a24 * 0 + a34 * x3 + a44 * x4 + a45 * 0 + a46 * 0 + b4' 
							// vn5 = a15 * 0 + a25 * 0 + a35 * x3 + a45 * x4 + a55 * 0 + a56 * 0 + b5' 
							// vn6 = a16 * 0 + a26 * 0 + a36 * x3 + a46 * x4 + a55 * 0 + a66 * 0 + b6' 
							//

							x[0] = 0.0f;
							x[1] = 0.0f;
							x[2] = -constraintPoints[2]->normalMass * b[2];
							x[3] = -constraintPoints[3]->normalMass * b[3];
							x[4] = 0.0f;
							x[5] = 0.0f;

							vn[0] = velocityConstraint->K[triangleIndex][0][2] * x[2] + velocityConstraint->K[triangleIndex][0][3] * x[3] + b[0];
							vn[1] = velocityConstraint->K[triangleIndex][1][2] * x[2] + velocityConstraint->K[triangleIndex][1][3] * x[3] + b[1];
							vn[2] = 0.0f;
							vn[3] = 0.0f;
							vn[4] = velocityConstraint->K[triangleIndex][4][2] * x[2] + velocityConstraint->K[triangleIndex][4][3] * x[3] + b[4];
							vn[5] = velocityConstraint->K[triangleIndex][5][2] * x[2] + velocityConstraint->K[triangleIndex][5][3] * x[3] + b[5];
							if (vn[0] >= 0.0f && vn[1] >= 0.0f && x[2] >= 0.0f && x[3] >= 0.0f && vn[4] >= 0.0f && vn[5] >= 0.0f)
							{
								flag = 11;
								break;
							}
						}

						{
							//
							// case 12: vn1 = vn2 = λn3 = λn4 = vn5 = vn6 = 0
							// x= {λn1, λn2, 0, 0, λn5, λn6 };
							// 
							//   0 = a11 * x1 + a12 * x2 + a13 * 0 + a14 * 0 + a15 * x5 + a16 * x6 + b1' 
							//   0 = a12 * x1 + a22 * x2 + a23 * 0 + a24 * 0 + a25 * x5 + a26 * x6 + b2' 
							// vn3 = a13 * x1 + a23 * x2 + a33 * 0 + a34 * 0 + a35 * x5 + a36 * x6 + b3' 
							// vn4 = a14 * x1 + a24 * x2 + a34 * 0 + a44 * 0 + a45 * x5 + a46 * x6 + b4' 
							//   0 = a15 * x1 + a25 * x2 + a35 * 0 + a45 * 0 + a55 * x5 + a56 * x6 + b5' 
							//   0 = a16 * x1 + a26 * x2 + a36 * 0 + a46 * 0 + a55 * x5 + a66 * x6 + b6' 
							//

							x[0] = -constraintPoints[0]->normalMass * b[0];
							x[1] = -constraintPoints[1]->normalMass * b[1];
							x[2] = 0.0f;
							x[3] = 0.0f;
							x[4] = -constraintPoints[4]->normalMass * b[4];
							x[5] = -constraintPoints[5]->normalMass * b[5];

							vn[0] = 0.0f;
							vn[1] = 0.0f;
							vn[2] = velocityConstraint->K[triangleIndex][2][0] * x[0] + velocityConstraint->K[triangleIndex][2][1] * x[1] + velocityConstraint->K[triangleIndex][2][4] * x[4] + velocityConstraint->K[triangleIndex][2][5] * x[5] + b[2];
							vn[3] = velocityConstraint->K[triangleIndex][3][0] * x[0] + velocityConstraint->K[triangleIndex][3][1] * x[1] + velocityConstraint->K[triangleIndex][3][4] * x[4] + velocityConstraint->K[triangleIndex][3][5] * x[5] + b[3];
							vn[4] = 0.0f;
							vn[5] = 0.0f;
							if (x[0] >= 0.0f && x[1] >= 0.0f && vn[2] >= 0.0f && vn[3] >= 0.0f && x[4] >= 0.0f && x[5] >= 0.0f)
							{
								flag = 12;
								break;
							}
						}

						{
							// case 13: λn1 = λn2 = λn3 = λn4 = vn5 = vn6 = 0
							// x= {0, 0, 0, 0, λn5, λn6 };
							// 
							// vn1 = a11 * 0 + a12 * 0 + a13 * 0 + a14 * 0 + a15 * x5 + a16 * x6 + b1' 
							// vn2 = a12 * 0 + a22 * 0 + a23 * 0 + a24 * 0 + a25 * x5 + a26 * x6 + b2' 
							// vn3 = a13 * 0 + a23 * 0 + a33 * 0 + a34 * 0 + a35 * x5 + a36 * x6 + b3' 
							// vn4 = a14 * 0 + a24 * 0 + a34 * 0 + a44 * 0 + a45 * x5 + a46 * x6 + b4' 
							//   0 = a15 * 0 + a25 * 0 + a35 * 0 + a45 * 0 + a55 * x5 + a56 * x6 + b5' 
							//   0 = a16 * 0 + a26 * 0 + a36 * 0 + a46 * 0 + a55 * x5 + a66 * x6 + b6' 
							//

							x[0] = 0.0f;
							x[1] = 0.0f;
							x[2] = 0.0f;
							x[3] = 0.0f;
							x[4] = -constraintPoints[4]->normalMass * b[4];
							x[5] = -constraintPoints[5]->normalMass * b[5];

							vn[0] = velocityConstraint->K[triangleIndex][0][4] * x[4] + velocityConstraint->K[triangleIndex][0][5] * x[5] + b[0];
							vn[1] = velocityConstraint->K[triangleIndex][1][4] * x[4] + velocityConstraint->K[triangleIndex][1][5] * x[5] + b[1];
							vn[2] = velocityConstraint->K[triangleIndex][2][4] * x[4] + velocityConstraint->K[triangleIndex][2][5] * x[5] + b[2];
							vn[3] = velocityConstraint->K[triangleIndex][3][4] * x[4] + velocityConstraint->K[triangleIndex][3][5] * x[5] + b[3];
							vn[4] = 0.0f;
							vn[5] = 0.0f;
							if (vn[0] >= 0.0f && vn[1] >= 0.0f && vn[2] >= 0.0f && vn[3] >= 0.0f && x[4] >= 0.0f && x[5] >= 0.0f)
							{
								flag = 13;
								break;
							}
						}

						{
							// case 14: vn1 = vn2 = vn3 = vn4 = λn5 = λn6 = 0
							// x= {λn1, λn2, λn3, λn4, 0, 0 };
							// 
							//   0 = a11 * x1 + a12 * x2 + a13 * x3 + a14 * x4 + a15 * 0 + a16 * 0 + b1' 
							//   0 = a12 * x1 + a22 * x2 + a23 * x3 + a24 * x4 + a25 * 0 + a26 * 0 + b2' 
							//   0 = a13 * x1 + a23 * x2 + a33 * x3 + a34 * x4 + a35 * 0 + a36 * 0 + b3' 
							//   0 = a14 * x1 + a24 * x2 + a34 * x3 + a44 * x4 + a45 * 0 + a46 * 0 + b4' 
							// vn5 = a15 * x1 + a25 * x2 + a35 * x3 + a45 * x4 + a55 * 0 + a56 * 0 + b5' 
							// vn6 = a16 * x1 + a26 * x2 + a36 * x3 + a46 * x4 + a55 * 0 + a66 * 0 + b6' 
							//

							x[0] = -constraintPoints[0]->normalMass * b[0];
							x[1] = -constraintPoints[1]->normalMass * b[1];
							x[2] = -constraintPoints[2]->normalMass * b[2];
							x[3] = -constraintPoints[3]->normalMass * b[3];
							x[4] = 0.0f;
							x[5] = 0.0f;

							vn[0] = 0.0f;
							vn[1] = 0.0f;
							vn[2] = 0.0f;
							vn[3] = 0.0f;
							vn[4] = velocityConstraint->K[triangleIndex][4][0] * x[0] + velocityConstraint->K[triangleIndex][4][1] * x[1] + velocityConstraint->K[triangleIndex][4][2] * x[2] + velocityConstraint->K[triangleIndex][4][3] * x[3] + b[4];
							vn[5] = velocityConstraint->K[triangleIndex][5][0] * x[0] + velocityConstraint->K[triangleIndex][5][1] * x[1] + velocityConstraint->K[triangleIndex][5][2] * x[2] + velocityConstraint->K[triangleIndex][5][3] * x[3] + b[5];
							if (x[0] >= 0.0f && x[1] >= 0.0f && x[2] >= 0.0f && x[3] >= 0.0f && vn[4] >= 0.0f && vn[5] >= 0.0f)
							{
								flag = 14;
								break;
							}
						}

						// 没有解决办法，放弃。这有时会被击中，但似乎并不重要。
						// No solution, give up. This is hit sometimes, but it doesn't seem to matter.
						break;
					}

					if (flag != -1)
					{
						VOL_TRACE("case " + std::to_string(flag));
						// 冲量增量
						std::array<float, 6> d;
						for (int i = 0; i < 6; i++)
							d[i] = x[i] - a[i];

						// 应用冲量增量
						std::array<glm::vec3, 6> P;
						for (int i = 0; i < 6; i++)
							P[i] = d[i] * normal;

						glm::vec3 sumP = glm::vec3(0.0f);
						for (int i = 0; i < 6; i++)
							sumP += P[i];

						vATemp[triangleIndex] -= imA * (sumP / (float)b3_MaxManifoldPoints);
						//vA -= imA * sumP;

						glm::vec3 sum_rA_cross_P = glm::vec3(0.0f);
						for (int i = 0; i < 6; i++)
							sum_rA_cross_P += glm::cross(constraintPoints[i]->rA, P[i]);
						wATemp[triangleIndex] -= iIA * (sum_rA_cross_P / (float)b3_MaxManifoldPoints);
						//wA -= iIA * (sum_rA_cross_P);

						vBTemp[triangleIndex] += imB * (sumP / (float)b3_MaxManifoldPoints);
						//vB += imB * sumP;

						glm::vec3 sum_rB_cross_P = glm::vec3(0.0f);
						for (int i = 0; i < 6; i++)
							sum_rB_cross_P += glm::cross(constraintPoints[i]->rB, P[i]);
						wBTemp[triangleIndex] += iIB * (sum_rB_cross_P / (float)b3_MaxManifoldPoints);
						//wB += iIB * (sum_rB_cross_P);

						// Accumulate
						for (int i = 0; i < 6; i++)
							constraintPoints[i]->normalImpulse = x[i];
					}

					for (int i = 0; i < 6; i++)
						VOL_ASSERT(std::isfinite(constraintPoints[i]->normalImpulse));
				}

				for (int i = 0; i < pointCount / 6; i++)
				{
					vA += vATemp[i];
					wA += wATemp[i];
					vB += vBTemp[i];
					wB += wBTemp[i];
				}
			}

			VOL_TRACE("vA: " + std::to_string(vA.x) + ", " + std::to_string(vA.y) + ", " + std::to_string(vA.z));
			VOL_TRACE("wA: " + std::to_string(wA.x) + ", " + std::to_string(wA.y) + ", " + std::to_string(wA.z));
			VOL_ASSERT(std::isfinite(vA.x));
			VOL_ASSERT(std::isfinite(wA.x) && std::isfinite(wA.y) && std::isfinite(wA.z));
			VOL_ASSERT(std::isfinite(vB.x));
			VOL_ASSERT(std::isfinite(wB.x) && std::isfinite(wB.y) && std::isfinite(wB.z));
			m_velocities[indexA].linearVelocity  = vA;
			m_velocities[indexA].angularVelocity = wA;
			m_velocities[indexB].linearVelocity  = vB;
			m_velocities[indexB].angularVelocity = wB;
		}
	}
	void b3_ContactSolver::StoreImpulses()
	{
		for (int i = 0; i < m_count; ++i)
		{
			b3_ContactVelocityConstraint* vc = m_velocityConstraints + i;
			b3_Manifold* manifold = m_contacts[vc->contactIndex]->GetManifold();

			for (int j = 0; j < vc->pointCount; ++j)
			{
				manifold->points[j].normalImpulse = vc->points[j].normalImpulse;
				manifold->points[j].tangentImpulse = vc->points[j].tangentImpulse;
			}
		}
	}

	struct b3_PositionSolverManifold
	{
		void Initialize(b3_ContactPositionConstraint* positionConstraint, const b3_Transform& transformA, const b3_Transform& transformB, int index)
		{
			assert(positionConstraint->pointCount > 0);

			switch (positionConstraint->type)
			{
			case b3_Manifold::e_circles:
			{
				glm::vec3 pointA = b3_Multiply(transformA, positionConstraint->localPoint);
				glm::vec3 pointB = b3_Multiply(transformB, positionConstraint->localPoints[0]);
				normal = pointB - pointA;
				normal = glm::normalize(normal);
				point = 0.5f * (pointA + pointB);
				separation = glm::dot(pointB - pointA, normal) - positionConstraint->radiusA - positionConstraint->radiusB;
			}
			break;

			case b3_Manifold::e_faceA:
			{
				normal = b3_Multiply(transformA.rotation, positionConstraint->localNormal);
				glm::vec3 planePoint = b3_Multiply(transformA, positionConstraint->localPoint);

				glm::vec3 clipPoint = b3_Multiply(transformB, positionConstraint->localPoints[index]);
				separation = glm::dot(clipPoint - planePoint, normal) - positionConstraint->radiusA - positionConstraint->radiusB;
				point = clipPoint;
			}
			break;

			case b3_Manifold::e_faceB:
			{
				normal = b3_Multiply(transformB.rotation, positionConstraint->localNormal);
				glm::vec3 planePoint = b3_Multiply(transformB, positionConstraint->localPoint);

				glm::vec3 clipPoint = b3_Multiply(transformA, positionConstraint->localPoints[index]);
				separation = glm::dot(clipPoint - planePoint, normal) - positionConstraint->radiusA - positionConstraint->radiusB;
				point = clipPoint;

				// Ensure normal points from A to B
				normal = -normal;
			}
			break;
			}
		}

		glm::vec3 normal;
		glm::vec3 point;
		float separation;
	};

	// Sequential solver.
	bool b3_ContactSolver::SolvePositionConstraints()
	{
		float minSeparation = 0.0f;

		for (int i = 0; i < m_count; ++i)
		{
			b3_ContactPositionConstraint* positionConstraint = m_positionConstraints + i;

			int indexA             = positionConstraint->indexA;
			int indexB             = positionConstraint->indexB;
			glm::vec3 localCenterA = positionConstraint->localCenterA;
			float imA              = positionConstraint->invMassA;
			glm::mat3 iIA          = positionConstraint->invIA;
			glm::vec3 localCenterB = positionConstraint->localCenterB;
			float imB              = positionConstraint->invMassB;
			glm::mat3 iIB          = positionConstraint->invIB;
			int pointCount         = positionConstraint->pointCount;

			const glm::vec3 cA           = m_positions[indexA].position;
			const glm::vec3 aA           = m_positions[indexA].rotation;
			const glm::vec3 cB           = m_positions[indexB].position;
			const glm::vec3 aB           = m_positions[indexB].rotation;

			std::vector<glm::vec3> cATemp; cATemp.resize(pointCount / 6); for (int i = 0; i < cATemp.size(); i++) cATemp[i] = glm::vec3(0.0f);
			std::vector<glm::vec3> aATemp; aATemp.resize(pointCount / 6); for (int i = 0; i < aATemp.size(); i++) aATemp[i] = glm::vec3(0.0f);
			std::vector<glm::vec3> cBTemp; cBTemp.resize(pointCount / 6); for (int i = 0; i < cBTemp.size(); i++) cBTemp[i] = glm::vec3(0.0f);
			std::vector<glm::vec3> aBTemp; aBTemp.resize(pointCount / 6); for (int i = 0; i < aBTemp.size(); i++) aBTemp[i] = glm::vec3(0.0f);

			// 结算法向约束 
			for (int triangleIndex = 0; triangleIndex < pointCount / 6; triangleIndex++)
			{
				for (int pointIndex = 0; pointIndex < 6; ++pointIndex)
				{
					int pi = triangleIndex * 6 + pointIndex;
					b3_Transform transformA, transformB;
					transformA.rotation.Set(aA);
					transformB.rotation.Set(aB);
					transformA.position = cA - b3_Multiply(transformA.rotation, localCenterA);
					transformB.position = cB - b3_Multiply(transformB.rotation, localCenterB);

					b3_PositionSolverManifold positionSolverManifold;
					positionSolverManifold.Initialize(positionConstraint, transformA, transformB, pi);
					glm::vec3 normal = positionSolverManifold.normal;

					glm::vec3 point = positionSolverManifold.point;
					float separation = positionSolverManifold.separation;

					glm::vec3 rA = point - cA;
					glm::vec3 rB = point - cB;

					// 跟踪最大约束误差。Track max constraint error.
					minSeparation = glm::min(minSeparation, separation);

					// 防止大幅修正，允许倾斜(slop)。 Prevent large corrections and allow slop.
					float C = b3_Clamp(b3_Baumgarte * (separation + b3_LinearSlop), -b3_MaxLinearCorrection, 0.0f);

					// 计算有效质量
					glm::vec3 rA_cross_n = glm::cross(rA, normal);
					glm::vec3 rB_cross_n = glm::cross(rB, normal);
					float K = imA + glm::dot(iIA * rA_cross_n * rA_cross_n) + imB + glm::dot(iIB * rB_cross_n * rB_cross_n);
					// 计算法向冲量
					float impulse = K > 0.0f ? -C / K : 0.0f;

					glm::vec3 P = impulse * normal;

					// L = Iw = r×P
					cATemp[i] -= imA * P;
					aATemp[i] -= iIA * glm::cross(rA, P);
					cBTemp[i] += imB * P;
					aBTemp[i] += iIB * glm::cross(rB, P);
				}
				for (int i = 0; i < 3; i++)
				{
					if (glm::abs(cATemp[triangleIndex][i]) < 0.1f * b3_LinearSlop) cATemp[triangleIndex][i] = 0.0f;
					if (glm::abs(aATemp[triangleIndex][i]) < 0.1f * b3_LinearSlop) aATemp[triangleIndex][i] = 0.0f;
					if (glm::abs(cBTemp[triangleIndex][i]) < 0.1f * b3_LinearSlop) cBTemp[triangleIndex][i] = 0.0f;
					if (glm::abs(aBTemp[triangleIndex][i]) < 0.1f * b3_LinearSlop) aBTemp[triangleIndex][i] = 0.0f;
				}

				m_positions[indexA].position += cATemp[triangleIndex];
				m_positions[indexA].rotation += aATemp[triangleIndex];
				m_positions[indexB].position += cBTemp[triangleIndex];
				m_positions[indexB].rotation += aBTemp[triangleIndex];
			}
		}

		// 我们不能期望minSpeparation >= -b3_LinearSlop，因为我们没有将separation推到超过-b3_Lineslop。
		return minSeparation >= -3.0f * b3_LinearSlop;
	}

	// Sequential position solver for position constraints.
	bool b3_ContactSolver::SolveTOIPositionConstraints(int toiIndexA, int toiIndexB)
	{
		float minSeparation = 0.0f;

		for (int i = 0; i < m_count; ++i)
		{
			b3_ContactPositionConstraint* pc = m_positionConstraints + i;

			int indexA = pc->indexA;
			int indexB = pc->indexB;
			glm::vec3 localCenterA = pc->localCenterA;
			glm::vec3 localCenterB = pc->localCenterB;
			int pointCount = pc->pointCount;

			float imA = 0.0f;
			glm::mat3 iIA = 0.0f;
			if (indexA == toiIndexA || indexA == toiIndexB)
			{
				imA = pc->invMassA;
				iIA = pc->invIA;
			}

			float imB = 0.0f;
			glm::mat3 iIB = 0.0f;
			if (indexB == toiIndexA || indexB == toiIndexB)
			{
				imB = pc->invMassB;
				iIB = pc->invIB;
			}

			glm::vec3 cA = m_positions[indexA].position;
			glm::vec3 aA = m_positions[indexA].rotation;

			glm::vec3 cB = m_positions[indexB].position;
			glm::vec3 aB = m_positions[indexB].rotation;

			// Solve normal constraints
			for (int j = 0; j < pointCount; ++j)
			{
				b3_Transform transformA, transformB;
				transformA.rotation.Set(aA);
				transformB.rotation.Set(aB);
				transformA.position = cA - b3_Multiply(transformA.rotation, localCenterA);
				transformB.position = cB - b3_Multiply(transformB.rotation, localCenterB);

				b3_PositionSolverManifold psm;
				psm.Initialize(pc, transformA, transformB, j);
				glm::vec3 normal = psm.normal;

				glm::vec3 point = psm.point;
				float separation = psm.separation;

				glm::vec3 rA = point - cA;
				glm::vec3 rB = point - cB;

				// Track max constraint error.
				minSeparation = glm::min(minSeparation, separation);

				// Prevent large corrections and allow slop.
				float C = b3_Clamp(b3_TOIBaumgarte * (separation + b3_LinearSlop), -b3_MaxLinearCorrection, 0.0f);

				// Compute the effective mass.
				glm::vec3 rA_cross_n = glm::cross(rA, normal);
				glm::vec3 rB_cross_n = glm::cross(rB, normal);
				float K = imA + glm::dot(iIA * rA_cross_n * rA_cross_n) + imB + glm::dot(iIB * rB_cross_n * rB_cross_n);

				// Compute normal impulse
				float impulse = K > 0.0f ? -C / K : 0.0f;

				glm::vec3 P = impulse * normal;

				cA -= imA * P;
				aA -= iIA * glm::cross(rA, P);

				cB += imB * P;
				aB += iIB * glm::cross(rB, P);
			}

			m_positions[indexA].position = cA;
			m_positions[indexA].rotation = aA;

			m_positions[indexB].position = cB;
			m_positions[indexB].rotation = aB;
		}

		// We can't expect minSpeparation >= -b3__linearSlop because we don't
		// push the separation above -b3__linearSlop.
		return minSeparation >= -1.5f * b3_LinearSlop;
	}
	
}
