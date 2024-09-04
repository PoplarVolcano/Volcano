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

	static void Math_Quaternion(glm::vec3 Euler, glm::quat* outQuaternion)
	{
		*outQuaternion = glm::quat(Euler);
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

	void MathFRegister::MathF_RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(Math_Rotate);
		VOL_ADD_INTERNAL_CALL(Math_Quaternion);
		VOL_ADD_INTERNAL_CALL(Math_Inverse);
		VOL_ADD_INTERNAL_CALL(Math_Transpose);
		VOL_ADD_INTERNAL_CALL(Math_TRS);
		VOL_ADD_INTERNAL_CALL(Math_Perspective);
	}
}