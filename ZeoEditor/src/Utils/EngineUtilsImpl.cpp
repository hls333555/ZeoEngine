#include "Engine/Utils/EngineUtils.h"

#include "Core/Editor.h"
#include "Worlds/EditorPreviewWorldBase.h"

namespace ZeoEngine {

	Ref<SceneRenderer> EngineUtils::GetSceneRendererFromContext(const Ref<Scene>& scene)
	{
		for (const auto& [name, world] : g_Editor->m_Worlds)
		{
			// We check scene context instead of scene because scene is not stable and will be copied when PIE starts
			if (world->GetActiveScene()->GetContext() == scene->GetContext())
			{
				return world->GetSceneRenderer();
			}
		}

		return nullptr;
	}

}
