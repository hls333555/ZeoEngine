#include "ZEpch.h"
#include "Engine/Asset/AssetFactory.h"

#include "Engine/Utils/PathUtils.h"
#include "Engine/Utils/EngineUtils.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Asset/AssetSerializer.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Renderer/Mesh.h"

namespace ZeoEngine {

	void AssetFactoryBase::CreateAssetFile(const std::filesystem::path& path) const
	{
		const std::filesystem::path templatePath = GetAssetTemplatePath();
		if (PathUtils::Exists(templatePath))
		{
			PathUtils::CopyFile(templatePath, path, true);
		}
		else
		{
			ZE_CORE_WARN("Failed to find the template when creating a new asset!");
		}
		const auto& ar = AssetRegistry::Get();
		const auto metadata = ar.GetAssetMetadata(path);
		AssetHandle handle = AssetHandle();
		// In most cases, asset metadata is generated after asset creation
		// The only exception is when a new asset is created via right clicking in the Content Browser Panel, metadata is generated before renaming request
		// When execution reaches here, metadata and AssetHandle is already there,
		// therefore, we just grab the handle instead of generating a new one
		if (metadata && metadata->Handle != 0)
		{
			handle = metadata->Handle;
		}
		AssetSerializerBase::SerializeEmptyAsset(path, m_TypeID, handle, false);
	}

	void ImportableAssetFactoryBase::CreateAsset(const std::filesystem::path& path, const std::filesystem::path& resourcePath) const
	{
		const std::filesystem::path templatePath = GetAssetTemplatePath();
		if (PathUtils::Exists(templatePath))
		{
			PathUtils::CopyFile(templatePath, path, true);
		}
		else
		{
			ZE_CORE_WARN("Failed to find the template when creating a new asset!");
		}
		ImportableAssetSerializerBase::SerializeEmptyAsset(path, resourcePath, m_TypeID, AssetHandle(), false);
	}

	void ImportableAssetFactoryBase::ImportAsset(const std::filesystem::path& srcPath, const std::filesystem::path& destPath) const
	{
		ZE_CORE_ASSERT(!srcPath.empty());

		// No need to copy self
		if (!PathUtils::Exists(destPath) || !PathUtils::Equivalent(srcPath, destPath))
		{
			// Copy resource file
			const bool bSuccess = PathUtils::CopyFile(srcPath, destPath, true);
			if (!bSuccess)
			{
				ZE_CORE_ERROR("Failed to import asset!");
				return;
			}
		}

		auto assetPath = destPath;
		assetPath += AssetRegistry::GetEngineAssetExtension();
		if (!PathUtils::Exists(assetPath))
		{
			// Create zasset file if not exists
			CreateAsset(assetPath, srcPath);
			const auto metadata = AssetRegistry::Get().OnPathCreated(assetPath, true);
			metadata->Flags |= PathFlag_Importable | PathFlag_HasResource;
			// Record source path
			std::dynamic_pointer_cast<AssetMetadata>(metadata)->SourcePath = srcPath;
		}
		else // Asset already exist, just reload and update
		{
			// TODO: Should be marked modified instead of saving directly
			//SaveAsset(path);
			AssetLibrary::ReloadAsset(assetPath);
			const auto metadata = AssetRegistry::Get().GetAssetMetadata(assetPath);
			// Update asset metadata after reloading
			metadata->UpdateThumbnail();
			metadata->SourcePath = srcPath;
		}

		ZE_CORE_INFO("Successfully imported {0} from {1}", destPath, srcPath);
	}

	void ResourceAssetFactoryBase::CreateAssetFile(const std::filesystem::path& path) const
	{
		const std::filesystem::path templatePath = GetAssetTemplatePath();
		const auto templateMetadata = AssetRegistry::Get().GetAssetMetadata(templatePath);
		const auto templateResourcePath = templateMetadata->GetResourcePath();
		ZE_CORE_ASSERT(PathUtils::Exists(templatePath) && PathUtils::Exists(templateResourcePath));

		const auto metadata = AssetRegistry::Get().GetAssetMetadata(path);
		metadata->Flags |= PathFlag_HasResource;
		// Copy resource template file
		const auto resourcePath = metadata->GetResourcePath();
		PathUtils::CopyFile(templateResourcePath, resourcePath, true);
		// Create zasset file
		AssetFactoryBase::CreateAssetFile(path);
	}

	const char* LevelAssetFactory::GetAssetTemplatePath() const
	{
		return Level::GetTemplatePath();
	}

	Ref<IAsset> LevelAssetFactory::CreateAsset(const Ref<AssetMetadata>& metadata) const
	{
		return CreateRef<Level>();
	}

	Ref<IAsset> ParticleTemplateAssetFactory::CreateAsset(const Ref<AssetMetadata>& metadata) const
	{
		return CreateRef<ParticleTemplate>();
	}

	Ref<IAsset> Texture2DAssetFactory::CreateAsset(const Ref<AssetMetadata>& metadata) const
	{
		return Texture2D::Create(metadata->GetResourcePath().string());
	}

	void MeshAssetFactory::ImportAsset(const std::filesystem::path& srcPath, const std::filesystem::path& destPath) const
	{
		ImportableAssetFactoryBase::ImportAsset(srcPath, destPath);

		auto assetPath = destPath;
		assetPath += AssetRegistry::GetEngineAssetExtension();
		const auto mesh = AssetLibrary::LoadAsset<Mesh>(assetPath);
		if (!mesh) return;

		// Create dummy materials automatically
		// TODO: Auto assign these to mesh asset
		for (const auto& materialName : mesh->GetMaterialNames())
		{
			const auto materialPath = destPath.parent_path() / fmt::format("Mat_{0}{1}", materialName, AssetRegistry::GetEngineAssetExtension());
			if (!PathUtils::Exists(materialPath))
			{
				if (AssetManager::Get().CreateAssetFile(Material::TypeID(), materialPath))
				{
					AssetRegistry::Get().OnPathCreated(materialPath, true);
				}
			}
		}
	}

	Ref<IAsset> MeshAssetFactory::CreateAsset(const Ref<AssetMetadata>& metadata) const
	{
		return CreateRef<Mesh>(metadata->GetResourcePath().string());
	}

	const char* MaterialAssetFactory::GetAssetTemplatePath() const
	{
		return  Material::GetTemplatePath();
	}

	Ref<IAsset> MaterialAssetFactory::CreateAsset(const Ref<AssetMetadata>& metadata) const
	{
		return CreateRef<Material>();
	}

	const char* ShaderAssetFactory::GetAssetTemplatePath() const
	{
		return Shader::GetTemplatePath();
	}

	const char* ShaderAssetFactory::GetResourceTemplatePath() const
	{
		return  Shader::GetResourceTemplatePath();
	}

	Ref<IAsset> ShaderAssetFactory::CreateAsset(const Ref<AssetMetadata>& metadata) const
	{
		return Shader::Create(metadata->GetResourcePath().string());
	}

}
