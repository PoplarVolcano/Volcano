#pragma once

#include "glm/glm.hpp"

namespace Volcano
{

	// 圆碰撞体
	struct CircleCollider2DComponent
	{
		bool enabled = true;

		glm::vec2 offset = { 0.0f, 0.0f };
		float radius = 0.5f;

		float density = 1.0f;
		float friction = 0.5f;
		float restitution = 0.0f;
		float restitutionThreshold = 0.5f;

		void* runtimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

}