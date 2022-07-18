#include "ZEpch.h"
#include "Engine/Core/AssetRegistry.h"

#include "Engine/Core/EngineTypes.h"
#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/Serializer.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Core/AssetActions.h"
#include "Engine/Core/AssetManager.h"

namespace ZeoEngine {

	namespace Utils {

		static std::string FileNameToUpperCase(const std::string& fileName)
		{
			std::string outStr = fileName;
			std::transform(outStr.begin(), outStr.end(), outStr.begin(), ::toupper);
			return outStr;
		}

		static bool ParseAssetFile(const std::string& path, const Ref<AssetSpec>& assetSpec)
		{
			auto data = Serializer::ReadDataFromAsset(path);
			if (!data) return false;

			auto assetTypeData = (*data)[g_AssetTypeToken];
			if (!assetTypeData) return false;

			// Record asset type id
			assetSpec->TypeId = assetTypeData.as<AssetTypeId>();

			// Possibly mark certain flags
			auto assetActions = AssetManager::Get().GetAssetActionsByAssetType(assetSpec->TypeId);
			if (std::dynamic_pointer_cast<ResourceAssetActionsBase>(assetActions))
			{
				assetSpec->Flags |= PathFlag_HasResource;
			}
			if (std::dynamic_pointer_cast<ImportableAssetActionsBase>(assetActions))
			{
				assetSpec->Flags |= PathFlag_Importable;
			}

			// Possibly record source path
			auto sourceData = (*data)[g_SourceToken];
			if (sourceData)
			{
				assetSpec->SourcePath = sourceData.as<std::string>();
			}

			return true;
		}

	}

	void AssetSpec::UpdateThumbnail()
	{
		ThumbnailTexture = ThumbnailManager::Get().GetAssetThumbnail(Path, TypeId);
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

		m_PathTree.emplace_back(std::make_pair(GetAssetRootDirectory(), std::vector<std::string>{}));
		m_PathSpecs[GetAssetRootDirectory()] = CreateRef<DirectorySpec>(GetAssetRootDirectory(), GetAssetRootDirectory());

		ConstructPathTreeRecursively(GetAssetRootDirectory());

		SortPathTree();

		ZE_CORE_WARN("Path tree construction took {0} ms", timer.ElapsedMillis());
	}

	void AssetRegistry::ConstructPathTreeRecursively(const std::string& baseDirectory)
	{
		std::filesystem::directory_iterator list(baseDirectory);
		for (auto& it : list)
		{
			switch (it.status().type())
			{
				case std::filesystem::file_type::directory:
					AddDirectoryToTree(baseDirectory, it.path().string());

					ConstructPathTreeRecursively(it.path().string());
					break;
				case std::filesystem::file_type::regular:
					if (it.path().extension().string() == GetEngineAssetExtension())
					{
						AddAssetToTree(baseDirectory, it.path().string(), {});
					}
					break;
			}
		}
	}

	bool AssetRegistry::ContainsPathInDirectory(const std::string& baseDirectory, const std::string& path)
	{
		auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it == m_PathTree.end()) return false;

