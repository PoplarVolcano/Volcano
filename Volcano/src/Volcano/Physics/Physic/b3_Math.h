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
	// �任����ƽ�ƺ���ת�������ڱ�ʾ�ռܵ�λ�úͷ���
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

	// b3_Sweep������TOI������body/shape���˶�(motion)�� 
	// shape�������bodyԭ�㶨��ģ�bodyԭ����������Ĳ�һ��(coincide)��Ϊ��֧�ֶ���ѧ�����Ǳ����ֵ(interpolate)����λ�á�
	struct b3_Sweep
	{
		b3_Sweep() = default;

		// ��ȡ�ض�ʱ���transform��ֵ��
		// @param transform the output transform
		// @param beta is a factor in [0,1], where 0 indicates alphrotation0.
		void GetTransform(b3_Transform* transform, float beta) const;

		// ��ǰ�ƽ�sweep�������µĳ�ʼ״̬�� Advance the sweep forward, yielding a new initial state.
		// @param alpha the new initial time.
		void Advance(float alpha);

		// �����ȷ�Χ������-pi��pi
		void Normalize();

		glm::vec3 localCenter;	        // ���ľֲ����꣬body����fixture����massDataʱ����localCenter
		glm::vec3 center0, center;		// center world positions  center0����һ֡������������λ��
		glm::vec3 rotation0, rotation;	// world rotation          rotation0����һ֡������ת

		// ��ǰʱ�䲽����[0,1]��Χ�ڵķ��� Fraction of the current time step in the range [0,1]
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
	// ������a������,�ѱ�׼��
	glm::vec3 b3_Tangent(const glm::vec3& a);
	// max(low, min(a, high)) ��a������[low,high]��
	template <typename T>
	inline T b3_Clamp(T a, T low, T high)
	{
		return glm::max(low, glm::min(a, high));
	}

	template <typename T, size_t N>
	constexpr size_t getArrayLength(T(&)[N]) {
		return N;
	}


	// ���󽻻�ĳ2��
	void MatrixSwapRow(float** A, int i, int j, int n);
	// �����i��=�����i��-�����j��*a
	void MatrixMinusInner(float** A, float a, int i, int j, int n);
	// ��˹��Ԫ���������
	void MatrixInverse(float A[6][6], float A_inverse[6][6], int n);

	typedef std::vector<float> vec;
	typedef std::vector<vec> mat;

	mat inverse(const mat& a);

}