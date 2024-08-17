#pragma once

#include "volpch.h"
#include "Volcano/Core/Layer.h"

namespace Volcano {

	class ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		ImGuiLayer(const std::string& name);
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }
		
		void SetDarkThemColors();
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};
}