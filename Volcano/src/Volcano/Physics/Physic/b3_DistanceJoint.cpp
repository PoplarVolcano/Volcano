#include "volpch.h"

#include "b3_DistanceJoint.h"
#include "b3_Body.h"

namespace Volcano {

	void b3_DistanceJointDef::Initialize(b3_Body* bodyA, b3_Body* bodyB, const glm::vec3& anchorA, const glm::vec3& anchorB)
	{
		this->bodyA = bodyA;
		this->bodyB = bodyB;
		localAnchorA = this->bodyA->GetLocalPoint(anchorA);
		localAnchorB = this->bodyB->GetLocalPoint(anchorB);
		glm::vec3 d = anchorB - anchorA;
		length = glm::max(glm::length(d), b3_LinearSlop);
		minLength = length;
		maxLength = length;
	}

	glm::vec3 b3_DistanceJoint::GetAnchorA() const
	{
		return m_bodyA->GetWorldPoint(m_localAnchorA);
	}

	glm::vec3 b3_DistanceJoint::GetAnchorB() const
	{
		return m_bodyB->GetWorldPoint(m_localAnchorB);
	}

	// ？？？
	glm::vec3 b3_DistanceJoint::GetReactionForce(float int_deltaTime) const
	{
		glm::vec3 F = int_deltaTime * (m_impulse + m_lowerImpulse - m_upperImpulse) * m_u;
		return F;
	}

	glm::vec3 b3_DistanceJoint::GetReactionTorque(float int_deltaTime) const
	{
		(void)(int_deltaTime); // not used
		return glm::vec3(0.0f);
	}

	float b3_DistanceJoint::SetLength(float length)
	{
		m_impulse = 0.0f;
		m_length = glm::max(b3_LinearSlop, length);
		return m_length;
	}

	float b3_DistanceJoint::SetMinLength(float minLength)
	{
		m_lowerImpulse = 0.0f;
		m_minLength = glm::max(b3_LinearSlop, glm::min(minLength, m_maxLength)); // clamp = max(b3_LinearSlop, min(minLength, m_maxLength)) 
		return m_minLength;
	}

	float b3_DistanceJoint::SetMaxLength(float maxLength)
	{
		m_upperImpulse = 0.0f;
		m_maxLength = glm::max(maxLength, m_minLength);
		return m_maxLength;
	}

	float b3_DistanceJoint::GetCurrentLength() const
	{
		glm::vec3 pA = m_bodyA->GetWorldPoint(m_localAnchorA);
		glm::vec3 pB = m_bodyB->GetWorldPoint(m_localAnchorB);
		glm::vec3 d = pB - pA;
		return glm::length(d);
	}

	b3_DistanceJoint::b3_DistanceJoint(const b3_DistanceJointDef* def)
		: b3_Joint(def)
	{
		m_localAnchorA = def->localAnchorA;
		m_localAnchorB = def->localAnchorB;
		m_length = glm::max(def->length, b3_LinearSlop);
		m_minLength = glm::max(def->minLength, b3_LinearSlop);
		m_maxLength = glm::max(def->maxLength, m_minLength);
		m_stiffness = def->stiffness;
		m_damping = def->damping;

		m_gamma = 0.0f;
		m_bias = 0.0f;
		m_impulse = 0.0f;
		m_lowerImpulse = 0.0f;
		m_upperImpulse = 0.0f;
		m_currentLength = 0.0f;
	}

