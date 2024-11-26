#pragma once

#include "b3_Joint.h"
#include "b3_TimeStep.h"

namespace Volcano {

	// Distance joint定义。这需要在两个物体上定义一个锚点，并定义distance joint的非零距离。
	// 该定义使用局部锚点，因此初始配置可能会略微违反(violate)约束(constraint)。这有助于保存和加载游戏。
	struct b3_DistanceJointDef : public b3_JointDef
	{
		b3_DistanceJointDef()
		{
			type = e_distanceJoint;
			localAnchorA = { 0.0f, 0.0f, 0.0f };
			localAnchorB = { 0.0f, 0.0f, 0.0f };
			length = 1.0f;
			minLength = 0.0f;
			maxLength = FLT_MAX;
			stiffness = 0.0f;
			damping = 0.0f;
		}

		// 使用世界空间锚点初始化实体、锚点和初始间距。最小和最大长度设置为初始间距
		void Initialize(b3_Body* bodyA, b3_Body* bodyB, const glm::vec3& anchorA, const glm::vec3& anchorB);
		
		glm::vec3 localAnchorA; // 相对于bodyA原点的局部锚点。
		glm::vec3 localAnchorB; // 相对于bodyB原点的局部锚点。
		float length;           // 这个关节的静止(rest)长度。被限制(Clamped)在一个稳定的最小值。
		float minLength;        // 最小长度。被限制(Clamped)在一个稳定的最小值。
		float maxLength;        // 最大长度。必须大于或等于最小长度。
		float stiffness;        // 线性刚度，单位为N/m。
		float damping;	        // 线性阻尼，单位为N*s/m。
	};

	// 一个距离关节 约束两个物体上的两个点保持彼此固定的距离。你可以将其视为一根无质量的刚性杆(rod)
	class b3_DistanceJoint : public b3_Joint
	{
	public:

		glm::vec3 GetAnchorA() const override;
		glm::vec3 GetAnchorB() const override;

		// 根据反时间步长(inverse time step)得到反作用力。  单位为N。
		glm::vec3 GetReactionForce(float int_deltaTime) const override;

		// 获得给定反时间步长(inverse time step)的反作用扭矩。单位为N*m。对于距离关节，该值始终为零。
		glm::vec3 GetReactionTorque(float int_deltaTime) const override;

		// 相对于bodyA原点的局部锚点
		const glm::vec3& GetLocalAnchorA() const { return m_localAnchorA; }

		// 相对于bodyB原点的局部锚点
		const glm::vec3& GetLocalAnchorB() const { return m_localAnchorB; }

		// 获取静止(rest)长度
		float GetLength() const { return m_length; }

		// 设置静止(rest)长度
		// @returns clamped rest length
		float SetLength(float length);

		// 获取最小长度
		float GetMinLength() const { return m_minLength; }

		// 设置最小长度
		// @returns the clamped minimum length
		float SetMinLength(float minLength);

		// 获取最大长度
		float GetMaxLength() const { return m_maxLength; }

		// 设置最大长度
		// @returns the clamped maximum length
		float SetMaxLength(float maxLength);

		// 获取现在长度
		float GetCurrentLength() const;

		// 设置/获取线性刚度，单位为N/m
		void SetStiffness(float stiffness) { m_stiffness = stiffness; }
		float GetStiffness() const { return m_stiffness; }

		// 设置/获取线性阻尼，单位为N*s/m
		void SetDamping(float damping) { m_damping = damping; }
		float GetDamping() const { return m_damping; }

		// Dump joint to dmLog
		//void Dump() override;

		//void Draw(b3_Draw* draw) const override;

	protected:

		friend class b3_Joint;

		b3_DistanceJoint(const b3_DistanceJointDef* def);

		void InitVelocityConstraints(const b3_SolverData& data) override;
		void SolveVelocityConstraints(const b3_SolverData& data) override;
		bool SolvePositionConstraints(const b3_SolverData& data) override;

		float m_stiffness;  // 刚度
		float m_damping;    // 阻尼
		float m_bias;       // 影响
		float m_length;     // 静止(rest)长度
		float m_minLength;  // 最小长度
		float m_maxLength;  // 最大长度

		// Solver shared
		glm::vec3 m_localAnchorA;// bodyA局部锚点
		glm::vec3 m_localAnchorB;// bodyB局部锚点
		float m_gamma;           // v/Ft
		float m_impulse;         // 冲量
		float m_lowerImpulse;    // 最小冲量
		float m_upperImpulse;    // 最大冲量

		// Solver temp
		int m_indexA;
		int m_indexB;
		glm::vec3 m_u;
		glm::vec3 m_rA;            // 经过rotationA旋转的向量原点a->锚点a
		glm::vec3 m_rB;            // 经过rotationB旋转的向量原点b->锚点b
		glm::vec3 m_localCenterA;
		glm::vec3 m_localCenterB;
		float m_currentLength;
		float m_invMassA;
		float m_invMassB;
		glm::vec3 m_invIA;
		glm::vec3 m_invIB;
		float m_softMass;
		float m_mass;
	};

}