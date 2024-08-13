#pragma once
#include "Volcano/Core/Timestep.h"
#include "Volcano/Renderer/OrthographicCamera.h"
#include "Volcano/Core/Events/MouseEvent.h"
#include "Volcano/Core/Events/ApplicationEvent.h"

namespace Volcano {

	struct OrthographicCameraBouds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);//0,1280
		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);
		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera()const { return m_Camera; }

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; CalculateView(); }


		const OrthographicCameraBouds& GetBounds() const { return m_Bounds; }
	private:
		void CalculateView();
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		float m_AspectRatio;//���ݱ�
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;
		OrthographicCameraBouds m_Bounds;

		bool m_Rotation;

		glm::vec3 m_CameraPosition = { 0.0f,0.0f,0.0f };
		float m_CameraRotation = 0.0f;
		float m_CameraTranslationSpeed = 1.0f, m_CameraRotationSpeed = 180.0f;
	};
}