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
		virtual void OnImGuiRender() {}//每层都可以拥有自己的UI窗口
		virtual void OnEvent(Event& event) {}
		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};

}