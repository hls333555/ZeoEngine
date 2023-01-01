#include "ZEpch.h"
#include "Engine/Asset/AssetRegistry.h"

#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Asset/AssetSerializer.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/Core/CommonPaths.h"
#include "Engine/Core/Project.h"
#include "Engine/Profile/Profiler.h"

namespace ZeoEngine {

	namespace Utils {

		static std::string FileNameToUpperCase(const std::string& fileName)
		{
			std::string outStr = fileName;
			std::transform(outStr.begin(), outStr.end(), outStr.begin(), ::toupper);
			return outStr;
		}

		static bool ParseAssetFile(const Ref<AssetMetadata>& metadata)
		{
			const auto res = AssetSerializerBase::DeserializeAsset(metadata->Path);
			if (!res) return false;

			const auto node = *res;
			const auto assetTypeData = node["AssetType"];
			if (!assetTypeData) return false;

			const auto assetHandleData = node["AssetHandle"];
			if (!assetHandleData) return false;

			// Retrieve IDs
			metadata->TypeID = assetTypeData.as<AssetTypeID>();
			metadata->Handle = assetHandleData.as<AssetHandle>();

			// Possible retrieve flags
			if (const auto flagsData = node["Flags"])
			{
				metadata->Flags = flagsData.as<U8>();
			}

			// Possibly retrieve source path
			if (const auto sourceData = node["SourcePath"])
			{
				metadata->SourcePath = sourceData.as<std::string>();
			}

			return true;
		}

	}

	void AssetMetadata::UpdateThumbnail()
	{
		ThumbnailTexture = ThumbnailManager::Get().GetAssetThumbnail(this);
	}

	void AssetRegistry::Register()
	{
		Project::GetProjectLoadedDelegate().connect<&AssetRegistry::Init>(this);
		Project::GetProjectUnloadedDelegate().connect<&AssetRegistry::Shutdown>(this);
	}

	void AssetRegistry::Init()
	{
		Timer timer;
		BuildPathTree(CommonPaths::GetEngineAssetDirectory());
		BuildPathTree(CommonPaths::GetProjectAssetDirectory());
		SortPathTree();
		ZE_CORE_WARN("Building path tree took {0} ms", timer.ElapsedMillis());
	}

	void AssetRegistry::Shutdown()
	{
		// Unload project assets
		for (const auto& [path, metadata] : m_PathMetadatas)
		{
			if (const auto assetMetadata = std::dynamic_pointer_cast<AssetMetadata>(metadata))
			{
				AssetLibrary::UnloadAsset(assetMetadata->Handle);
			}
		}

		m_PathTree.clear();
		m_PathMetadatas.clear();
		m_AssetMetadatasByID.clear();
	}

	void AssetRegistry::BuildPathTree(const std::filesystem::path& rootDirectory)
	{
		std::string rootPath = FileSystemUtils::GetStandardPath(rootDirectory);
		m_PathTree.emplace_back(rootPath, std::vector<std::string>{});
		m_PathMetadatas[std::move(rootPath)] = CreateRef<DirectoryMetadata>(rootPath);

		BuildPathTreeRecursively(rootDirectory);
	}

	void AssetRegistry::BuildPathTreeRecursively(const std::filesystem::path& baseDirectory)
	{
		for (const auto& it : std::filesystem::directory_iterator(baseDirectory))
		{
			switch (it.status().type())
			{
				case std::filesystem::file_type::directory:
					AddDirectoryToTree(FileSystemUtils::GetStandardPath(baseDirectory), FileSystemUtils::GetStandardPath(it.path()));

					BuildPathTreeRecursively(it.path());
					break;
				case std::filesystem::file_type::regular:
					if (it.path().extension().string() == GetAssetExtension())
					{
						AddAssetToTree(FileSystemUtils::GetStandardPath(baseDirectory), FileSystemUtils::GetStandardPath(it.path()));
					}
					break;
				default:
					break;
			}
		}
	}

	AssetHandle AssetRegistry::GetAssetHandleFromPath(const std::string& path) const
	{
		const auto* metadata = GetAssetMetadata(path);
		return metadata ? metadata->Handle : 0;
	}

	bool AssetRegistry::ContainsPathInDirectory(const std::string& baseDirectory, const std::string& path)
	{
		const auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it == m_PathTree.end()) return false;

