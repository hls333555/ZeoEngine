#include "ZEpch.h"
#include "Engine/Core/AssetRegistry.h"

#include <filesystem>

#include "Engine/Core/EngineTypes.h"
#include "Engine/Debug/BenchmarkTimer.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/ThumbnailManager.h"

namespace ZeoEngine {

	namespace Utils {

		static std::string FileNameToUpperCase(const std::string& fileName)
		{
			std::string outStr = fileName;
			std::transform(outStr.begin(), outStr.end(), outStr.begin(), ::toupper);
			return outStr;
		}

	}

	void AssetRegistry::Init()
	{
		ConstructPathTree();
	}

	void AssetRegistry::ConstructPathTree()
	{
		BenchmarkTimer timer;

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
					AddPathToTree(baseDirectory, it.path().string());

					ConstructPathTreeRecursively(it.path().string());
					break;
				case std::filesystem::file_type::regular:
					if (it.path().extension().string() == GetEngineAssetExtension())
					{
						AddPathToTree(baseDirectory, it.path().string(), {});
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
	
	void AssetRegistry::AddPathToTree(const std::string& baseDirectory, const std::string& path)
	{
		m_PathTree.emplace_back(std::make_pair(path, std::vector<std::string>{}));
		auto directorySpec = CreateRef<DirectorySpec>(path, PathUtils::GetNameFromPath(path));
		m_PathSpecs[path] = std::move(directorySpec);
		GetPathSpec<DirectorySpec>(baseDirectory)->bHasAnySubDirectory = true;

		auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it != m_PathTree.end())
		{
			it->second.emplace_back(path);
		}
	}

	void AssetRegistry::AddPathToTree(const std::string& baseDirectory, const std::string& path, std::optional<AssetTypeId> optionalAssetTypeId)
	{
		auto assetSpec = CreateRef<AssetSpec>(path, PathUtils::GetNameFromPath(path));
		if (!optionalAssetTypeId)
		{
			auto parsedTypeId = PathUtils::ParseAssetTypeIdFromFile(path);
			if (!parsedTypeId)
			{
				ZE_CORE_WARN("Invalid asset detected! Asset path: {0}", path);
				return;
			}
			assetSpec->TypeId = *parsedTypeId;
		}
		else
		{
			assetSpec->TypeId = *optionalAssetTypeId;
		}
		assetSpec->ThumbnailTexture = ThumbnailManager::Get().GetAssetThumbnail(path, assetSpec->TypeId);
		m_AssetSpecsById[assetSpec->TypeId].emplace_back(assetSpec);
		m_PathSpecs[path] = std::move(assetSpec);

		auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (it != m_PathTree.end())
		{
			it->second.emplace_back(path);
		}
	}

	void AssetRegistry::RemovePathFromTree(const std::string& baseDirectory, const std::string& path)
	{
		auto parentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		ZE_CORE_ASSERT(parentPathIt != m_PathTree.end());
		auto currentPathInParentIt = std::find(parentPathIt->second.begin(), parentPathIt->second.end(), path);
		ZE_CORE_ASSERT(currentPathInParentIt != parentPathIt->second.end());
		parentPathIt->second.erase(currentPathInParentIt);
		auto baseDirectorySpec = GetPathSpec<DirectorySpec>(baseDirectory);
		ZE_CORE_ASSERT(baseDirectorySpec);
		baseDirectorySpec->bHasAnySubDirectory = false;
		for (const auto& subPath : parentPathIt->second)
		{
			if (GetPathSpec<DirectorySpec>(subPath))
			{
				baseDirectorySpec->bHasAnySubDirectory = true;
				break;
			}
		}

		auto currentPathIt = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&path](const auto& pair)
		{
			return pair.first == path;
		});
		// For assets, currentPathIt should be invalid
		if (currentPathIt != m_PathTree.end())
		{
			m_PathTree.erase(currentPathIt);
		}

		if (auto assetSpec = GetPathSpec<AssetSpec>(path))
		{
			auto& assetSpecs = m_AssetSpecsById[assetSpec->TypeId];
			auto specIt = std::find(assetSpecs.begin(), assetSpecs.end(), assetSpec);
			ZE_CORE_ASSERT(specIt != assetSpecs.end());
			assetSpecs.erase(specIt);
		}

		auto specIt = m_PathSpecs.find(path);
		ZE_CORE_ASSERT(specIt != m_PathSpecs.end());
		m_PathSpecs.erase(specIt);
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

	void AssetRegistry::OnPathCreated(const std::string& path, std::optional<AssetTypeId> optionalAssetTypeId)
	{
		std::string parentPath = PathUtils::GetParentPath(path);
		if (optionalAssetTypeId)
		{
			AddPathToTree(parentPath, path, optionalAssetTypeId);
		}
		else
		{
			AddPathToTree(parentPath, path);
		}
		SortPathTree();
	}

	void AssetRegistry::OnPathRemoved(const std::string& path)
	{
		std::string parentPath = PathUtils::GetParentPath(path);
		RemovePathFromTree(parentPath, path);
	}

	void AssetRegistry::OnPathRenamed(const std::string oldPath, const std::string newPath, bool bIsAsset)
	{
		std::string parentPath = PathUtils::GetParentPath(oldPath);
		RenamePathInTree(parentPath, oldPath, newPath, bIsAsset);
		SortPathTree();
	}

}