	void b3_DistanceJoint::InitVelocityConstraints(const b3_SolverData& data)
	{
		m_indexA = m_bodyA->m_islandIndex;
		m_indexB = m_bodyB->m_islandIndex;
		m_localCenterA = m_bodyA->m_sweep.localCenter;
		m_localCenterB = m_bodyB->m_sweep.localCenter;
		m_invMassA = m_bodyA->m_invMass;
		m_invMassB = m_bodyB->m_invMass;
		m_invIA = m_bodyA->m_invI;
		m_invIB = m_bodyB->m_invI;

		glm::vec3 positionA        = data.positions[m_indexA].position;
		glm::vec3 rotationA        = data.positions[m_indexA].rotation;
		glm::vec3 vA               = data.velocities[m_indexA].linearVelocity;  // linearVelocityA
		glm::vec3 wA               = data.velocities[m_indexA].angularVelocity;	// angularVelocityA

		glm::vec3 positionB        = data.positions[m_indexB].position;          
		glm::vec3 rotationB        = data.positions[m_indexB].rotation;			 
		glm::vec3 vB               = data.velocities[m_indexB].linearVelocity;  // linearVelocityB
		glm::vec3 wB               = data.velocities[m_indexB].angularVelocity;	// angularVelocityB

		b3_Rotation rA(rotationA), rB(rotationB);

		m_rA = b3_Multiply(rA, m_localAnchorA - m_localCenterA); // 经过rotationA旋转的向量原点a->锚点a
		m_rB = b3_Multiply(rB, m_localAnchorB - m_localCenterB); // 经过rotationB旋转的向量原点b->锚点b
		m_u = positionB + m_rB - positionA - m_rA; // 向量锚点A->锚点B

		//处理奇异性(singularity)。
		m_currentLength = glm::length(m_u);
		if (m_currentLength > b3_LinearSlop)
		{
			m_u *= 1.0f / m_currentLength; // 标准化
		}
		else
		{
			m_u = { 0.0f, 0.0f, 0.0f };
			m_mass = 0.0f;
			m_impulse = 0.0f;
			m_lowerImpulse = 0.0f;
			m_upperImpulse = 0.0f;
		}

		// C = norm(p2 - p1) - L
		// u = (p2 - p1) / norm(p2 - p1)
		// Cdot = dot(u, v2 + cross(w2, r2) - v1 - cross(w1, r1))
		// J = [-u -cross(r1, u) u cross(r2, u)]
		// K = J * invM * JT
		//   = invMass1 + invI1 * cross(r1, u)^2 + invMass2 + invI2 * cross(r2, u)^2

		// 约束 C(p(t))    p(t)：位置position随时间变化，整体速度  dp/dt = v + w×r, rb为锚点到接触点的半径
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
		// dC/dt = n(va+Rsawa-vb-Rsbwb) = n^T[I3x3  Rsa -I3x3 -Rsb][va wa vb wb]^T  I3x3是3x3的单位矩阵
		// 
		// J = dC/dt / v = [n^T n^TRsa -n^T -n^TRsb]  // 雅可比矩阵
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

		glm::vec3 crossrAu = glm::cross(m_rA, m_u);
		glm::vec3 crossrBu = glm::cross(m_rB, m_u);
		float invMass = m_invMassA + glm::length(m_invIA * glm::dot(crossrAu, crossrAu)) + m_invMassB + glm::length(m_invIB * glm::dot(crossrBu, crossrBu));
		m_mass = invMass != 0.0f ? 1.0f / invMass : 0.0f;

		// 1-D constrained system
		// m (v2 - v1) = lambda(λ)
		// v2 + (beta/h) * x1 + gamma * lambda = 0, gamma has units of inverse mass.
		// x2 = x1 + h * v2

		// 1-D mass-damper-spring system
		// m (v2 - v1) + h * d * v2 + h * k * 

		if (m_stiffness > 0.0f && m_minLength < m_maxLength)
		{
			// soft
			float C = m_currentLength - m_length;

			float damping = m_damping;      // d
			float stiffness = m_stiffness;  // k

			// 神奇公式(formulas)
			float deltaTime = data.step.deltaTime;  // h

			// 阻尼力F = k*x+c*v  x形变量 x2 = x1 + h * v2
			// F = khv+cv  
			// m = hF/v = h(kh+c)
			// gamma = 1 / (h * (d + h * k))
			// 分母(denominator)中的额外因子h是因为lambda是一个冲量，而不是力
			m_gamma = deltaTime * (damping + deltaTime * stiffness); 
			m_gamma = m_gamma != 0.0f ? 1.0f / m_gamma : 0.0f;
			m_bias = C * deltaTime * stiffness * m_gamma;  //刚体弹力冲量

			invMass += m_gamma;
			m_softMass = invMass != 0.0f ? 1.0f / invMass : 0.0f;
		}
		else
		{
			// rigid
			m_gamma = 0.0f;
			m_bias = 0.0f;
			m_softMass = m_mass;
		}

		if (data.step.warmStarting)
		{
			//缩放冲量以支持可变时间步长
			m_impulse *= data.step.deltaTimeRatio;
			m_lowerImpulse *= data.step.deltaTimeRatio;
			m_upperImpulse *= data.step.deltaTimeRatio;

			glm::vec3 P = (m_impulse + m_lowerImpulse - m_upperImpulse) * m_u;
			vA -= m_invMassA * P;
			wA -= m_invIA * glm::cross(m_rA, P);
			vB += m_invMassB * P;
			wB += m_invIB * glm::cross(m_rB, P);
		}
		else
		{
			m_impulse = 0.0f;
		}

		data.velocities[m_indexA].linearVelocity  = vA;
		data.velocities[m_indexA].angularVelocity = wA;
		data.velocities[m_indexB].linearVelocity  = vB;
		data.velocities[m_indexB].angularVelocity = wB;
	}