		return std::find(it->second.begin(), it->second.end(), path) != it->second.end();
	}
	
	const std::vector<std::string>& AssetRegistry::GetPathsInDirectory(const std::string& baseDirectory)
	{
		const auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		ZE_CORE_ASSERT(it != m_PathTree.end());

		return it->second;
	}

	DirectoryMetadata* AssetRegistry::AddDirectoryToTree(const std::string& baseDirectory, const std::string& path)
	{
		m_PathTree.emplace_back(path, std::vector<std::string>{});
		auto metadata = CreateRef<DirectoryMetadata>(path);
		metadata->ThumbnailTexture = ThumbnailManager::Get().GetDirectoryIcon();
		auto* metadataPtr = metadata.get();
		m_PathMetadatas[metadata->Path] = std::move(metadata);
		GetPathMetadata<DirectoryMetadata>(baseDirectory)->bHasAnySubDirectory = true;

		const auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it != m_PathTree.end())
		{
			it->second.emplace_back(path);
		}

		return metadataPtr;
	}

	AssetMetadata* AssetRegistry::AddAssetToTree(const std::string& baseDirectory, const std::string& path, AssetTypeID typeID)
	{
		auto metadata = CreateRef<AssetMetadata>(path);
		if (!typeID)
		{
			const bool bResult = Utils::ParseAssetFile(metadata);
			if (!bResult)
			{
				ZE_CORE_WARN("Invalid asset detected! Asset path: {0}", path);
				return {};
			}
			// Ignore asset with existing handle
			const auto it = std::find_if(m_PathMetadatas.begin(), m_PathMetadatas.end(), [&metadata](const auto& pair)
			{
				return pair.second->IsAsset() && std::static_pointer_cast<AssetMetadata>(pair.second)->Handle == metadata->Handle;
			});
			if (it != m_PathMetadatas.end())
			{
				ZE_CORE_WARN("Duplicated asset detected! Asset path: {0}", path);
				return {};
			}
		}
		else
		{
			metadata->TypeID = typeID;
			metadata->Handle = AssetHandle();
		}

		// TODO: Optimize: No need to load all textures
		metadata->UpdateThumbnail();
		m_AssetMetadatasByID[metadata->TypeID].emplace_back(metadata);
		auto* metadataPtr = metadata.get();
		m_PathMetadatas[metadata->Path] = std::move(metadata);

		const auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it != m_PathTree.end())
		{
			it->second.emplace_back(path);
		}

		return metadataPtr;
	}

	std::vector<std::string>::iterator AssetRegistry::RemovePathFromTree(std::string path)
	{
		if (const auto* metadata = GetAssetMetadata(path))
		{
			auto& assetMetadatas = m_AssetMetadatasByID[metadata->TypeID];
			const auto it = std::find_if(assetMetadatas.begin(), assetMetadatas.end(), [metadata](const Ref<AssetMetadata>& assetMetadata)
			{
				return assetMetadata.get() == metadata;
			});
			ZE_CORE_ASSERT(it != assetMetadatas.end());
			assetMetadatas.erase(it);
		}

		const auto currentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&path](const auto& pair)
		{
			return pair.first == path;
		});
		// For assets, currentPathIt should be invalid
		if (currentPathIt != m_PathTree.end())
		{
			for (auto it = currentPathIt->second.begin(); it != currentPathIt->second.end();)
			{
				// Remove path from tree recursively
				it = RemovePathFromTree(*it);
			}
			m_PathTree.erase(currentPathIt);
		}

		const auto parentPath = FileSystemUtils::GetParentPath(path);
		const auto parentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&parentPath](const auto& pair)
		{
			return pair.first == parentPath;
		});
		ZE_CORE_ASSERT(parentPathIt != m_PathTree.end());
		const auto currentPathInParentIt = std::find(parentPathIt->second.begin(), parentPathIt->second.end(), path);
		ZE_CORE_ASSERT(currentPathInParentIt != parentPathIt->second.end());
		// Return the iterator for next loop use
		auto retIt = parentPathIt->second.erase(currentPathInParentIt);
		auto* parentPathMetadata = GetPathMetadata<DirectoryMetadata>(parentPath);
		ZE_CORE_ASSERT(parentPathMetadata);
		parentPathMetadata->bHasAnySubDirectory = false;
		for (const auto& subPath : parentPathIt->second)
		{
			if (GetPathMetadata<DirectoryMetadata>(subPath))
			{
				parentPathMetadata->bHasAnySubDirectory = true;
				break;
			}
		}

		const auto it = m_PathMetadatas.find(FileSystemUtils::GetStandardPath(path));
		ZE_CORE_ASSERT(it != m_PathMetadatas.end());
		m_PathMetadatas.erase(it);

		return retIt;
	}

	void AssetRegistry::RenamePathInTree(const std::string& baseDirectory, const std::string& oldPath, const std::string& newPath)
	{
		const auto parentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		ZE_CORE_ASSERT(parentPathIt != m_PathTree.end());
		const auto currentPathInParentIt = std::find(parentPathIt->second.begin(), parentPathIt->second.end(), oldPath);
		ZE_CORE_ASSERT(currentPathInParentIt != parentPathIt->second.end());
		*currentPathInParentIt = newPath;

		const auto currentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&oldPath](const auto& pair)
		{
			return pair.first == oldPath;
		});
		// For assets, currentPathIt should be invalid
		if (currentPathIt != m_PathTree.end())
		{
			currentPathIt->first = newPath;
		}

		m_PathMetadatas[oldPath]->Path = newPath;
		m_PathMetadatas[oldPath]->PathName = FileSystemUtils::GetPathName(newPath);
		// C++17 way of modifying key of a map
		auto node = m_PathMetadatas.extract(oldPath);
		node.key() = newPath;
		m_PathMetadatas.insert(std::move(node));
	}

	void AssetRegistry::SortPathTree()
	{
		for (auto& [baseDirectory, subPaths] : m_PathTree)
		{
			std::sort(subPaths.begin(), subPaths.end(), [this](const auto& lhs, const auto& rhs)
			{
				const bool bIsLhsAsset = m_PathMetadatas[lhs]->IsAsset();
				const bool bIsRhsAsset = m_PathMetadatas[rhs]->IsAsset();
				return !bIsLhsAsset && bIsRhsAsset ||
					(((!bIsLhsAsset && !bIsRhsAsset) || (bIsLhsAsset && bIsRhsAsset)) &&
					// Default comparison behaviour for directories: "NewFolder" is ahead of "cache"
					// So we have to convert file name to upper or lower case before comparing
					// Comparison behaviour after conversion: "cache" is ahead of "NewFolder"
					Utils::FileNameToUpperCase(lhs) < Utils::FileNameToUpperCase(rhs));
			});
		}
		//std::sort(m_PathTree.begin(), m_PathTree.end(), [this](const auto& lhs, const auto& rhs)
		//{
		//	const bool bIsLhsAsset = m_PathMetadatas[lhs.first]->IsAsset();
		//	const bool bIsRhsAsset = m_PathMetadatas[rhs.first]->IsAsset();
		//	return !bIsLhsAsset && bIsRhsAsset ||
		//		(((!bIsLhsAsset && !bIsRhsAsset) || (bIsLhsAsset && bIsRhsAsset)) &&
		//		Utils::FileNameToUpperCase(lhs.first) < Utils::FileNameToUpperCase(rhs.first));
		//});
	}

	AssetMetadata* AssetRegistry::GetAssetMetadata(const std::string& path) const
	{
		if (const auto it = m_PathMetadatas.find(FileSystemUtils::GetStandardPath(path)); it != m_PathMetadatas.cend())
		{
			return dynamic_cast<AssetMetadata*>(it->second.get());
		}
		return nullptr;
	}

	AssetMetadata* AssetRegistry::GetAssetMetadata(AssetHandle handle) const
	{
		if (!handle) return nullptr;

		for (const auto& [path, metadata] : m_PathMetadatas)
		{
			if (auto* assetMetadata = dynamic_cast<AssetMetadata*>(metadata.get()))
			{
				if (assetMetadata->Handle == handle)
				{
					return assetMetadata;
				}
			}
		}
		return nullptr;
	}

	PathMetadata* AssetRegistry::OnPathCreated(const std::string& path, bool bIsAsset)
	{
		const auto parentPath = FileSystemUtils::GetParentPath(path);
		PathMetadata* metadata;
		if (bIsAsset)
		{
			metadata = AddAssetToTree(parentPath, path);
		}
		else
		{
			metadata = AddDirectoryToTree(parentPath, path);
		}
		SortPathTree();
		return metadata;
	}

	void AssetRegistry::OnTempPathCreated(const std::string& path, AssetTypeID typeID)
	{
		const auto parentPath = FileSystemUtils::GetParentPath(path);
		if (typeID)
		{
			// Called when a new asset is created in the Content Browser
			// Any actual asset file is not created yet so we have to pass typeID to metadata instead of parsing from the actual asset file
			AddAssetToTree(parentPath, path, typeID);
		}
		else
		{
			AddDirectoryToTree(parentPath, path);
		}
		SortPathTree();
	}

	void AssetRegistry::OnPathRemoved(const std::string& path)
	{
		// Possibly unload asset
		AssetLibrary::UnloadAsset(GetAssetHandleFromPath(path));
		RemovePathFromTree(path);
	}

	void AssetRegistry::OnPathRenamed(const std::string oldPath, const std::string newPath)
	{
		RenamePathInTree(FileSystemUtils::GetParentPath(oldPath), oldPath, newPath);
		SortPathTree();
	}

}
