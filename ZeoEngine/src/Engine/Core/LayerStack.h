#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Layer.h"

namespace ZeoEngine {

	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack();

		/** Layers are pushed to the first half of stack. */
		void PushLayer(Layer* layer);
		/** Overlays are pushed to the second half of stack so that they can always be rendered at last. */
		void PushOverlay(Layer* overlay);
		/** Remove the layer from stack but do not delete it. */
		void PopLayer(Layer* layer);
		/** Remove the overlay from stack but do not delete it. */
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
		std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

		std::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		std::vector<Layer*>::const_iterator end()	const { return m_Layers.end(); }
		std::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		std::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }

	private:
		std::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;

	};

}
