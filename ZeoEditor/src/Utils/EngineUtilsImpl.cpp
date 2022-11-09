#include "Engine/Utils/EngineUtils.h"

#include "Core/Editor.h"
#include "Worlds/EditorPreviewWorldBase.h"

namespace ZeoEngine {

	SceneRenderer* EngineUtils::GetSceneRendererFromContext(const SceneContext* sceneContext)
	{
		for (const auto& [name, world] : g_Editor->m_Worlds)
		{
			// We check scene context instead of scene because scene is not stable and will be copied when PIE starts
			if (world->GetActiveScene()->GetContextShared() == sceneContext)
			{
				return world->GetSceneRenderer();
			}
		}

		return nullptr;
	}

}
