#include "Core/AssetFactory.h"

#include "Engine/Utils/PathUtils.h"

namespace ZeoEngine {

	void AssetFactoryBase::CreateAsset(AssetTypeId typeId, const std::string& path)
	{
		PathUtils::CreateEmptyAsset(typeId, path);
	}

}
