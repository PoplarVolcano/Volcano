#pragma once

#include "Volcano/Core/UUID.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

namespace Volcano {

	struct IDComponent
	{
		Volcano::UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const Volcano::UUID& uuid)
			: ID(uuid) {
		}
	};

	struct TagComponent
	{
		std::string tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: tag(tag) {
		}
	};

	// 空组件会被视为一个空结构，被视为特殊情况，并且它不会编译
	struct TransformComponent
	{
		glm::vec3 translation{ 0.0f, 0.0f, 0.0f };
		// GLM 的构造函数参数顺序是 (w, x, y, z)，无旋转四元数是(0,0,0,1)，构造时w在前(1,0,0,0)
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

		// 角度值欧拉角
		glm::vec3 inspectorEulerHint{ 0.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: translation(translation) {
		}

		void SetRotation(const glm::quat& q)
		{
			rotation = q;
			inspectorEulerHint = glm::degrees(glm::eulerAngles(q));   // 同步 Hint
		}

		// 局部变换矩阵（相对于父级）
		glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), translation)
				* glm::toMat4(rotation)
				* glm::scale(glm::mat4(1.0f), scale);
		}

		// 法线矩阵（用于光照计算，需要在世界空间下使用）
		glm::mat3 GetNormalTransform() const
		{
			return  glm::mat3(transpose(inverse(GetTransform())));
		}
	};
}