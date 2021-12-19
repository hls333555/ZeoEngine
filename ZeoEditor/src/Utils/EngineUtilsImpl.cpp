#include "Engine/Utils/EngineUtils.h"

#include "Core/EditorManager.h"
#include "Editors/EditorBase.h"

namespace ZeoEngine {

	Ref<SceneRenderer> EngineUtils::GetSceneRendererFromContext(Entity* entityContext)
	{
		if (!entityContext) return nullptr;

		for (const auto& [name, editor] : EditorManager::Get().GetEditors())
		{
			if (editor->GetScene() == entityContext->GetScene())
			{
				return editor->GetSceneRenderer();
			}
		}

		return nullptr;
	}

}
