#include "Engine/Core/AssetActions.h"

#include "Engine/GameFramework/ParticleSystem.h"
#include "Core/EditorManager.h"
#include "Editors/EditorBase.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	void SceneAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().GetEditor(EditorType::MainEditor)->LoadAsset(path);
	}

	void ParticleAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().OpenEditor(EditorType::ParticleEditor)->LoadAsset(path);
	}

}
