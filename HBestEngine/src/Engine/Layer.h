#pragma once

#include "Core.h"
#include "Core/DeltaTime.h"
#include "Events/Event.h"

namespace HBestEngine
{
	/**
	 * Layers stored in an ordered list called layer stack not only determine what order thing's DRAWING,
	 * but also are applicable to EVENTS and some update logic.
	 *
	 * During the main game loop, each layer will be updated in the order that it is appeared in the layer stack
	 * and rendering can be done here.
	 * Since the layer stack is ordered,
	 * you can put layers on top of other layers which will turn draw order;
	 * also, it is very useful for overlays which are always at the end of the list so that they get rendered last,
	 * which is very important in the case of rendering a gizmo or some debug graphics which we want them to be always on top.
	 *
	 * As for events, the order will be reverse.
	 * Consider the case that you click the mouse on a button, the first layer that should receive that event
	 * is actually the last rendered layer containing the button,
	 * if you decide to handle the event and mark it "handled", 
	 * it does not need to get propagated further down the layer stack into the other layers such as the 3D game world.
	 *
	 * In general, the update process is going forwards through the list to render and update things and going backwards from the topmost to handle events.
	 * @see Run() and OnEvent() in Application.cpp
	 */
	class HBE_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		/** Called when the layer is pushed onto layer stack. */
		virtual void OnAttach() {}
		/** Called when the layer is popped from layer stack. */
		virtual void OnDetach() {}
		virtual void OnUpdate(DeltaTime dt) {}
		virtual void OnImGuiRender() {}
		/** Called when an event gets sent to the layer. */
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};

}
