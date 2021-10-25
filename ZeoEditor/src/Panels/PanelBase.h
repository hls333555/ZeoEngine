#pragma once

#include "Core/EditorTypes.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorBase;

	class PanelBase
	{
	public:
		PanelBase() = delete;
		PanelBase(const char* panelName, const Ref<EditorBase>& contextEditor);
	protected:
		virtual ~PanelBase() = default;

	public:
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		const std::string& GetPanelName() const { return m_PanelName; }

		const Ref<EditorBase>& GetContextEditor() const { return m_ContextEditor; }
		template<typename T>
		Ref<T> GetContextEditor() const
		{
			return std::dynamic_pointer_cast<T>(m_ContextEditor);
		}

		bool* GetShowPtr() { return &m_bShow; }
		bool IsPanelFocused() const { return m_bIsPanelFocused; }
		bool IsPanelHovered() const { return m_bIsPanelHovered; }

		void Open();
		void Close();

	private:
		virtual void ProcessUpdate(DeltaTime dt) {}
		virtual void ProcessRender() = 0;
		virtual void ProcessEvent(Event& e) {}

		virtual std::string GetPanelTitle() const { return m_PanelName; }

		virtual void OnPanelOpen() {}

	protected:
		PanelSpec m_PanelSpec;

	private:
		std::string m_PanelName;
		Ref<EditorBase> m_ContextEditor;

		bool m_bShow = true;
		bool m_bIsPanelFocused = false, m_bIsPanelHovered = false;
	};

}
