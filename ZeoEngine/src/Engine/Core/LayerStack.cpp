#include "ZEpch.h"
#include "Engine/Core/LayerStack.h"

namespace ZeoEngine {

	void LayerStack::PushLayer(Layer* layer)
	{
		m_Layers.emplace(m_Layers.cbegin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		m_Layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_Layers.cbegin(), m_Layers.cend(), layer);
		if (it != m_Layers.cend())
		{
			layer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto it = std::find(m_Layers.cbegin(), m_Layers.cend(), overlay);
		if (it != m_Layers.cend())
		{
			overlay->OnDetach();
			m_Layers.erase(it);
		}
	}

}
