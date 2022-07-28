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
		Ref<EditorBase> OpenEditor(std::string editorName, const std::filesystem::path& assetPath = {})
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

		template<typename T = EditorBase>
		Ref<T> GetEditor(const std::string& editorName) const
		{
			const auto it = m_Editors.find(editorName);
			if (it == m_Editors.end()) return {};

			Ref<EditorBase> editor = it->second;
			if constexpr (std::is_same_v<T, EditorBase>)
			{
				return editor;
			}
			else
			{
				return std::dynamic_pointer_cast<T>(editor);
			}
		}

		void RebuildLayoutForAllEditors();

	private:
		std::unordered_map<std::string, Ref<EditorBase>> m_Editors;
	};

}
