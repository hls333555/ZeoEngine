#pragma once

#include "Core/EditorTypes.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class DockspaceBase;

	class PanelBase
	{
		friend class PanelManager;

	private:
		PanelBase() = delete;
		PanelBase(const PanelSpec& spec, DockspaceBase* context);
	protected:
		virtual ~PanelBase() = default;

	public:
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		template<typename T = DockspaceBase>
		T* GetContext() { return dynamic_cast<T*>(m_Context); }

		bool* GetShowPtr() { return &m_bShow; }
		bool IsPanelFocused() const { return m_bIsPanelFocused; }
		bool IsPanelHovered() const { return m_bIsPanelHovered; }

	private:
		virtual void ProcessUpdate(DeltaTime dt) {}
		virtual void ProcessRender() = 0;
		virtual void ProcessEvent(Event& e) {}

	private:
		PanelSpec m_PanelSpec;
		DockspaceBase* m_Context = nullptr;

		bool m_bShow = true;
		bool m_bIsPanelFocused = false, m_bIsPanelHovered = false;
	};

}
