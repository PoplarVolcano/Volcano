#pragma once

#include "glm/glm.hpp"

namespace Volcano
{
	struct CircleRendererComponent
	{
		bool enabled = true;

		glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float thickness = 1.0f; // 厚度，1是实心圆
		float fade = 0.005f;    // 淡入效果

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

}