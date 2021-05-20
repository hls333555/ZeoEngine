#pragma once

#include <unordered_map>

#include "Engine/Core/Core.h"
#include "Core/EditorTypes.h"
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

		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e);

		EditorBase* CreateEditor(EditorType type);
		EditorBase* OpenEditor(EditorType type);
		EditorBase* GetEditor(EditorType type);

		void RebuildLayoutForAllEditors();

	private:
		std::unordered_map<EditorType, EditorBase*> m_Editors;
	};

}
