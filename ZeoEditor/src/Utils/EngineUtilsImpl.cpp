#include "Engine/Utils/EngineUtils.h"

#include "Core/EditorManager.h"
#include "Editors/EditorBase.h"

namespace ZeoEngine {

	Ref<SceneRenderer> EngineUtils::GetSceneRendererFromContext(const Ref<Scene>& scene)
	{
		for (const auto& [name, editor] : EditorManager::Get().GetEditors())
		{
			// We check scene context instead of scene because scene is not stable and will be copied when PIE starts
			if (editor->GetScene()->GetContext() == scene->GetContext())
			{
				return editor->GetSceneRenderer();
			}
		}

		return nullptr;
	}

}
