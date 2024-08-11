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
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	};
}