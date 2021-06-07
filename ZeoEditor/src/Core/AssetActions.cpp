#include "Engine/Core/AssetActions.h"

#include "Engine/GameFramework/ParticleSystem.h"
#include "Core/EditorManager.h"
#include "Editors/EditorBase.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/AssetRegistry.h"

namespace ZeoEngine {

	void AssetActionsBase::DeleteAsset(const std::string& path) const
	{
		PathUtils::DeletePath(path);
		AssetRegistry::Get().OnPathRemoved(path);
	}

	void SceneAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().GetEditor(EditorType::MainEditor)->LoadAsset(path);
	}

	void ParticleAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().OpenEditor(EditorType::ParticleEditor)->LoadAsset(path);
	}

	void Texture2DAssetActions::DeleteAsset(const std::string& path) const
	{
		auto texturePath = Texture2DAsset::GetTexturePath(path);
		// Delete actual texture
		PathUtils::DeletePath(texturePath);
		// Delete texture asset
		AssetActionsBase::DeleteAsset(path);
	}

}
