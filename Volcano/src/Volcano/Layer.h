#pragma once

#include "Volcano/Core.h"
#include "Volcano/Core/Timestep.h"
#include "Volcano/Events/Event.h"

namespace Volcano {

	class VOLCANO_API Layer {
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}//ÿ�㶼����ӵ���Լ���UI����
		virtual void OnEvent(Event& event) {}
		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};

}