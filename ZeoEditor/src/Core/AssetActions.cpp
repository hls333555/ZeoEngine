#include "Engine/Core/AssetActions.h"

#include "Engine/GameFramework/ParticleSystem.h"
#include "Core/EditorManager.h"
#include "Core/EditorTypes.h"
#include "Editors/ParticleEditor.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Utils/PlatformUtils.h"

namespace ZeoEngine {

	void AssetActionsBase::RenameAsset(const std::string& oldPath, const std::string& newPath) const
	{
		PathUtils::RenamePath(oldPath, newPath);
	}

	void AssetActionsBase::DeleteAsset(const std::string& path) const
	{
		PathUtils::DeletePath(path);
		AssetRegistry::Get().OnPathRemoved(path);
	}

	void ImportableAssetActionsBase::RenameAsset(const std::string& oldPath, const std::string& newPath) const
	{
		auto resourcePath = PathUtils::GetResourcePathFromAssetPath(oldPath);
		auto newResourcePath = PathUtils::GetResourcePathFromAssetPath(newPath);
		// Rename resource
		PathUtils::RenamePath(resourcePath, newResourcePath);
		// Rename asset
		AssetActionsBase::RenameAsset(oldPath, newPath);
	}

	void ImportableAssetActionsBase::DeleteAsset(const std::string& path) const
	{
		auto resourcePath = PathUtils::GetResourcePathFromAssetPath(path);
		// Delete resource
		PathUtils::DeletePath(resourcePath);
		// Delete asset
		AssetActionsBase::DeleteAsset(path);
	}

	void ImportableAssetActionsBase::ReimportAsset(const std::string& path) const
	{
		auto assetSpec = AssetRegistry::Get().GetPathSpec<AssetSpec>(path);
		auto& srcPath = assetSpec->ResourcePath;
		if (srcPath.empty() || !PathUtils::DoesPathExist(srcPath))
		{
			auto filePath = FileDialogs::OpenFile();
			if (!filePath) return;

			srcPath = PathUtils::GetRelativePath(*filePath);
		}

		const auto destPath = PathUtils::GetResourcePathFromAssetPath(path);
		if (destPath != srcPath)
		{
			// Copy and overwrite existing resource
			bool bSuccess = PathUtils::CopyFile(srcPath, destPath, true);
			if (!bSuccess)
			{
				ZE_CORE_ERROR("Failed to reimport asset!");
				return;
			}
		}
		assetSpec->UpdateAll(srcPath);
		// TODO: Should be marked modified instead of saving directly
		// Save asset
		SaveAsset(path);
		// Reload asset
		ReloadAsset(path);
		ZE_CORE_INFO("Successfully reimported \"{0}\" from \"{1}\"", path, srcPath);
	}

	void SceneAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().GetEditor(SCENE_EDITOR)->LoadScene(path);
	}

	void SceneAssetActions::ReloadAsset(const std::string& path) const
	{
		SceneAssetLibrary::Get().ReloadAsset(path);
	}

	// TODO: Only to save those which are marked modified
	void SceneAssetActions::SaveAsset(const std::string& path) const
	{
		if (SceneAssetLibrary::Get().HasAsset(path))
		{
			SceneAssetLibrary::Get().GetAsset(path)->Serialize(path);
		}
	}

	void ParticleAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().OpenEditor<ParticleEditor>(PARTICLE_EDITOR)->LoadScene(path);
	}

	void ParticleAssetActions::ReloadAsset(const std::string& path) const
	{
		ParticleTemplateAssetLibrary::Get().ReloadAsset(path);
	}

	void ParticleAssetActions::SaveAsset(const std::string& path) const
	{
		if (ParticleTemplateAssetLibrary::Get().HasAsset(path))
		{
			ParticleTemplateAssetLibrary::Get().GetAsset(path)->Serialize(path);
		}
	}

	void Texture2DAssetActions::OpenAsset(const std::string& path) const
	{

	}

	void Texture2DAssetActions::ReloadAsset(const std::string& path) const
	{
		Texture2DAssetLibrary::Get().ReloadAsset(path);
	}

	void Texture2DAssetActions::SaveAsset(const std::string& path) const
	{
		if (Texture2DAssetLibrary::Get().HasAsset(path))
		{
			Texture2DAssetLibrary::Get().GetAsset(path)->Serialize(path);
		}
	}

}
