#pragma once

#include "Volcano/Core/Core.h"
#include "Volcano/Renderer/Camera.h"

namespace Volcano {

	class VOL_API SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Prespective = 0, Orthographic = 1 };
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		// 设置透视矩阵
		void SetPerspective(float verticalFov, float nearClip, float farClip);
		// 设置正交矩阵
		void SetOrthographic(float size, float nearClip, float farClip);
		// 设置视图尺寸
		void SetViewportSize(uint32_t width, uint32_t height);

		float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
		void SetPerspectiveVerticalFOV(float verticalFov) { m_PerspectiveFOV = verticalFov; RecalculateProjection(); }
		float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
		float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }

		float GetOrthographicSize() const { return m_OrthographicSize; }
		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
		float GetOrthographicNearClip() const { return m_OrthographicNear; }
		void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
		float GetOrthographicFarClip() const { return m_OrthographicFar; }
		void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

		virtual const float GetNearClip() const override
		{
			if (m_ProjectionType == ProjectionType::Prespective)
				return GetPerspectiveNearClip();
			else if (m_ProjectionType == ProjectionType::Orthographic)
				return GetOrthographicNearClip();
			else
				return 0;
		}
		virtual const float GetFarClip() const override
		{
			if (m_ProjectionType == ProjectionType::Prespective)
				return GetPerspectiveFarClip();
			else if (m_ProjectionType == ProjectionType::Orthographic)
				return GetOrthographicFarClip();
			else
				return 0;
		}

	private:
		// 重新计算投影矩阵
		void RecalculateProjection();
	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		float m_PerspectiveFOV = glm::radians(45.0f);
		float m_PerspectiveNear = 0.001f, m_PerspectiveFar = 1000.0f;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_AspectRatio = 0.0f;
	};
}