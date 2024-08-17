#pragma once

#include "Volcano.h"
#include <Volcano/Renderer/OrthographicCameraController.h>
#include <ParticleSystem.h>

namespace Volcano {

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:
		OrthographicCameraController m_CameraController;

		Ref<Texture2D> m_Texture;

		struct ProfileResult
		{
			const char* Name;
			float Time;
		};
		std::vector<ProfileResult> m_ProfileResults;

	};
}