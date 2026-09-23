#pragma once

#include "glm/glm.hpp"

namespace Volcano
{

	static const uint32_t s_MaxInstanceData = 65536u;

	struct InstanceData
	{
		glm::mat4 transform = glm::mat4(1.0f);
	};

	struct InstanceDataMaterial
	{
		glm::mat4 normalTransform = glm::mat4(1.0f);
		glm::vec4 color           = glm::vec4(1.0f);
		glm::vec4 uvRect          = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // offsetX, offsetY, scaleX, scaleY
		float     parallaxScale   = 0.0f;
		float     tilingFactor    = 1.0f;  //控制纹理在物体表面上的重复次数，通常取1
		uint32_t  materialIndex   = 0;    // 指向材质表的索引
		float     pad;
	};

	struct InstanceDataExplosion
	{
		float offset = 0.0f;
	};

	struct InstanceDataOutline
	{
		glm::vec4 color = glm::vec4(1.0f);
	};

	struct InstanceDataNormalVisualization
	{
		glm::vec4 color = glm::vec4(1.0f);
		float     length = 1.0f;
		int       index1 = -1;
		int       index2 = -1;
		float     pad;
	};

	struct InstanceDataEntityID
	{
		int entityID = -1;
	};

	struct InstanceDataLightingMode
	{
		int lightingMode = 0;
	};

	struct InstanceDataTotal
	{
		InstanceData instanceData;
		InstanceDataMaterial material;
		InstanceDataExplosion explosion;
		InstanceDataOutline outline;
		InstanceDataNormalVisualization normalVisualization;
		InstanceDataEntityID entityID;
		InstanceDataLightingMode lightingMode;
	};

}