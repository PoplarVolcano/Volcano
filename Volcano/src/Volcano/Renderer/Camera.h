#pragma once

#include "UniformBuffer.h"
#include <glm/glm.hpp>

namespace Volcano {

	struct CameraPositionData
	{
		glm::vec3 CameraPosition;
	};
	static CameraPositionData s_CameraPositionBuffer;

	struct CameraDataBuffer
	{
		glm::mat4 View;
		glm::mat4 Projection;
	};
	static CameraDataBuffer s_CameraDataBuffer;

	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: m_Projection(projection) {}
		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }
	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};
}