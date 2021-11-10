#include "ZEpch.h"
#include "Engine/Core/AssetFactory.h"

#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Core/Serializer.h"
#include "Engine/Renderer/Mesh.h"

namespace ZeoEngine {

	void AssetFactoryBase::CreateAsset(const std::string& path) const
	{
		AssetSerializer::Serialize(path, m_TypeId, {});
	}

	void ImportableAssetFactoryBase::ImportAsset(const std::string& srcPath, const std::string& destPath) const
	{
		// No need to copy self
		if (destPath != srcPath)
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
			ImportableAssetSerializer::Serialize(assetPath, m_TypeId, {}, srcPath);
			auto spec = AssetRegistry::Get().OnPathCreated(assetPath, m_TypeId);
			spec->Flags |= PathFlag_Importable;
			// Record resource path
			std::dynamic_pointer_cast<AssetSpec>(spec)->UpdateResourcePath(srcPath);
		}
		ZE_CORE_INFO("Successfully imported \"{0}\" from \"{1}\"", destPath, srcPath);
	}

}
