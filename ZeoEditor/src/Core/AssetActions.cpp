#include "Engine/Core/AssetActions.h"

#include "Engine/GameFramework/ParticleSystem.h"
#include "Core/EditorManager.h"
#include "Core/EditorTypes.h"
#include "Editors/ParticleEditor.h"
#include "Editors/MaterialEditor.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Shader.h"

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
		const auto resourcePath = PathUtils::GetResourcePathFromPath(oldPath);
		const auto newResourcePath = PathUtils::GetResourcePathFromPath(newPath);
		// Rename resource
		PathUtils::RenamePath(resourcePath, newResourcePath);
		// Rename asset
		AssetActionsBase::RenameAsset(oldPath, newPath);
	}

	void ResourceAssetActionsBase::DeleteAsset(const std::string& path) const
	{
		const auto resourcePath = PathUtils::GetResourcePathFromPath(path);
		// Delete resource
		PathUtils::DeletePath(resourcePath);
		// Delete asset
		AssetActionsBase::DeleteAsset(path);
	}

	void ImportableAssetActionsBase::ReimportAsset(const std::string& path) const
	{
		const auto assetSpec = AssetRegistry::Get().GetPathSpec<AssetSpec>(path);
		auto& srcPath = assetSpec->SourcePath;
		if (srcPath.empty() || !PathUtils::DoesPathExist(srcPath))
		{
			const auto filePath = FileDialogs::OpenFile();
			if (!filePath) return;

			srcPath = PathUtils::GetRelativePath(*filePath);
		}

		const auto destPath = PathUtils::GetResourcePathFromPath(path);
		if (destPath != srcPath)
		{
			// Copy and overwrite existing resource
			const bool bSuccess = PathUtils::CopyFile(srcPath, destPath, true);
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

	void LevelAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().GetEditor(LEVEL_EDITOR)->LoadScene(path);
	}

	void LevelAssetActions::ReloadAsset(const std::string& path) const
	{
		// TODO:
		LevelLibrary::Get().ReloadAsset(path);
	}

	// TODO: Only to save those which are marked modified
	void LevelAssetActions::SaveAsset(const std::string& path) const
	{
		if (const auto level = LevelLibrary::Get().LoadAsset(path))
		{
			level->Serialize(path);
		}
	}

	void ParticleTemplateAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().OpenEditor<ParticleEditor>(PARTICLE_EDITOR)->LoadScene(path);
	}

	void ParticleTemplateAssetActions::ReloadAsset(const std::string& path) const
	{
		ParticleTemplateLibrary::Get().ReloadAsset(path);
	}

	void ParticleTemplateAssetActions::SaveAsset(const std::string& path) const
	{
		if (const auto particleTemplate = ParticleTemplateLibrary::Get().LoadAsset(path))
		{
			particleTemplate->Serialize(path);
		}
	}

	void Texture2DAssetActions::OpenAsset(const std::string& path) const
	{

	}

	void Texture2DAssetActions::ReloadAsset(const std::string& path) const
	{
		Texture2DLibrary::Get().ReloadAsset(path);
	}

	void Texture2DAssetActions::SaveAsset(const std::string& path) const
	{
		if (const auto texture = Texture2DLibrary::Get().LoadAsset(path))
		{
			texture->Serialize(path);
		}
	}

	void MeshAssetActions::OpenAsset(const std::string& path) const
	{
		
	}

	void MeshAssetActions::ReloadAsset(const std::string& path) const
	{
		MeshLibrary::Get().ReloadAsset(path);
	}

	void MeshAssetActions::SaveAsset(const std::string& path) const
	{
		if (const auto mesh = MeshLibrary::Get().LoadAsset(path))
		{
			mesh->Serialize(path);
		}
	}

	void MaterialAssetActions::OpenAsset(const std::string& path) const
	{
		EditorManager::Get().OpenEditor<MaterialEditor>(MATERIAL_EDITOR)->LoadScene(path);
	}

	void MaterialAssetActions::ReloadAsset(const std::string& path) const
	{
		MaterialLibrary::Get().ReloadAsset(path);
	}

	void MaterialAssetActions::SaveAsset(const std::string& path) const
	{
		if (const auto material = MaterialLibrary::Get().LoadAsset(path))
		{
			material->Serialize(path);
		}
	}

	void ShaderAssetActions::OpenAsset(const std::string& path) const
	{
		const std::string resourcePath = PathUtils::GetResourcePathFromPath(path);
		PlatformUtils::OpenFile(resourcePath);
	}

	void ShaderAssetActions::ReloadAsset(const std::string& path) const
	{
		ShaderLibrary::Get().ReloadAsset(path);
	}

	void ShaderAssetActions::HotReloadAsset(const std::string& path) const
	{
		ReloadAsset(path);
	}

	void ShaderAssetActions::SaveAsset(const std::string& path) const
	{
		if (const auto shader = ShaderLibrary::Get().LoadAsset(path))
		{
			shader->Serialize(path);
		}
	}

}
