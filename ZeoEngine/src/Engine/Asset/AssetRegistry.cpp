#include "ZEpch.h"
#include "Engine/Asset/AssetRegistry.h"

#include "Engine/Core/EngineTypes.h"
#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Asset/AssetSerializer.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Asset/AssetActions.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Asset/AssetLibrary.h"

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

			// Possibly mark certain flags
			const auto assetActions = AssetManager::Get().GetAssetActionsByAssetType(metadata->TypeID);
			if (std::dynamic_pointer_cast<ResourceAssetActionsBase>(assetActions))
			{
				metadata->Flags |= PathFlag_HasResource;
			}
			if (std::dynamic_pointer_cast<ImportableAssetActionsBase>(assetActions))
			{
				metadata->Flags |= PathFlag_Importable;
			}

			// Possibly retrieve source path
			if (const auto sourceData = node["SourcePath"])
			{
				metadata->SourcePath = sourceData.as<std::filesystem::path>();
			}

			return true;
		}

		/** Returns true if provided path is in the editor directory. */
		// https://stackoverflow.com/questions/67144806/c-check-if-path-is-outside-a-given-directory
		static bool IsEditorPath(const std::filesystem::path& path)
		{
			const std::string relativePath = std::filesystem::relative(path, AssetRegistry::GetEditorRootDirectory()).string();
			// Size check for "." result
			// If path starts with ".." it's not subdir
			return relativePath.size() == 1 || relativePath[0] != '.' && relativePath[1] != '.';
		}

	}

	void AssetMetadata::UpdateThumbnail()
	{
		ThumbnailTexture = ThumbnailManager::Get().GetAssetThumbnail(shared_from_this());
	}

	void AssetRegistry::Init()
	{
		m_FileWatcher = CreateScope<FileWatcher>(GetAssetRootDirectory(), std::chrono::duration<I32, std::milli>(1000));
		m_FileWatcher->m_OnFileModified.connect<&AssetRegistry::OnAssetModified>(this);
		ConstructPathTree();
	}

	void AssetRegistry::ConstructPathTree()
	{
		Timer timer;

		m_PathTree.emplace_back(std::make_pair(GetAssetRootDirectory(), std::vector<std::filesystem::path>{}));
		m_PathMetadatas[GetAssetRootDirectory()] = CreateRef<DirectoryMetadata>(GetAssetRootDirectory());

		ConstructPathTreeRecursively(GetAssetRootDirectory());

		SortPathTree();

		ZE_CORE_WARN("Path tree construction took {0} ms", timer.ElapsedMillis());
	}

	void AssetRegistry::ConstructPathTreeRecursively(const std::filesystem::path& baseDirectory)
	{
		for (const auto& it : std::filesystem::directory_iterator(baseDirectory))
		{
			switch (it.status().type())
			{
				case std::filesystem::file_type::directory:
					AddDirectoryToTree(baseDirectory, it.path());

					ConstructPathTreeRecursively(it.path());
					break;
				case std::filesystem::file_type::regular:
					if (it.path().extension().string() == GetEngineAssetExtension())
					{
						AddAssetToTree(baseDirectory, it.path());
					}
					break;
				default:
					break;
			}
		}
	}

	AssetHandle AssetRegistry::GetAssetHandleFromPath(const std::filesystem::path& path) const
	{
		const auto metadata = GetAssetMetadata(path);
		return metadata ? metadata->Handle : 0;
	}

	std::filesystem::path AssetRegistry::GetRelativePath(const std::filesystem::path& path) const
	{
		const std::string temp = path.string();
		if (temp.find(GetAssetRootDirectory()) != std::string::npos)
		{
			return std::filesystem::relative(path, GetAssetRootDirectory());
		}
		return path;
	}

	bool AssetRegistry::ContainsPathInDirectory(const std::filesystem::path& baseDirectory, const std::filesystem::path& path)
	{
		const auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it == m_PathTree.end()) return false;

		return std::find(it->second.begin(), it->second.end(), path) != it->second.end();
	}
	
	const std::vector<std::filesystem::path>& AssetRegistry::GetPathsInDirectory(const std::filesystem::path& baseDirectory)
	{
		const auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		ZE_CORE_ASSERT(it != m_PathTree.end());

		return it->second;
	}

	Ref<DirectoryMetadata> AssetRegistry::AddDirectoryToTree(const std::filesystem::path& baseDirectory, const std::filesystem::path& path)
	{
		m_PathTree.emplace_back(std::make_pair(path, std::vector<std::filesystem::path>{}));
		auto metadata = CreateRef<DirectoryMetadata>(path);
		metadata->ThumbnailTexture = ThumbnailManager::Get().GetDirectoryIcon();
		m_PathMetadatas[path] = metadata;
		GetPathMetadata<DirectoryMetadata>(baseDirectory)->bHasAnySubDirectory = true;

		const auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it != m_PathTree.end())
		{
			it->second.emplace_back(path);
		}

		return metadata;
	}

	Ref<AssetMetadata> AssetRegistry::AddAssetToTree(const std::filesystem::path& baseDirectory, const std::filesystem::path& path, AssetTypeID typeID)
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
				return pair.second->IsAsset() && std::dynamic_pointer_cast<AssetMetadata>(pair.second)->Handle == metadata->Handle;
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

		if (Utils::IsEditorPath(path))
		{
			metadata->Flags |= PathFlag_Template;
		}
		// TODO: Optimize: No need to load all textures
		metadata->UpdateThumbnail();
		m_AssetMetadatasByID[metadata->TypeID].emplace_back(metadata);
		m_PathMetadatas[path] = metadata;

		const auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it != m_PathTree.end())
		{
			it->second.emplace_back(path);
		}

		return metadata;
	}

	std::vector<std::filesystem::path>::iterator AssetRegistry::RemovePathFromTree(std::filesystem::path path)
	{
		if (const auto metadata = GetAssetMetadata(path))
		{
			auto& assetMetadatas = m_AssetMetadatasByID[metadata->TypeID];
			const auto it = std::find(assetMetadatas.begin(), assetMetadatas.end(), metadata);
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

		const auto parentPath = path.parent_path();
		const auto parentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&parentPath](const auto& pair)
		{
			return pair.first == parentPath;
		});
		ZE_CORE_ASSERT(parentPathIt != m_PathTree.end());
		const auto currentPathInParentIt = std::find(parentPathIt->second.begin(), parentPathIt->second.end(), path);
		ZE_CORE_ASSERT(currentPathInParentIt != parentPathIt->second.end());
		// Return the iterator for next loop use
		auto retIt = parentPathIt->second.erase(currentPathInParentIt);
		const auto parentPathMetadata = GetPathMetadata<DirectoryMetadata>(parentPath);
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

		const auto it = m_PathMetadatas.find(path);
		ZE_CORE_ASSERT(it != m_PathMetadatas.end());
		m_PathMetadatas.erase(it);

		return retIt;
	}

	void AssetRegistry::RenamePathInTree(const std::filesystem::path& baseDirectory, const std::filesystem::path& oldPath, const std::filesystem::path& newPath)
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
		m_PathMetadatas[oldPath]->PathName = newPath.stem().string();
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
				const bool bIsLhsDirectory = m_PathMetadatas[lhs]->IsAsset();
				const bool bIsRhsDirectory = m_PathMetadatas[rhs]->IsAsset();
				return !bIsLhsDirectory && bIsRhsDirectory ||
					(((!bIsLhsDirectory && !bIsRhsDirectory) || (bIsLhsDirectory && bIsRhsDirectory)) &&
					// Default comparison behaviour for directories: "NewFolder" is ahead of "cache"
					// So we have to convert file name to upper or lower case before comparing
					// Comparison behaviour after conversion: "cache" is ahead of "NewFolder"
					Utils::FileNameToUpperCase(lhs.string()) < Utils::FileNameToUpperCase(rhs.string()));
			});
		}
		std::sort(m_PathTree.begin(), m_PathTree.end(), [this](const auto& lhs, const auto& rhs)
		{
			const bool bIsLhsDirectory = m_PathMetadatas[lhs.first]->IsAsset();
			const bool bIsRhsDirectory = m_PathMetadatas[rhs.first]->IsAsset();
			return !bIsLhsDirectory && bIsRhsDirectory ||
				(((!bIsLhsDirectory && !bIsRhsDirectory) || (bIsLhsDirectory && bIsRhsDirectory)) &&
				Utils::FileNameToUpperCase(lhs.first.string()) < Utils::FileNameToUpperCase(rhs.first.string()));
		});
	}

	void AssetRegistry::OnAssetModified(const std::filesystem::path& path)
	{
		m_PendingModifiedAssets.emplace(path);
	}

	void AssetRegistry::OnUpdate(DeltaTime dt)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		for (auto it = m_PendingModifiedAssets.begin(); it != m_PendingModifiedAssets.end();)
		{
			auto path = *it;
			// Currently only resource hot-reloading is supported
			if (path.extension().string().c_str() != GetEngineAssetExtension())
			{
				path += GetEngineAssetExtension();
				AssetLibrary::ReloadAsset(path);
			}
			it = m_PendingModifiedAssets.erase(it);
		}
	}

	Ref<AssetMetadata> AssetRegistry::GetAssetMetadata(const std::filesystem::path& path) const
	{
		if (const auto it = m_PathMetadatas.find(path); it != m_PathMetadatas.cend())
		{
			return std::dynamic_pointer_cast<AssetMetadata>(it->second);
		}
		return nullptr;
	}

	Ref<AssetMetadata> AssetRegistry::GetAssetMetadata(AssetHandle handle) const
	{
		if (!handle) return nullptr;

		for (const auto& [path, metadata] : m_PathMetadatas)
		{
			if (auto assetMetadata = std::dynamic_pointer_cast<AssetMetadata>(metadata))
			{
				if (assetMetadata->Handle == handle)
				{
					return assetMetadata;
				}
			}
		}
		return nullptr;
	}

	Ref<PathMetadata> AssetRegistry::OnPathCreated(const std::filesystem::path& path, bool bIsAsset)
	{
		const auto parentPath = path.parent_path();
		Ref<PathMetadata> metadata;
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

	void AssetRegistry::OnTempAssetPathCreated(const std::filesystem::path& path, AssetTypeID typeID)
	{
		AddAssetToTree(path.parent_path(), path, typeID);
		SortPathTree();
	}

	void AssetRegistry::OnPathRemoved(const std::filesystem::path& path)
	{
		RemovePathFromTree(path);
	}

	void AssetRegistry::OnPathRenamed(const std::filesystem::path oldPath, const std::filesystem::path newPath)
	{
		RenamePathInTree(oldPath.parent_path(), oldPath, newPath);
		SortPathTree();
	}

}
