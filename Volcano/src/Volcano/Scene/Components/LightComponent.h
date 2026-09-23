#pragma once

#include "glm/glm.hpp"

namespace Volcano {

	struct LightComponent
	{
		// 方向光，点光源，聚光灯
		enum class LightType { DirectionalLight, PointLight, SpotLight };

		int shadowEnabled  = 0;

		LightType type     = LightType::DirectionalLight;
		glm::vec3 ambient  = glm::vec3(0.0f);		  // 环境光
		glm::vec3 diffuse  = glm::vec3(0.5f);		  // 漫反射，主光颜色与强度
		glm::vec3 specular = glm::vec3(1.0f);		  // 高光，镜面反射强度

		// 衰减系数 (Attenuation) ，衰减 = 1.0 / (Constant + Linear * distance + Quadratic * distance²)
		float constant    = 1.0f;					       // 常数项
		float linear      = 0.09f;					       // 一次项
		float quadratic   = 0.032f;				           // 二次项

		float cutoffAngle      = 12.5f;
		float outerCutoffAngle = 17.5f;

		float cutoff      = glm::cos(glm::radians(cutoffAngle));      // 聚光灯角度余弦值
		float outerCutoff = glm::cos(glm::radians(outerCutoffAngle));

		float radius = 50.0f;

		bool enabled = true;

		LightComponent() = default;
		LightComponent(const LightComponent&) = default;
	};
}