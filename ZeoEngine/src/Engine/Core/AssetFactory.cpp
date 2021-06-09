#include "ZEpch.h"
#include "Engine/Core/AssetFactory.h"

#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	void AssetFactoryBase::CreateAsset(const std::string& path) const
	{
		Serializer::WriteDataToAsset(path, m_TypeId, [](YAML::Emitter&){});
	}

	void ImportableAssetFactoryBase::ImportAsset(const std::string& srcPath, const std::string& destPath) const
	{
		// No need to copy self
		std::string canonicalSrcPath = PathUtils::GetCanonicalPath(srcPath);
		std::string canonicalDestPath = PathUtils::GetCanonicalPath(destPath);
		if (canonicalDestPath != canonicalSrcPath)
		{
			// Copy resource file
			bool bSuccess = PathUtils::CopyFile(srcPath, destPath, true);
			if (!bSuccess)
			{
				ZE_CORE_ERROR("Failed to import asset!");
				return;
			}
		}
		std::string assetPath = destPath + AssetRegistry::Get().GetEngineAssetExtension();
		if (!PathUtils::DoesPathExist(assetPath))
		{
			// Create zasset file if not exists
			Serializer::WriteDataToAsset(assetPath, m_TypeId, [&canonicalSrcPath](YAML::Emitter& out)
			{
				out << YAML::Key << g_ResourceSourceToken << YAML::Value << canonicalSrcPath;
			});
			auto spec = AssetRegistry::Get().OnPathCreated(assetPath, m_TypeId);
			// Record resource source path
			std::dynamic_pointer_cast<AssetSpec>(spec)->ResourceSourcePath = canonicalSrcPath;
		}
		ZE_CORE_INFO("Successfully imported \"{0}\" from \"{1}\"", destPath, canonicalSrcPath);
	}

}
