#include "ZEpch.h"
#include "Engine/Asset/AssetFactory.h"

#include "Engine/Utils/FileSystemUtils.h"
#include "Engine/Utils/EngineUtils.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Asset/AssetSerializer.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Physics/PhysicsMaterial.h"
#include "Engine/Renderer/Mesh.h"

namespace ZeoEngine {

	void AssetFactoryBase::CreateAssetFile(const std::string& path) const
	{
		const std::string templatePath = GetAssetTemplatePath();
		if (FileSystemUtils::Exists(templatePath))
		{
			FileSystemUtils::CopyFile(templatePath, path, true);
		}
		else
		{
			ZE_CORE_WARN("Failed to find the template when creating a new asset!");
		}
		const auto* metadata = AssetRegistry::Get().GetAssetMetadata(path);
		AssetHandle handle = AssetHandle();
		// In most cases, asset metadata is generated after asset creation
		// The only exception is when a new asset is created via right clicking in the Content Browser Panel, metadata is generated before renaming request
		// When execution reaches here, metadata and AssetHandle is already there,
		// therefore, we just grab the handle instead of generating a new one
		if (metadata && metadata->Handle != 0)
		{
			handle = metadata->Handle;
		}
		AssetSerializerBase::SerializeEmptyAsset(path, m_TypeID, handle, metadata->Flags, false);
	}

	void ImportableAssetFactoryBase::CreateAssetFile(const std::string& path, const std::string& resourcePath, U8 flags) const
	{
		const std::string templatePath = GetAssetTemplatePath();
		if (FileSystemUtils::Exists(templatePath))
		{
			FileSystemUtils::CopyFile(templatePath, path, true);
		}
		else
		{
			ZE_CORE_WARN("Failed to find the template when creating a new asset!");
		}
		ImportableAssetSerializerBase::SerializeEmptyAsset(path, resourcePath, m_TypeID, AssetHandle(), flags, false);
	}

	void ImportableAssetFactoryBase::ImportAsset(const std::string& srcPath, const std::string& destPath) const
	{
		ZE_CORE_ASSERT(!srcPath.empty());

		// Copy resource file
		const bool bSuccess = FileSystemUtils::CopyFile(srcPath, destPath, true);
		if (!bSuccess)
		{
			ZE_CORE_ERROR("Failed to import asset!");
			return;
		}

		const std::string assetPath = destPath + AssetRegistry::GetAssetExtension();
		if (!FileSystemUtils::Exists(assetPath))
		{
			U8 flags = PathFlag_Importable | PathFlag_HasResource;
			// Create zasset file if not exists
			CreateAssetFile(assetPath, srcPath, flags);
			auto* metadata = static_cast<AssetMetadata*>(AssetRegistry::Get().OnPathCreated(assetPath, true));
			metadata->Flags = flags;
			metadata->SourcePath = srcPath;
		}
		else // Asset already exist, just reload and update
		{
			// TODO: Should be marked modified instead of saving directly
			//SaveAsset(path);
			AssetLibrary::ReloadAsset(assetPath);
			auto* metadata = AssetRegistry::Get().GetAssetMetadata(assetPath);
			// Update asset metadata after reloading
			metadata->UpdateThumbnail();
			metadata->SourcePath = srcPath;
		}

		ZE_CORE_INFO("Successfully imported {0} from {1}", destPath, srcPath);
	}

	void ResourceAssetFactoryBase::CreateAssetFile(const std::string& path) const
	{
		const std::string templatePath = GetAssetTemplatePath();
		const auto templateMetadata = AssetRegistry::Get().GetAssetMetadata(templatePath);
		const auto templateResourcePath = templateMetadata->GetResourceFileSystemPath();
		ZE_CORE_ASSERT(FileSystemUtils::Exists(templatePath) && FileSystemUtils::Exists(templateResourcePath));

		auto* metadata = AssetRegistry::Get().GetAssetMetadata(path);
		metadata->Flags |= PathFlag_HasResource;
		// Copy resource template file
		const auto resourcePath = metadata->GetResourceFileSystemPath();
		FileSystemUtils::CopyFile(templateResourcePath, resourcePath, true);
		// Create zasset file
		AssetFactoryBase::CreateAssetFile(path);
	}

	const char* LevelAssetFactory::GetAssetTemplatePath() const
	{
		return Level::GetTemplatePath();
	}

	Ref<IAsset> LevelAssetFactory::CreateAsset(const AssetMetadata* metadata) const
	{
		return CreateRef<Level>();
	}

	Ref<IAsset> ParticleTemplateAssetFactory::CreateAsset(const AssetMetadata* metadata) const
	{
		return CreateRef<ParticleTemplate>();
	}

	Ref<IAsset> Texture2DAssetFactory::CreateAsset(const AssetMetadata* metadata) const
	{
		return Texture2D::Create(metadata->GetResourceFileSystemPath());
	}

	void MeshAssetFactory::ImportAsset(const std::string& srcPath, const std::string& destPath) const
	{
		ImportableAssetFactoryBase::ImportAsset(srcPath, destPath);

		auto assetPath = destPath;
		assetPath += AssetRegistry::GetAssetExtension();
		const auto mesh = AssetLibrary::LoadAsset<Mesh>(assetPath);
		if (!mesh) return;

		// Create dummy materials automatically
		const auto& materialNames = mesh->GetMaterialNames();
		for (SizeT i = 0; i < materialNames.size(); ++i)
		{
			const auto materialPath = fmt::format("{}/Mat_{}{}", FileSystemUtils::GetParentPath(destPath), materialNames[i], AssetRegistry::GetAssetExtension());
			if (!FileSystemUtils::Exists(materialPath))
			{
				if (AssetManager::Get().CreateAssetFile(Material::TypeID(), materialPath))
				{
					AssetRegistry::Get().OnPathCreated(materialPath, true);
					// Assign material to mesh
					mesh->SetDefaultMaterialAsset(static_cast<U32>(i), AssetLibrary::LoadAsset<Material>(materialPath)->GetHandle());
				}
			}
		}
	}

	Ref<IAsset> MeshAssetFactory::CreateAsset(const AssetMetadata* metadata) const
	{
		return CreateRef<Mesh>(metadata->GetResourceFileSystemPath());
	}

	const char* MaterialAssetFactory::GetAssetTemplatePath() const
	{
		return  Material::GetTemplatePath();
	}

	Ref<IAsset> MaterialAssetFactory::CreateAsset(const AssetMetadata* metadata) const
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

	Ref<IAsset> ShaderAssetFactory::CreateAsset(const AssetMetadata* metadata) const
	{
		return Shader::Create(metadata->GetResourceFileSystemPath());
	}

	Ref<IAsset> PhysicsMaterialAssetFactory::CreateAsset(const AssetMetadata* metadata) const
	{
		return CreateRef<PhysicsMaterial>();
	}

}
