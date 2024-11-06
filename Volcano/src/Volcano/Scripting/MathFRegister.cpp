#include "volpch.h"
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Volcano/Scripting/MathFRegister.h"

#include "mono/metadata/object.h"
namespace Volcano {

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.MathF::" #Name, Name)

	static void Math_Rotate(glm::quat q, glm::vec3 v, glm::vec3* outRotate)
	{
		*outRotate = glm::rotate(q, v);
	}

	static void Math_QuaternionFromEuler(glm::vec3 euler, glm::quat* outQuaternion)
	{
		*outQuaternion = glm::quat(euler);
	}

	static void Math_QuaternionFromToRotation(glm::vec3 fromDirection, glm::vec3 toDirection, glm::quat* outQuaternion)
	{
		glm::vec3 fromDir = glm::normalize(fromDirection);
		glm::vec3 toDir = glm::normalize(toDirection);
		glm::vec3 axis = glm::cross(fromDir, toDir);
		float angle = acos(glm::dot(fromDir, toDir));

		// 如果两个方向向量相同或者相反，则不需要旋转
		if (angle == 0.0f) {
			*outQuaternion = glm::quat();
			return;
		}

		*outQuaternion = glm::rotate(glm::mat4(1.0f), angle, axis);
	}

	static void Math_QuaternionLookRotation(glm::vec3 forward, glm::vec3 upwards, glm::quat* outQuaternion)
	{
		*outQuaternion = glm::lookAt(glm::vec3(0.0f), forward, upwards);
	}

	static void Math_QuaternionInverse(glm::quat quaternion, glm::quat* outQuaternion)
	{
		*outQuaternion = glm::inverse(quaternion);
	}

	static void Math_EulerFromQuaternion(glm::quat quaternion, glm::vec3* outEuler)
	{
		*outEuler = glm::eulerAngles(quaternion);
	}

	static void Math_Inverse(glm::mat4 mat4, glm::mat4* outMat4)
	{
		*outMat4 = glm::inverse(mat4);
	}

	static void Math_Transpose(glm::mat4 mat4, glm::mat4* outMat4)
	{
		*outMat4 = glm::transpose(mat4);
	}

	static void Math_TRS(glm::vec3 translation, glm::quat rotation, glm::vec3 scale, glm::mat4* outTransform)
	{
		*outTransform = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
	}

	static void Math_Perspective(float fov, float aspect, float zNear, float zFar, glm::mat4* perspective)
	{
		*perspective = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
	}

	static void Math_Cross(glm::vec3 v1, glm::vec3 v2, glm::vec3* cross)
	{
		*cross = glm::cross(v1, v2);
	}

	static void Math_LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up, glm::mat4* lookAt)
	{
		*lookAt = glm::lookAt(eye, center, up);
	}

	void MathFRegister::MathF_RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(Math_Rotate);
		VOL_ADD_INTERNAL_CALL(Math_QuaternionFromEuler);
		VOL_ADD_INTERNAL_CALL(Math_QuaternionFromToRotation);
		VOL_ADD_INTERNAL_CALL(Math_QuaternionLookRotation);
		VOL_ADD_INTERNAL_CALL(Math_QuaternionInverse);
		VOL_ADD_INTERNAL_CALL(Math_EulerFromQuaternion);
		VOL_ADD_INTERNAL_CALL(Math_Inverse);
		VOL_ADD_INTERNAL_CALL(Math_Transpose);
		VOL_ADD_INTERNAL_CALL(Math_TRS);
		VOL_ADD_INTERNAL_CALL(Math_Perspective);
		VOL_ADD_INTERNAL_CALL(Math_Cross);
		VOL_ADD_INTERNAL_CALL(Math_LookAt);
	}
}