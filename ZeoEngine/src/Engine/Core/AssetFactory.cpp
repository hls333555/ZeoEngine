#include "ZEpch.h"
#include "Engine/Core/AssetFactory.h"

#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Core/Serializer.h"
#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	void AssetFactoryBase::CreateAsset(const std::string& path) const
	{
		const char* templatePath = GetAssetTemplatePath();
		if (strlen(templatePath))
		{
			PathUtils::CopyFile(templatePath, path, true);
		}
		else
		{
			ZE_CORE_WARN("Failed to find the template when creating a new asset!");
			AssetSerializer::Serialize(path, m_TypeId, {});
		}
	}

	void ImportableAssetFactoryBase::CreateAsset(const std::string& path, const std::string& resourcePath) const
	{
		const char* templatePath = GetAssetTemplatePath();
		if (strlen(templatePath))
		{
			PathUtils::CopyFile(templatePath, path, true);
		}
		else
		{
			ZE_CORE_WARN("Failed to find the template when creating a new asset!");
			ImportableAssetSerializer::Serialize(path, m_TypeId, {}, resourcePath);
		}
	}

	void ImportableAssetFactoryBase::ImportAsset(const std::string& srcPath, const std::string& destPath) const
	{
		// No need to copy self
		if (destPath != srcPath)
		{
			// Copy resource file
			const bool bSuccess = PathUtils::CopyFile(srcPath, destPath, true);
			if (!bSuccess)
			{
				ZE_CORE_ERROR("Failed to import asset!");
				return;
			}
		}
		const std::string assetPath = destPath + AssetRegistry::GetEngineAssetExtension();
		if (!PathUtils::DoesPathExist(assetPath))
		{
			// Create zasset file if not exists
			CreateAsset(assetPath, srcPath);
			const auto spec = AssetRegistry::Get().OnPathCreated(assetPath, m_TypeId);
			spec->Flags |= PathFlag_Importable | PathFlag_HasResource;
			// Record source path
			std::dynamic_pointer_cast<AssetSpec>(spec)->SourcePath = srcPath;
		}
		ZE_CORE_INFO("Successfully imported \"{0}\" from \"{1}\"", destPath, srcPath);
	}

	void ResourceAssetFactoryBase::CreateAsset(const std::string& path) const
	{
		// Copy resource template file
		const std::string resourcePath = PathUtils::GetResourcePathFromPath(path);
		PathUtils::CopyFile(GetResourceTemplatePath(), resourcePath, true);
		// Create zasset file
		AssetFactoryBase::CreateAsset(path);
	}

	const char* LevelAssetFactory::GetAssetTemplatePath() const
	{
		return Level::GetTemplatePath();
	}

	const char* MaterialAssetFactory::GetAssetTemplatePath() const
	{
		return  Material::GetTemplatePath();
	}

	const char* ShaderAssetFactory::GetAssetTemplatePath() const
	{
		return Shader::GetTemplatePath();
	}

	const char* ShaderAssetFactory::GetResourceTemplatePath() const
	{
		return  Shader::GetResourceTemplatePath();
	}
}
