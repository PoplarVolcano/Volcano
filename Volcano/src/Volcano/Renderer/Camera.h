#pragma once

#include "Volcano/Core/Core.h"

#include <glm/glm.hpp>

namespace Volcano {

	struct CameraData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 position;
		float pad1;
		float nearClip;
		float farClip;
		float pad2[2];
	};

	class VOL_API Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: m_Projection(projection) 
		{}

		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }
		virtual const float GetNearClip() const  = 0;
		virtual const float GetFarClip() const = 0;
	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};
}