		return std::find(it->second.begin(), it->second.end(), path) != it->second.end();
	}
	
	const std::vector<std::string>& AssetRegistry::GetPathsInDirectory(const std::string& directory)
	{
		auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&directory](const auto& pair)
		{
			return pair.first == directory;
		});
		ZE_CORE_ASSERT(it != m_PathTree.end());

		return it->second;
	}

	Ref<DirectorySpec> AssetRegistry::AddDirectoryToTree(const std::string& baseDirectory, const std::string& path)
	{
		m_PathTree.emplace_back(std::make_pair(path, std::vector<std::string>{}));
		auto directorySpec = CreateRef<DirectorySpec>(path, PathUtils::GetNameFromPath(path));
		directorySpec->ThumbnailTexture = ThumbnailManager::Get().GetDirectoryIcon();
		m_PathSpecs[path] = directorySpec;
		GetPathSpec<DirectorySpec>(baseDirectory)->bHasAnySubDirectory = true;

		auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it != m_PathTree.end())
		{
			it->second.emplace_back(path);
		}

		return directorySpec;
	}

	Ref<AssetSpec> AssetRegistry::AddAssetToTree(const std::string& baseDirectory, const std::string& path, std::optional<AssetTypeId> optionalAssetTypeId)
	{
		auto assetSpec = CreateRef<AssetSpec>(path, PathUtils::GetNameFromPath(path));
		if (!optionalAssetTypeId)
		{
			bool bResult = Utils::ParseAssetFile(path, assetSpec);
			if (!bResult)
			{
				ZE_CORE_WARN("Invalid asset detected! Asset path: {0}", path);
				return {};
			}
		}
		else
		{
			assetSpec->TypeId = *optionalAssetTypeId;
		}
		// TODO: Optimize: No need to load all textures
		assetSpec->UpdateThumbnail();
		m_AssetSpecsById[assetSpec->TypeId].emplace_back(assetSpec);
		m_PathSpecs[path] = assetSpec;

		auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it != m_PathTree.end())
		{
			it->second.emplace_back(path);
		}

		return assetSpec;
	}

	std::vector<std::string>::iterator AssetRegistry::RemovePathFromTree(std::string path)
	{
		if (auto spec = GetPathSpec(path); spec->IsAsset())
		{
			auto& assetSpecs = m_AssetSpecsById[spec->GetAssetTypeId()];
			auto specIt = std::find(assetSpecs.begin(), assetSpecs.end(), spec);
			ZE_CORE_ASSERT(specIt != assetSpecs.end());
			assetSpecs.erase(specIt);
		}

		auto currentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&path](const auto& pair)
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

		std::string parentPath = PathUtils::GetParentPath(path);
		auto parentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&parentPath](const auto& pair)
		{
			return pair.first == parentPath;
		});
		ZE_CORE_ASSERT(parentPathIt != m_PathTree.end());
		auto currentPathInParentIt = std::find(parentPathIt->second.begin(), parentPathIt->second.end(), path);
		ZE_CORE_ASSERT(currentPathInParentIt != parentPathIt->second.end());
		// Return the iterator for next loop use
		auto retIt = parentPathIt->second.erase(currentPathInParentIt);
		auto parentPathSpec = GetPathSpec<DirectorySpec>(parentPath);
		ZE_CORE_ASSERT(parentPathSpec);
		parentPathSpec->bHasAnySubDirectory = false;
		for (const auto& subPath : parentPathIt->second)
		{
			if (GetPathSpec<DirectorySpec>(subPath))
			{
				parentPathSpec->bHasAnySubDirectory = true;
				break;
			}
		}

		auto specIt = m_PathSpecs.find(path);
		ZE_CORE_ASSERT(specIt != m_PathSpecs.end());
		m_PathSpecs.erase(specIt);

		return retIt;
	}

	void AssetRegistry::RenamePathInTree(const std::string& baseDirectory, const std::string& oldPath, const std::string& newPath, bool bIsAsset)
	{
		auto parentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		ZE_CORE_ASSERT(parentPathIt != m_PathTree.end());
		auto currentPathInParentIt = std::find(parentPathIt->second.begin(), parentPathIt->second.end(), oldPath);
		ZE_CORE_ASSERT(currentPathInParentIt != parentPathIt->second.end());
		*currentPathInParentIt = newPath;

		auto currentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&oldPath](const auto& pair)
		{
			return pair.first == oldPath;
		});
		// For assets, currentPathIt should be invalid
		if (currentPathIt != m_PathTree.end())
		{
			currentPathIt->first = newPath;
		}

		m_PathSpecs[oldPath]->Path = newPath;
		m_PathSpecs[oldPath]->PathName = PathUtils::GetNameFromPath(newPath);
		// C++17 way of modifying key of a map
		auto node = m_PathSpecs.extract(oldPath);
		node.key() = newPath;
		m_PathSpecs.insert(std::move(node));
	}

	void AssetRegistry::SortPathTree()
	{
		for (auto& [baseDirectory, subPaths] : m_PathTree)
		{
			std::sort(subPaths.begin(), subPaths.end(), [this](const auto& lhs, const auto& rhs)
			{
				bool bIsLhsDirectory = static_cast<bool>(std::dynamic_pointer_cast<DirectorySpec>(m_PathSpecs[lhs]));
				bool bIsRhsDirectory = static_cast<bool>(std::dynamic_pointer_cast<DirectorySpec>(m_PathSpecs[rhs]));
				return bIsLhsDirectory && !bIsRhsDirectory ||
					(((bIsLhsDirectory && bIsRhsDirectory) || (!bIsLhsDirectory && !bIsRhsDirectory)) &&
					// Default comparison behaviour for directories: "NewFolder" is ahead of "cache"
					// So we have to convert file name to upper or lower case before comparing
					// Comparison behaviour after conversion: "cache" is ahead of "NewFolder"
					Utils::FileNameToUpperCase(lhs) < Utils::FileNameToUpperCase(rhs));
			});
		}
		std::sort(m_PathTree.begin(), m_PathTree.end(), [this](const auto& lhs, const auto& rhs)
		{
			bool bIsLhsDirectory = static_cast<bool>(std::dynamic_pointer_cast<DirectorySpec>(m_PathSpecs[lhs.first]));
			bool bIsRhsDirectory = static_cast<bool>(std::dynamic_pointer_cast<DirectorySpec>(m_PathSpecs[rhs.first]));
			return bIsLhsDirectory && !bIsRhsDirectory ||
				(((bIsLhsDirectory && bIsRhsDirectory) || (!bIsLhsDirectory && !bIsRhsDirectory)) &&
				Utils::FileNameToUpperCase(lhs.first) < Utils::FileNameToUpperCase(rhs.first));
		});
	}

	void AssetRegistry::OnAssetModified(const std::string& path)
	{
		m_PendingModifiedAssets.emplace(path);
	}

	void AssetRegistry::OnUpdate(DeltaTime dt)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		for (auto it = m_PendingModifiedAssets.begin(); it != m_PendingModifiedAssets.end();)
		{
			std::string assetPath = *it;
			if (!GetPathSpec(*it))
			{
				assetPath = PathUtils::GetNormalizedAssetPath(*it);
			}
			ZE_CORE_ASSERT(PathUtils::DoesPathExist(assetPath));
			AssetManager::Get().HotReloadAsset(assetPath);
			it = m_PendingModifiedAssets.erase(it);
		}
	}

	Ref<PathSpec> AssetRegistry::OnPathCreated(const std::string& path, AssetTypeId typeId)
	{
		std::string parentPath = PathUtils::GetParentPath(path);
		Ref<PathSpec> spec;
		if (typeId)
		{
			spec = AddAssetToTree(parentPath, path, typeId);
		}
		else
		{
			spec = AddDirectoryToTree(parentPath, path);
		}
		SortPathTree();
		return spec;
	}

	void AssetRegistry::OnPathRemoved(const std::string& path)
	{
		RemovePathFromTree(path);
	}

	void AssetRegistry::OnPathRenamed(const std::string oldPath, const std::string newPath, bool bIsAsset)
	{
		std::string parentPath = PathUtils::GetParentPath(oldPath);
		RenamePathInTree(parentPath, oldPath, newPath, bIsAsset);
		SortPathTree();
	}

}
