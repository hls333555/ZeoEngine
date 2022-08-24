#pragma once

#include <unordered_map>

#include "Editors/EditorBase.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"

namespace ZeoEngine {

	class EditorBase;

	class EditorManager
	{
	private:
		EditorManager() = default;
		~EditorManager();
	public:
		EditorManager(const EditorManager&) = delete;
		EditorManager& operator=(const EditorManager&) = delete;

		static EditorManager& Get()
		{
			static EditorManager instance;
			return instance;
		}

		const auto& GetEditors() const { return m_Editors; }

		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		template<typename T>
		Ref<T> CreateEditor(std::string editorName)
		{
			if (GetEditor(editorName))
			{
				ZE_CORE_ERROR("Failed to create {0}! Editor already exists!", editorName);
				return {};
			}

			ZE_CORE_INFO("Creating editor: {0}", editorName);

			Ref<T> editor = CreateRef<T>(editorName);
			m_Editors.emplace(std::move(editorName), editor);
			editor->OnAttach();
			editor->Open();
			return editor;
		}

		template<typename T>
		Ref<EditorBase> OpenEditor(std::string editorName, const std::string& assetPath = {})
		{
			Ref<EditorBase> editor = GetEditor(editorName);
			if (!editor)
			{
				editor = CreateEditor<T>(std::move(editorName));
			}

			editor->Open();
			if (!assetPath.empty())
			{
				editor->LoadScene(assetPath);
			}
			return editor;	
		}

		template<typename EditorClass = EditorBase>
		Ref<EditorClass> GetEditor(const std::string& editorName) const
		{
			static_assert(std::is_base_of_v<EditorBase, EditorClass>, "EditorClass must be derived from 'EditorBase'!");

			const auto it = m_Editors.find(editorName);
			if (it == m_Editors.end()) return {};

			return std::static_pointer_cast<EditorClass>(it->second);
		}

		void RebuildLayoutForAllEditors();

	private:
		std::unordered_map<std::string, Ref<EditorBase>> m_Editors;
	};

}
