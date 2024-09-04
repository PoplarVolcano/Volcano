#pragma once

#include "UniformBuffer.h"
#include <glm/glm.hpp>

namespace Volcano {

	struct CameraData
	{
		glm::mat4 viewProjection;
	};
	static CameraData s_CameraBuffer;
	static Ref<UniformBuffer> s_CameraUniformBuffer;

	struct CameraPositionData
	{
		glm::vec3 CameraPosition;
	};
	static CameraPositionData s_CameraPositionBuffer;
	static Ref<UniformBuffer> s_CameraPositionUniformBuffer;

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