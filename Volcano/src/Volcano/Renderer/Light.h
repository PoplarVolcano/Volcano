#pragma once
#include "Volcano/Core/Base.h"
#include "glm/glm.hpp"
#include "UniformBuffer.h"

namespace Volcano {


	// 定向光（平行光）
	struct DirectionalLight
	{
		glm::vec3 direction;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
	};
	static DirectionalLight s_DirectionalLightBuffer;
	static Ref<UniformBuffer> s_DirectionalLightUniformBuffer;

	// 点光源
	struct PointLight
	{
		glm::vec3 position;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		float constant;
		float linear;
		float quadratic;
	};
	static PointLight s_PointLightBuffer;
	static Ref<UniformBuffer> s_PointLightUniformBuffer;

	// 聚光(手电筒Flashlight)
	struct SpotLight
	{
		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		float constant;
		float linear;
		float quadratic;
		float cutOff;
		float outerCutOff;

	};
	static SpotLight s_SpotLightBuffer;
	static Ref<UniformBuffer> s_SpotLightUniformBuffer;

	struct Material
	{
		float shininess;
	};
	static Material s_MaterialBuffer;
	static Ref<UniformBuffer> s_MaterialUniformBuffer;

}