	void b3_DistanceJoint::SolveVelocityConstraints(const b3_SolverData& data)
	{
		glm::vec3 vA = data.velocities[m_indexA].linearVelocity;
		glm::vec3 wA = data.velocities[m_indexA].angularVelocity;
		glm::vec3 vB = data.velocities[m_indexB].linearVelocity;
		glm::vec3 wB = data.velocities[m_indexB].angularVelocity;

		if (m_minLength < m_maxLength)
		{
			if (m_stiffness > 0.0f)
			{
				// Cdot = dot(u, v + cross(w, r))
				glm::vec3 vpA = vA + glm::cross(wA, m_rA);
				glm::vec3 vpB = vB + glm::cross(wB, m_rB);
				float Cdot = glm::dot(m_u, vpB - vpA);

				float impulse = -m_softMass * (Cdot + m_bias + m_gamma * m_impulse);
				m_impulse += impulse;

				glm::vec3 P = impulse * m_u;
				vA -= m_invMassA * P;
				wA -= m_invIA * glm::cross(m_rA, P);
				vB += m_invMassB * P;
				wB += m_invIB * glm::cross(m_rB, P);
			}

			// lower
			{
				float C = m_currentLength - m_minLength;
				float bias = glm::max(0.0f, C) * data.step.inv_deltaTime;

				glm::vec3 vpA = vA + glm::cross(wA, m_rA);
				glm::vec3 vpB = vB + glm::cross(wB, m_rB);
				float Cdot = glm::dot(m_u, vpB - vpA);

				float impulse = -m_mass * (Cdot + bias);
				float oldImpulse = m_lowerImpulse;
				m_lowerImpulse = glm::max(0.0f, m_lowerImpulse + impulse);
				impulse = m_lowerImpulse - oldImpulse;
				glm::vec3 P = impulse * m_u;

				vA -= m_invMassA * P;
				wA -= m_invIA * glm::cross(m_rA, P);
				vB += m_invMassB * P;
				wB += m_invIB * glm::cross(m_rB, P);
			}

			// upper
			{
				float C = m_maxLength - m_currentLength;
				float bias = glm::max(0.0f, C) * data.step.inv_deltaTime;

				glm::vec3 vpA = vA + glm::cross(wA, m_rA);
				glm::vec3 vpB = vB + glm::cross(wB, m_rB);
				float Cdot = glm::dot(m_u, vpA - vpB);

				float impulse = -m_mass * (Cdot + bias);
				float oldImpulse = m_upperImpulse;
				m_upperImpulse = glm::max(0.0f, m_upperImpulse + impulse);
				impulse = m_upperImpulse - oldImpulse;
				glm::vec3 P = -impulse * m_u;

				vA -= m_invMassA * P;
				wA -= m_invIA * glm::cross(m_rA, P);
				vB += m_invMassB * P;
				wB += m_invIB * glm::cross(m_rB, P);
			}
		}
		else
		{
			// Equal limits

			// Cdot = dot(u, v + cross(w, r))
			glm::vec3 vpA = vA + glm::cross(wA, m_rA);
			glm::vec3 vpB = vB + glm::cross(wB, m_rB);
			float Cdot = glm::dot(m_u, vpB - vpA);

			float impulse = -m_mass * Cdot;
			m_impulse += impulse;

			glm::vec3 P = impulse * m_u;
			vA -= m_invMassA * P;
			wA -= m_invIA * glm::cross(m_rA, P);
			vB += m_invMassB * P;
			wB += m_invIB * glm::cross(m_rB, P);
		}

		data.velocities[m_indexA].linearVelocity  = vA;
		data.velocities[m_indexA].angularVelocity = wA;
		data.velocities[m_indexB].linearVelocity  = vB;
		data.velocities[m_indexB].angularVelocity = wB;
	}

	bool b3_DistanceJoint::SolvePositionConstraints(const b3_SolverData& data)
	{
		glm::vec3 cA = data.positions[m_indexA].position;
		glm::vec3 aA = data.positions[m_indexA].rotation;
		glm::vec3 cB = data.positions[m_indexB].position;
		glm::vec3 aB = data.positions[m_indexB].rotation;

		b3_Rotation qA(aA), qB(aB);

		glm::vec3 rA = b3_Multiply(qA, m_localAnchorA - m_localCenterA); // 经过rotationA旋转的向量原点a->锚点a 
		glm::vec3 rB = b3_Multiply(qB, m_localAnchorB - m_localCenterB); // 经过rotationB旋转的向量原点b->锚点b
		glm::vec3 u = cB + rB - cA - rA;   // 向量锚点A->锚点B

		float length = glm::length(u);
		u = glm::normalize(u);
		float C;
		if (m_minLength == m_maxLength)
		{
			C = length - m_minLength;
		}
		else if (length < m_minLength)
		{
			C = length - m_minLength;
		}
		else if (m_maxLength < length)
		{
			C = length - m_maxLength;
		}
		else
		{
			return true;
		}

		float impulse = -m_mass * C;
		glm::vec3 P = impulse * u;

		cA -= m_invMassA * P;
		aA -= m_invIA * glm::cross(rA, P);
		cB += m_invMassB * P;
		aB += m_invIB * glm::cross(rB, P);

		data.positions[m_indexA].position = cA;
		data.positions[m_indexA].rotation = aA;
		data.positions[m_indexB].position = cB;
		data.positions[m_indexB].rotation = aB;

		return glm::abs(C) < b3_LinearSlop;
	}

}