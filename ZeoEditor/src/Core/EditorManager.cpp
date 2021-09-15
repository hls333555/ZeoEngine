#include "Core/EditorManager.h"

#include "Editors/SceneEditor.h"
#include "Editors/ParticleEditor.h"
#include "Utils/EditorUtils.h"
#include "Dockspaces/DockspaceBase.h"

namespace ZeoEngine {

	EditorManager::~EditorManager()
	{
		for (auto& [type, editor] : m_Editors)
		{
			editor->OnDetach();
			delete editor;
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

	EditorBase* EditorManager::CreateEditor(EditorType type)
	{
		const char* editorName = EditorUtils::GetEditorName(type);
		ZE_CORE_INFO("Creating editor: {0}", editorName);
		EditorBase* editor = nullptr;
		switch (type)
		{
			case EditorType::SceneEditor:		editor = new SceneEditor(type); break;
			case EditorType::ParticleEditor:	editor = new ParticleEditor(type); break;
			default:							ZE_CORE_ASSERT(false, "Unknown EditorType!"); return nullptr;
		}

		m_Editors.emplace(type, editor);
		editor->OnAttach(); // Call this after emplacement as scene class should be able to get editor instance on construction
		return editor;
	}

	EditorBase* EditorManager::OpenEditor(EditorType type)
	{
		EditorBase* editor = GetEditor(type);
		if (editor)
		{
			editor->Open();
			return editor;
		}

		return CreateEditor(type);
	}

	EditorBase* EditorManager::GetEditor(EditorType type)
	{
		auto result = m_Editors.find(type);
		return result == m_Editors.end() ? nullptr : result->second;
	}

	void EditorManager::RebuildLayoutForAllEditors()
	{
		for (auto& [type, editor] : m_Editors)
		{
			editor->GetDockspace()->RebuildDockLayout();
		}
	}

}
