#pragma once

#include <string>

#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	struct PathSpec
	{
		PathSpec(const std::string& path, const std::string& pathName)
			: Path(path), PathName(pathName) {}
		virtual ~PathSpec() = default;

		std::string Path;
		std::string PathName;
	};

	struct DirectorySpec : public PathSpec
	{
		using PathSpec::PathSpec;

		uint32_t TreeNodeId = 0;
		bool bHasAnySubDirectory = false;
	};

	struct AssetSpec : public PathSpec
	{
		using PathSpec::PathSpec;
		
		AssetTypeId TypeId;
		Ref<Texture2D> ThumbnailTexture;
	};

	class AssetRegistry
	{
	public:
		static AssetRegistry& Get()
		{
			static AssetRegistry instance;
			return instance;
		}

		static constexpr const char* GetAssetRootDirectory() { return "assets"; }
		static constexpr const char* GetEngineAssetExtension() { return ".zasset"; }

		void ConstructPathTree();

		template<typename T = PathSpec>
		Ref<T> GetPathSpec(const std::string& path) const
		{
			if (auto it = m_PathSpecs.find(path); it != m_PathSpecs.cend())
			{
				return std::dynamic_pointer_cast<T>(it->second);
			}

			return {};
		}

		bool ContainsPathInDirectory(const std::string& baseDirectory, const std::string& path);

		template<typename Func>
		void ForEachPathInDirectory(const std::string& baseDirectory, Func func)
		{
			auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
			{
				return pair.first == baseDirectory;
			});
			if (it == m_PathTree.end()) return;

			for (const auto& path : it->second)
			{
				func(path);
			}
		}

		template<typename Func>
		void ForEachPathInDirectoryRecursively(const std::string& baseDirectory, Func func)
		{
			auto it = std::find_if(m_PathTree.begin(), m_PathTree.end(), [&baseDirectory](const auto& pair)
			{
				return pair.first == baseDirectory;
			});
			if (it == m_PathTree.end()) return;

			for (const auto& path : it->second)
			{
				func(path);
				ForEachPathInDirectoryRecursively(path, func);
			}
		}

		template<typename Func>
		void ForEachAssetByTypeId(AssetTypeId typeId, Func func)
		{
			if (auto it = m_AssetSpecsById.find(typeId); it != m_AssetSpecsById.cend())
			{
				for (const auto& assetSpec : it->second)
				{
					func(assetSpec);
				}
			}
		}

		void OnPathCreated(const std::string& path, bool bIsAsset);
		void OnPathRemoved(const std::string& path);
		/** NOTE: Here we pass string by value because we will then modify values in container directly which will affect these strings if passed by reference. */
		void OnPathRenamed(const std::string oldPath, const std::string newPath, bool bIsAsset);

	private:
		void ConstructPathTreeRecursively(const std::string& baseDirectory);

		/**
		 * Add a path to the path tree.
		 * 
		 * @param baseDirectory - Parent path
		 * @param path - Path to add
		 * @param bIsAsset - This path is an asset rather than a directory
		 * @param bIsTemp - Indicates this function is called during path creation. The real path is not constructed yet.
		 */
		void AddPathToTree(const std::string& baseDirectory, const std::string& path, bool bIsAsset, bool bIsTemp);
		void RemovePathFromTree(const std::string& baseDirectory, const std::string& path);
		void RenamePathInTree(const std::string& baseDirectory, const std::string& oldPath, const std::string& newPath, bool bIsAsset);

		/**
		 * Sorting:
		 * Directories first, assets second
		 * In alphabetical order
		 */
		void SortPathTree();

	protected:
		AssetRegistry() = default;
		AssetRegistry(const AssetRegistry&) = delete;
		AssetRegistry& operator=(const AssetRegistry&) = delete;

	private:
		/** Map from base directory to list of its direct sub-paths in order */
		std::vector<std::pair<std::string, std::vector<std::string>>> m_PathTree;
		/** Map from path string to path spec */
		std::unordered_map<std::string, Ref<PathSpec>> m_PathSpecs;
		/** Map from asset type id to list of asset spec of this type */
		std::unordered_map<AssetTypeId, std::vector<Ref<AssetSpec>>> m_AssetSpecsById;
	};

}
