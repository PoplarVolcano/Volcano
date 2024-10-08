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

	/*
	class Light
	{
	public:
		enum class LightType { DirectionalLight, PointLight, SpotLight };
	public:
		Light();
		~Light() = default;

		LightType GetLightType() const { return m_LightType; }
		void SetLightType(LightType type) { m_LightType = type; }

		glm::vec3 GetAmbient()   const { return m_Ambient;     }
		glm::vec3 GetDiffuse()   const { return m_Diffuse;     }
		glm::vec3 GetSpecular()  const { return m_Specular;    }
		float GetConstant()      const { return m_Constant;    }
		float GetLinear()        const { return m_Linear;      }
		float GetQuadratic()     const { return m_Quadratic;   }
		float GetCutOff()        const { return m_CutOff;      }
		float GetOuterCutOff()   const { return m_OuterCutOff; }

		
		void SetAmbient(glm::vec3 ambient)     { m_Ambient     = ambient;     }
		void SetDiffuse(glm::vec3 diffuse)     { m_Diffuse     = diffuse;     }
		void SetSpecular(glm::vec3 specular)   { m_Specular    = specular;    }
		void SetConstant(float constant)       { m_Constant    = constant;    }
		void SetLinear(float linear)           { m_Linear      = linear;      }
		void SetQuadratic(float quadratic)     { m_Quadratic   = quadratic;   }
		void SetCutOff(float cutOff)           { m_CutOff      = cutOff;      }
		void SetOuterCutOff(float outerCutOff) { m_OuterCutOff = outerCutOff; }

	private:
		LightType m_LightType = LightType::DirectionalLight;

		glm::vec3 m_Ambient;
		glm::vec3 m_Diffuse;
		glm::vec3 m_Specular;
		float     m_Constant;
		float     m_Linear;
		float     m_Quadratic;
		float     m_CutOff;
		float     m_OuterCutOff;
	};
	*/
}