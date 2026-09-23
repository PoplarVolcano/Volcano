#pragma once

#include "Volcano/Renderer/Texture.h"
#include "Volcano/Renderer/Material.h"

namespace Volcano
{

	struct MeshRendererComponent
	{
		enum class MeshRendererFlags : uint32_t
		{
			None = 0,
			Outline = 1 << 0,            // 第一位：启用轮廓
			Explosion = 1 << 1,          // 第二位：爆破效果
			NormalVisualization = 1 << 2 // 第三位：法向量可视化
		};

		MaterialLibraryKey materialLibraryKey;

		bool enabled = true;

		glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 uvRect{ 0.0f, 0.0f, 1.0f, 1.0f };
		float parallaxScale = 0.0f;
		float tilingFactor = 1.0f;

		// Circle
		float thickness = 1.0f; // 厚度，1是实心圆
		float fade = 0.005f;    // 淡入效果

		uint32_t flags = 0;

		glm::vec4 outlineColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec3 outlineScale{ 1.1f, 1.1f, 1.1f};

		float explosionOffset = 0.0f;

		glm::vec4 normalVisualizationColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		float normalVisualizationLength = 1.0f;
		int normalVisualizationIndex1 = -1;
		int normalVisualizationIndex2 = -1;

		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent&) = default;

		inline bool HasFlag(uint32_t flags, MeshRendererFlags flag)
		{
			return (flags & static_cast<uint32_t>(flag)) != 0;
		}
	};

}