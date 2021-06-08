#include "ZEpch.h"
#include "Engine/Core/AssetFactory.h"

#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/AssetRegistry.h"

namespace ZeoEngine {

	void AssetFactoryBase::CreateAsset(const std::string& path)
	{
		PathUtils::CreateEmptyAsset(m_TypeId, path);
	}

	void AssetFactoryBase::ImportAsset(const std::string& srcPath, const std::string& destPath)
	{
		// No need to copy self
		if (PathUtils::GetCanonicalPath(destPath) != PathUtils::GetCanonicalPath(srcPath))
		{
			bool bSuccess = PathUtils::CopyAsset(srcPath, destPath, true);
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
			CreateAsset(assetPath);
			AssetRegistry::Get().OnPathCreated(assetPath, m_TypeId);
		}
		ZE_CORE_INFO("Successfully imported {0}", srcPath);
	}

}
