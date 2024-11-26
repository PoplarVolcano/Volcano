#pragma once

#include "b3_Common.h"
#include "glm/glm.hpp"

namespace glm {

	float dot(const glm::vec3& vec);
}

namespace Volcano {

	// Rotation
	struct b3_Rotation
	{
		b3_Rotation() = default;

		explicit b3_Rotation(glm::vec3 rotation);

		void Set(glm::vec3 rotation);
		void SetIdentity();

		glm::vec3 GetRotation() const;
		glm::quat GetQuat() const;


		/*
		// Get the x-axis
		b3_Vec2 GetXAxis() const
		{
			return b3_Vec2(c, s);
		}

		// Get the u-axis
		b3_Vec2 GetYAxis() const
		{
			return b3_Vec2(-s, c);
		}
		*/
		// Sine and cosine
		//float sx, cx, sy, cy, sz, cz;
		float x, y, z;
	};

	// A transform contains translation and rotation. It is used to represent the position and orientation of rigid frames.
	// 变换包含平移和旋转。它用于表示刚架的位置和方向。
	struct b3_Transform
	{
		b3_Transform() = default;

		b3_Transform(const glm::vec3& position, const b3_Rotation& rotation) : position(position), rotation(rotation) {}

		void SetIdentity();
		void Set(const glm::vec3& position, glm::vec3& rotation);
		glm::mat4 Transform() const;
		glm::vec3 position;
		b3_Rotation rotation;
	};

	// b3_Sweep描述了TOI计算中body/shape的运动(motion)。 
	// shape是相对于body原点定义的，body原点可能与质心不一致(coincide)。为了支持动力学，我们必须插值(interpolate)质心位置。
	struct b3_Sweep
	{
		b3_Sweep() = default;

		// 获取特定时间的transform插值。
		// @param transform the output transform
		// @param beta is a factor in [0,1], where 0 indicates alphrotation0.
		void GetTransform(b3_Transform* transform, float beta) const;

		// 向前推进sweep，产生新的初始状态。 Advance the sweep forward, yielding a new initial state.
		// @param alpha the new initial time.
		void Advance(float alpha);

		// 将弧度范围限制在-pi到pi
		void Normalize();

		glm::vec3 localCenter;	        // 质心局部坐标，body创建fixture更新massData时更新localCenter
		glm::vec3 center0, center;		// center world positions  center0：上一帧质心世界坐标位置
		glm::vec3 rotation0, rotation;	// world rotation          rotation0：上一帧物体旋转

		// 当前时间步长在[0,1]范围内的分数 Fraction of the current time step in the range [0,1]
		// center0 and rotation0 are the positions at alpha0.
		float alpha0;
	};

	glm::vec3 b3_Multiply(const b3_Rotation& q, const glm::vec3& v);
	glm::vec3 b3_MultiplyT(const b3_Rotation& q, const glm::vec3& v);
	b3_Rotation b3_Multiply(const b3_Rotation& q, const b3_Rotation& r);
	b3_Rotation b3_MultiplyT(const b3_Rotation& q, const b3_Rotation& r);
	glm::vec3 b3_Multiply(const b3_Transform& T, const glm::vec3& v);
	glm::vec3 b3_MultiplyT(const b3_Transform& T, const glm::vec3& v);
	b3_Transform b3_Multiply(const b3_Transform& A, const b3_Transform& B);
	b3_Transform b3_MultiplyT(const b3_Transform& A, const b3_Transform& B);
	glm::vec3 b3_Cross(const glm::vec3& a, const glm::vec3& b);
	// 求向量a的切线,已标准化
	glm::vec3 b3_Tangent(const glm::vec3& a);
	// max(low, min(a, high)) 将a限制在[low,high]内
	template <typename T>
	inline T b3_Clamp(T a, T low, T high)
	{
		return glm::max(low, glm::min(a, high));
	}

	template <typename T, size_t N>
	constexpr size_t getArrayLength(T(&)[N]) {
		return N;
	}


	// 矩阵交换某2行
	void MatrixSwapRow(float** A, int i, int j, int n);
	// 矩阵第i行=矩阵第i行-矩阵第j行*a
	void MatrixMinusInner(float** A, float a, int i, int j, int n);
	// 高斯消元法求逆矩阵
	void MatrixInverse(float A[6][6], float A_inverse[6][6], int n);

	typedef std::vector<float> vec;
	typedef std::vector<vec> mat;

	mat inverse(const mat& a);

}