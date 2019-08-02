#pragma once

#include "Core.h"
#include "Layer.h"

namespace HBestEngine
{
	class HBE_API LayerStack
	{
	public:
		LayerStack();
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

	private:
		std::vector<Layer*> m_Layers;
		std::vector<Layer*>::iterator m_LayerInsert;

	};

}
