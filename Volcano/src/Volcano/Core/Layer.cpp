#include "volpch.h"
#include "Layer.h"

namespace Volcano
{
	struct LayerImpl
	{
		std::string m_DebugName;
	};

	Layer::Layer(const std::string& debugName)
		:m_Impl(std::make_unique<LayerImpl>())
	{
		m_Impl->m_DebugName = debugName;
	}

	Layer::~Layer()
	{
	}

	inline const std::string& Layer::GetName() const
	{
		return m_Impl->m_DebugName;
	}

}