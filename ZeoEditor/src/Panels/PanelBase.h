#pragma once

#include "Core/EditorTypes.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorBase;
	class PanelBase;

	class PanelBase
	{
		friend class PanelManager;

	private:
		PanelBase() = delete;
		PanelBase(PanelType type, EditorBase* owningEditor);
	protected:
		virtual ~PanelBase() = default;

	public:
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		template<typename T = EditorBase>
		T* GetOwningEditor()
		{
			if constexpr (std::is_same<T, EditorBase>::value)
			{
				return m_OwningEditor;
			}
			else
			{
				return dynamic_cast<T*>(m_OwningEditor);
			}
		}

		bool* GetShowPtr() { return &m_bShow; }
		bool IsPanelFocused() const { return m_bIsPanelFocused; }
		bool IsPanelHovered() const { return m_bIsPanelHovered; }

		void Open();

	private:
		virtual void ProcessUpdate(DeltaTime dt) {}
		virtual void ProcessRender() = 0;
		virtual void ProcessEvent(Event& e) {}

	protected:
		PanelSpec m_PanelSpec;

	private:
		PanelType m_PanelType;
		EditorBase* m_OwningEditor = nullptr;

		bool m_bShow = true;
		bool m_bIsPanelFocused = false, m_bIsPanelHovered = false;
	};

}
