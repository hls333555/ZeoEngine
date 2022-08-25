#include "Engine/Asset/AssetActions.h"

#include "Core/EditorManager.h"
#include "Core/EditorTypes.h"
#include "Editors/LevelEditor.h"
#include "Editors/ParticleEditor.h"
#include "Editors/MaterialEditor.h"
#include "Editors/MeshEditor.h"
#include "Editors/TextureEditor.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Utils/PathUtils.h"

namespace ZeoEngine {

	void AssetActionsBase::RenameAsset(const std::string& oldPath, const std::string& newPath) const
	{
		PathUtils::RenamePath(oldPath, newPath);
		// TODO: Fixup references

	}

	void AssetActionsBase::DeleteAsset(const std::string& path) const
	{
		PathUtils::DeletePath(path);
		AssetRegistry::Get().OnPathRemoved(path);
	}

	void ResourceAssetActionsBase::RenameAsset(const std::string& oldPath, const std::string& newPath) const
	{
		const auto& ar = AssetRegistry::Get();
		const auto resourcePath = ar.GetAssetMetadata(oldPath)->GetResourceFileSystemPath();
		const auto newResourcePath = ar.GetAssetMetadata(newPath)->GetResourceFileSystemPath();
		// Rename resource
		PathUtils::RenamePath(resourcePath, newResourcePath);
		// Rename asset
		AssetActionsBase::RenameAsset(oldPath, newPath);
	}

	void ResourceAssetActionsBase::DeleteAsset(const std::string& path) const
	{
		const auto resourcePath = AssetRegistry::Get().GetAssetMetadata(path)->GetResourceFileSystemPath();
		// Delete resource
		PathUtils::DeletePath(resourcePath);
		// Delete asset
		AssetActionsBase::DeleteAsset(path);
	}

	void ImportableAssetActionsBase::ReimportAsset(const std::string& path) const
	{
		const auto& ar = AssetRegistry::Get();
		const auto metadata = ar.GetAssetMetadata(path);
		auto srcPath = metadata->SourcePath;
		if (srcPath.empty() || !PathUtils::Exists(srcPath))
		{
			// Open file dialog to select another source to reimport
			const auto filePaths = FileDialogs::Open(false);
			if (filePaths.size() != 1) return;

			srcPath = filePaths[0];
		}

		const std::string destPath = metadata->GetResourceFileSystemPath();
		// Copy and overwrite existing resource
		const bool bSuccess = PathUtils::CopyFile(srcPath, destPath, true);
		if (!bSuccess)
		{
			ZE_CORE_ERROR("Failed to reimport asset!");
			return;
		}

		// TODO: Should be marked modified instead of saving directly
		//SaveAsset(path);
		AssetLibrary::ReloadAsset(path);
		// Update asset metadata after reloading
		metadata->UpdateThumbnail();
		metadata->SourcePath = std::move(srcPath);

		ZE_CORE_INFO("Successfully reimported {0} from {1}", path, metadata->SourcePath);
	}

	void LevelAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().OpenEditor<LevelEditor>(LEVEL_EDITOR, path);
	}

	void ParticleTemplateAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().OpenEditor<ParticleEditor>(PARTICLE_EDITOR, path);
	}

	void Texture2DAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().OpenEditor<TextureEditor>(TEXTURE_EDITOR, path);
	}

	void MeshAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().OpenEditor<MeshEditor>(MESH_EDITOR, path);
	}

	void MaterialAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().OpenEditor<MaterialEditor>(MATERIAL_EDITOR, path);
	}

	void ShaderAssetActions::OpenAsset(const std::string& path) const
	{
		const std::string resourcePath = AssetRegistry::Get().GetAssetMetadata(path)->GetResourceFileSystemPath();
		PlatformUtils::OpenFile(resourcePath);
	}

}
