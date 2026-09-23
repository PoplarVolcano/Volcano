#pragma once
#include "Camera.h"
#include "Volcano/Core/Events/MouseEvent.h"
#include "Volcano/Core/Events/ApplicationEvent.h"

namespace Volcano {

	class VOL_API EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		/*
		fov(Field of View - 视场角)：摄像机的视野宽度，通常指垂直视场角（Vertical FOV）
		单位：角度（degrees）
		值越大 → 看得越广 → 物体显得更小（广角效果）
		值越小 → 看得越窄 → 物体显得更大（长焦效果）
		aspectRatio (宽高比)：视口的宽度 / 高度
		典型值：16.0f/9.0f = 1.778（宽屏）4.0f/3.0f = 1.333（传统）
		nearClip (近裁剪面)：摄像机能看到物体的最近距离
		farClip (远裁剪面)：摄像机能看到物体的最远距离

		glm::perspective(float fovy, float aspect, float zNear, float zFar);
		创建透视投影矩阵,其中fovy读取弧度
		*/
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate();
		void OnEvent(Event& e);

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * glm::inverse(m_ViewMatrix); }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		glm::vec3 GetPosition() { return m_Position; }
		glm::quat GetOrientation() const; // 定向(orientation)。glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f))
		glm::vec3 GetRotation() const;    // glm::vec3(-m_Pitch, -m_Yaw, 0.0f)

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		virtual const float GetNearClip() const override { return m_NearClip; }
		virtual const float GetFarClip() const override { return m_FarClip; }
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnWindowResizeEvent(WindowResizeEvent& e);

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		// 1280 / 720 = 1.777...
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };   // 相机位置
		glm::vec3 m_FocalPoint = { 0.0f, 1.0f, 0.0f }; // 视点位置

		glm::vec2 m_InitialMousePosition;

		float m_Distance;
		/*
		欧拉角（Euler Angles）：
		俯仰角（Pitch）：绕 X 轴旋转，控制上下看，向上看为正
		偏航角（Yaw）：绕 Y 轴旋转，控制左右看，向右转为正
		翻滚角（Roll）：绕 Z 轴旋转，控制倾斜，常用于飞机、船只，在 FPS 相机中通常固定为 0
		*/
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};
}