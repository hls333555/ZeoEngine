#include "Core/EditorManager.h"

#include "Editors/EditorBase.h"
#include "EditorUIRenderers/EditorUIRendererBase.h"

namespace ZeoEngine {

	EditorManager::~EditorManager()
	{
		for (auto& [type, editor] : m_Editors)
		{
			editor->OnDetach();
		}
	}

	void EditorManager::OnUpdate(DeltaTime dt)
	{
		for (auto& [type, editor] : m_Editors)
		{
			editor->OnUpdate(dt);
		}
	}

	void EditorManager::OnImGuiRender()
	{
		for (auto& [type, editor] : m_Editors)
		{
			editor->OnImGuiRender();
		}
	}

	void EditorManager::OnEvent(Event& e)
	{
		for (auto& [type, editor] : m_Editors)
		{
			editor->OnEvent(e);
		}
	}

	void EditorManager::RebuildLayoutForAllEditors()
	{
		for (auto& [type, editor] : m_Editors)
		{
			editor->GetEditorUIRenderer()->RebuildDockLayout();
		}
	}

}
