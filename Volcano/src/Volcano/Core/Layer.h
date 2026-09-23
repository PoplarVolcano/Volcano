#pragma once
#pragma warning(push)
#pragma warning(disable : 4251)

#include <string>
#include "Volcano/Core/Events/Event.h"
#include "Volcano/Core/Core.h"

namespace Volcano
{
	struct LayerImpl;

	class VOL_API Layer {
	public:
		Layer(const std::string& debugName = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}//每层都可以拥有自己的UI窗口
		virtual void OnEvent(Event& event) {}
		inline const std::string& GetName() const;
	protected:
		std::unique_ptr<LayerImpl> m_Impl;
	};
}