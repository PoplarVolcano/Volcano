#include "volpch.h"

#include "b3_Math.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

float glm::dot(const glm::vec3& vec)
{
	return glm::dot(vec, vec);
}

namespace Volcano {

	b3_Rotation::b3_Rotation(glm::vec3 rotation)
	{
		x = rotation.x;
		y = rotation.y;
		z = rotation.z;
		/*
		sx = sinf(rotation.x); cx = cosf(rotation.x);
		sy = sinf(rotation.y); cy = cosf(rotation.y);
		sz = sinf(rotation.z); cz = cosf(rotation.z);
		*/
	}

	void b3_Rotation::Set(glm::vec3 rotation)
	{
		x = rotation.x;
		y = rotation.y;
		z = rotation.z;

		/*
		sx = sinf(rotation.x); cx = cosf(rotation.x);
		sy = sinf(rotation.y); cy = cosf(rotation.y);
		sz = sinf(rotation.z); cz = cosf(rotation.z);
		*/
	}

	void b3_Rotation::SetIdentity()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		/*
		sx = 0.0f; cx = 1.0f;
		sy = 0.0f; cy = 1.0f;
		sz = 0.0f; cz = 1.0f;
		*/
	}

	glm::vec3 b3_Rotation::GetRotation() const
	{
		return { x, y, z };
		//return { atan2f(sx, cx), atan2f(sy, cy), atan2f(sz, cz) };
	}

	glm::quat b3_Rotation::GetQuat() const
	{
		return glm::quat({ x, y, z });
	}

	/*
	glm::quat b3_Rotation::GetRotationMatrix() const
	{
		return glm::mat3(
			{  cy * cz + sy * sx * sz,   sz * cx, -sy * cx + cy * sx * sz },
			{ -cy * sz + sy * sx * cz,   cz * cx,  sz * sy + cy * sx * cz },
			{  sy * cx,                 -sx,       cy * cx                });
	}
	*/



	void b3_Transform::SetIdentity()
	{
		position = { 0.0f, 0.0f, 0.0f };
		rotation.SetIdentity();
	}

	void b3_Transform::Set(const glm::vec3& position, glm::vec3& rotation)
	{
		this->position = position;
		this->rotation.Set(rotation);
	}

	glm::mat4 b3_Transform::Transform() const
	{
		return glm::translate(glm::mat4(1.0f), position) * glm::toMat4(rotation.GetQuat());
	}




	glm::vec3 b3_Multiply(const b3_Rotation& q, const glm::vec3& v)
	{
		return q.GetQuat() * v;
	}

	glm::vec3 b3_MultiplyT(const b3_Rotation& q, const glm::vec3& v)
	{
		return (-q.GetQuat()) * v;
	}

	b3_Rotation b3_Multiply(const b3_Rotation& q, const b3_Rotation& r)
	{
		b3_Rotation qr;
		qr.Set(r.GetRotation() + q.GetRotation());
		return qr;
	}

	b3_Rotation b3_MultiplyT(const b3_Rotation& q, const b3_Rotation& r)
	{
		b3_Rotation qr;
		qr.Set(r.GetRotation() - q.GetRotation());
		return qr;
	}
	glm::vec3 b3_Multiply(const b3_Transform& T, const glm::vec3& v)
	{
		return T.rotation.GetQuat() * v + T.position;
	}
	glm::vec3 b3_MultiplyT(const b3_Transform& T, const glm::vec3& v)
	{
		return (-T.rotation.GetQuat()) * (v - T.position);
	}
	b3_Transform b3_Multiply(const b3_Transform& A, const b3_Transform& B)
	{
		b3_Transform C;
		C.rotation = b3_Multiply(A.rotation, B.rotation);
		C.position = b3_Multiply(A.rotation, B.position) + A.position;
		return C;
	}
	b3_Transform b3_MultiplyT(const b3_Transform& A, const b3_Transform& B)
	{
		b3_Transform C;
		C.rotation = b3_MultiplyT(A.rotation, B.rotation);
		C.position = b3_MultiplyT(A.rotation, B.position - A.position);
		return C;
	}

	glm::vec3 b3_Cross(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::cross(a, b);
	}

	glm::vec3 b3_Tangent(const glm::vec3& a)
	{
		// 令切线b.x=a1x+b1y+c1z,b.y=a2x+b2y+c2z,b.z=a3x+b3y+c3z
		// b·a = 0 
		// => a.x*b.x+a.y*b.y+a.z*b.z=0 
		// => b1+a2=0,c1+a3=0,c2+b3=0 
		// => 取b1=i,c1=j,c2=k 有a2=-i,a3=-j,b3=-k
		// => b = { iy+jz, -ix+kz, -jx-ky };
		glm::vec3 ijk = { 1.0f, 1.0f, 1.0f };
		return glm::normalize(glm::vec3(ijk.x * a.y + ijk.y * a.z, -ijk.x * a.x + ijk.z * a.z, -ijk.y * a.x - ijk.z * a.y));
	}

	glm::vec3 b3_TangentEdge(const glm::vec3& a)
	{
		// 令切线b.x=a1yz+b1xz+c1xy,b.y=a2yz+b2xz+c2xy,b.z=a3yz+b3xz+c3xy
		// b·a = 0 
		// => a.x*b.x+a.y*b.y+a.z*b.z=0 
		// => a1+b2+c3=0
		// => 取a1=1,b2=1,则c3=-2
		// => b = { a.y * a.z, a.x * a.z, -2 * a.x * a.y}
		return glm::normalize(glm::vec3(a.y * a.z, a.x * a.z, - 2 * a.x * a.y));
	}

	void b3_Sweep::GetTransform(b3_Transform* transform, float beta) const
	{
		transform->position = (1.0f - beta) * center0 + beta * center;
		glm::vec3 rotation = (1.0f - beta) * rotation0 + beta * rotation;
		transform->rotation.Set(rotation);

		// Shift to origin
		transform->position -= b3_Multiply(transform->rotation, localCenter);
	}

	void b3_Sweep::Advance(float alpha)
	{
		assert(alpha0 < 1.0f);
		float beta = (alpha - alpha0) / (1.0f - alpha0);
		center0 += beta * (center - center0);
		rotation0 += beta * (rotation - rotation0);
		alpha0 = alpha;
	}

	void b3_Sweep::Normalize()
	{
		float twoPi = 2.0f * b3_PI;
		glm::vec3 d = twoPi * glm::vec3(floorf(rotation0.x / twoPi), floorf(rotation0.y / twoPi), floorf(rotation0.z / twoPi));
		rotation0 -= d;
		rotation -= d;
	}

	void MatrixSwapRow(float** A, int i, int j, int n)
	{
		float temp;
		for (int k = 0; k < n; k++)
		{
			temp = A[i][k];
			A[i][k] = A[j][k];
			A[j][k] = temp;
		}
	}

	void MatrixMinusInner(float** A, float a, int i, int j, int n)
	{
		for (int k = 0;k < n;k++)
		{
			A[i][k] -= a * A[j][k];
		}
	}

	void MatrixInverse(float A[6][6], float A_inverse[6][6], int n)
	{
		float** A_E = new float* [2 * n];
		//构建增广矩阵
		for (int i = 0;i < n;i++)
		{
			A_E[i] = new float[n * 2];
			for (int j = 0;j < n * 2;j++)
			{
				if (j < n)
				{
					A_E[i][j] = A[i][j];
				}
				else if ((j - n) == i) {
					A_E[i][j] = 1;
				}
				else {
					A_E[i][j] = 0;
				}
			}
		}
		//首先将矩阵化为上三角矩阵
		for (int i = 0;i < n;i++)
		{
			if (A_E[i][i] == 0)
			{
				for (int k = i + 1;k < n;k++)
				{
					if (A_E[k][i] != 0)
					{
						MatrixSwapRow(A_E, i, k, n * 2);
						break;
					}
				}
			}
			for (int j = i + 1;j < n;j++)
			{
				MatrixMinusInner(A_E, A_E[j][i] / A_E[i][i], j, i, 2 * n);
			}
		}
		//打印上三角矩阵进行检验
		// for(int i=0;i<n;i++)
		// {
		//     for(int j=0;j<n*2;j++)
		//     {
		//         std::cout<<A_E[i][j]<<" ";
		//     }
		//     std::cout<<std::endl;
		// }
		//判断矩阵是否可逆
		for (int i = 0; i < n; i++)
		{
			if (A_E[i][i] == 0)
			{
				VOL_ERROR("矩阵不可逆");
				//std::cout << "矩阵不可逆" << std::endl;
				return;
				//exit(0);
			}
		}
		//将上三角转换为对角矩阵
		for (int j = 1;j < n;j++)
		{
			for (int i = 0;i < j;i++)
			{
				MatrixMinusInner(A_E, A_E[i][j] / A_E[j][j], i, j, 2 * n);
			}
		}
		for (int i = 0;i < n;i++)
		{
			for (int j = n;j < 2 * n;j++)
			{
				A_inverse[i][j - n] = A_E[i][j] / A_E[i][i];
			}
		}
		// for(int i=0;i<n;i++)
		// {
		//     for(int j=0;j<n;j++)
		//     {
		//         std::cout<<A_inverse[i][j]<<" ";
		//     }
		//     std::cout<<std::endl;
		// }

	}

	mat inverse(const mat& a) 
	{
		int n = a.size();
		mat b(n, vec(n * 2));

		// 构建增广矩阵
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				b[i][j] = a[i][j];
			}
			b[i][n + i] = 1;
		}

		// 高斯-若尔当消元法
		for (int i = 0; i < n; ++i) {
			int max_r = i;
			for (int j = i + 1; j < n; ++j) {
				if (abs(b[j][i]) > abs(b[max_r][i])) {
					max_r = j;
				}
			}
			if (max_r != i) {
				for (int k = 0; k < n * 2; ++k) {
					std::swap(b[i][k], b[max_r][k]);
				}
			}
			for (int j = i + 1; j < n; ++j) {
				if (b[i][i] == 0)
					continue;
				float c = b[j][i] / b[i][i];
				for (int k = i; k < n * 2; ++k) {
					b[j][k] -= c * b[i][k];
				}
			}
		}

		// 求逆矩阵
		mat inv(n, vec(n));
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				inv[i][j] = b[i][n + j];
			}
		}

		return inv;
	}

}