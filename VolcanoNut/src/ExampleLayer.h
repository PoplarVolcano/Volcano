#pragma once

#include "Volcano.h"
#include <Volcano/Renderer/OrthographicCameraController.h>
#include <ParticleSystem.h>

namespace Volcano {

	class ExampleLayer : public Layer 
	{
	public:
		ExampleLayer();
		virtual ~ExampleLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:
		OrthographicCameraController m_CameraController;

		Ref<VertexArray> m_VertexArray;
		Ref<Texture2D> m_Texture, m_AlterTexture;
		Ref<Texture2D> m_SpriteSheet;
		Ref<SubTexture2D> m_TextureStairs, m_TextureTree;

		struct ProfileResult
		{
			const char* Name;
			float Time;
		};
		std::vector<ProfileResult> m_ProfileResults;

		ParticleProps m_Particle;
		ParticleSystem m_ParticleSystem;

		uint32_t m_MapWidth, m_MapHeight;
		std::unordered_map<char, Ref<SubTexture2D>> s_TextureMap;
	};
}