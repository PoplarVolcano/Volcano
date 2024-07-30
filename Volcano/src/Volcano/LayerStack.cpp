#include "volpch.h"
#include "LayerStack.h"

namespace Volcano {

	LayerStack::~LayerStack() {
		for (Layer* layer : m_Layers){
			delete layer;
		}
	}

	//普通push在队列最左（查找时候性能更优）
	void LayerStack::PushLayer(Layer* layer) {
		// emplace(放置，安放)在vector容器指定位置之前生成一个新的元素。返回生成元素的位置
		// 生成 1 2 3，vector是 1 2 3
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;

	}
	//在最右生成
	void LayerStack::PushOverlay(Layer* overlay) {
		m_Layers.emplace_back(overlay);
	}
	//查找
	void LayerStack::PopLayer(Layer* layer) {
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end()) {
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}
	void LayerStack::PopOverlay(Layer* overlay) {
		auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
		if (it != m_Layers.end()){
			m_Layers.erase(it);
		}
	}
}