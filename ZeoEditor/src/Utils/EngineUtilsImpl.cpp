#include "Engine/Utils/EngineUtils.h"

#include "Core/EditorManager.h"
#include "Editors/EditorBase.h"

namespace ZeoEngine {

	Ref<SceneRenderer> EngineUtils::GetSceneRendererFromContext(const Ref<Scene>& sceneContext)
	{
		for (const auto& [name, editor] : EditorManager::Get().GetEditors())
		{
			if (editor->GetScene()->GetContext() == sceneContext->GetContext())
			{
				return editor->GetSceneRenderer();
			}
		}

		return nullptr;
	}

}
