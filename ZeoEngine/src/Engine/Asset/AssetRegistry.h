#pragma once

#include <optional>

#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Core/FileWatcher.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Asset/Asset.h"

namespace ZeoEngine {

	class Texture2D;

	enum EPathFlag : U8
	{
		PathFlag_None = 0,
		PathFlag_HasResource = ZE_BIT(0),
		PathFlag_Importable = ZE_BIT(1),
		PathFlag_Template = ZE_BIT(2),
	};

	struct PathMetadata
	{
		explicit PathMetadata(const std::filesystem::path& path)
			: Path(path), PathName(path.stem().string()) {}
		virtual ~PathMetadata() = default;

		virtual bool IsAsset() const = 0;
		virtual AssetTypeID GetAssetTypeID() const = 0;

		std::filesystem::path Path;
		std::string PathName;
		U8 Flags = 0;
		Ref<Texture2D> ThumbnailTexture;
	};

	struct DirectoryMetadata : public PathMetadata
	{
		using PathMetadata::PathMetadata;

		virtual bool IsAsset() const override { return false; }
		virtual AssetTypeID GetAssetTypeID() const override { return {}; }

		bool bIsTreeExpanded = false;
		U32 TreeNodeId = 0;
		bool bHasAnySubDirectory = false;
	};

	struct AssetMetadata : public PathMetadata, public std::enable_shared_from_this<AssetMetadata>
	{
		using PathMetadata::PathMetadata;

		virtual bool IsAsset() const override { return true; }
		virtual AssetTypeID GetAssetTypeID() const override { return TypeID; }

		bool IsResourceAsset() const { return Flags & PathFlag_HasResource; }
		bool IsImportableAsset() const { return Flags & PathFlag_Importable; }
		bool IsTemplateAsset() const { return Flags & PathFlag_Template; }
		std::filesystem::path GetResourcePath() const { return IsResourceAsset() ? Path.parent_path() / Path.stem() : std::filesystem::path{}; }

		void UpdateThumbnail();

		AssetHandle Handle = 0;
		AssetTypeID TypeID = {};
		std::filesystem::path SourcePath; // Can be empty if this is not an imported asset
	};

	class AssetRegistry
	{
		friend class EditorLayer;
		friend class ThumbnailManager;

	public:
		AssetRegistry(const AssetRegistry&) = delete;
		AssetRegistry& operator=(const AssetRegistry&) = delete;

		static AssetRegistry& Get()
		{
			static AssetRegistry instance;
			return instance;
		}

		// TODO: Move to other file
		static constexpr const char* GetAssetRootDirectory() { return "assets"; } // TODO: Change to project directory
		static constexpr const char* GetEngineAssetExtension() { return ".zasset"; }
		static constexpr const char* GetEditorRootDirectory() { return "assets/editor"; } // TODO:
		static constexpr const char* GetCPUProfileOutputDirectory() { return "saved/profiling/CPU/"; }
		static constexpr const char* GetGPUProfileOutputDirectory() { return "saved/profiling/GPU/"; }

		void OnUpdate(DeltaTime dt);

		/** Get directory/asset metadata of the specific path. The path should be relative. */
		template<typename T = PathMetadata>
		Ref<T> GetPathMetadata(const std::filesystem::path& path) const
		{
			if (const auto it = m_PathMetadatas.find(path); it != m_PathMetadatas.cend())
			{
				if constexpr (std::is_same_v<T, PathMetadata>)
				{
					return it->second;
				}
				else
				{
					return std::dynamic_pointer_cast<T>(it->second);
				}
			}

			return nullptr;
		}

		Ref<AssetMetadata> GetAssetMetadata(const std::filesystem::path& path) const;
		Ref<AssetMetadata> GetAssetMetadata(AssetHandle handle) const;

		AssetHandle GetAssetHandleFromPath(const std::filesystem::path& path) const;

		std::filesystem::path GetRelativePath(const std::filesystem::path& path) const;
		bool ContainsPathInDirectory(const std::filesystem::path& baseDirectory, const std::filesystem::path& path);
		const std::vector<std::filesystem::path>& GetPathsInDirectory(const std::filesystem::path& baseDirectory);

		template<typename Func>
		void ForEachPathInDirectory(const std::filesystem::path& baseDirectory, Func func)
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
		void ForEachPathInDirectoryRecursively(const std::filesystem::path& baseDirectory, Func func)
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
		void ForEachAssetByTypeID(AssetTypeID typeId, Func func)
		{
			if (const auto it = m_AssetMetadatasByID.find(typeId); it != m_AssetMetadatasByID.cend())
			{
				for (const auto& assetMetadata : it->second)
				{
					func(assetMetadata);
				}
			}
		}

		Ref<PathMetadata> OnPathCreated(const std::filesystem::path& path, bool bIsAsset);
		void OnTempAssetPathCreated(const std::filesystem::path& path, AssetTypeID typeID);
		void OnPathRemoved(const std::filesystem::path& path);
		/** NOTE: Here we pass path by value because we will then modify values in container directly which will affect these paths if passed by reference. */
		void OnPathRenamed(const std::filesystem::path oldPath, const std::filesystem::path newPath);

	private:
		void Init();

		void ConstructPathTree();
		void ConstructPathTreeRecursively(const std::filesystem::path& baseDirectory);

		/**
		 * Add a directory to the path tree.
		 * 
		 * @param baseDirectory - Parent path
		 * @param path - Path to add
		 * @return - Created directory metadata
		 */
		Ref<DirectoryMetadata> AddDirectoryToTree(const std::filesystem::path& baseDirectory, const std::filesystem::path& path);
		/**
		 * Add an asset to the path tree.
		 * 
		 * @param baseDirectory - Parent path
		 * @param path - Path to add
		 * @param typeID - If set, the asset is created in the Content Browser Panel
		 * @return - Created asset metadata
		 */
		Ref<AssetMetadata> AddAssetToTree(const std::filesystem::path& baseDirectory, const std::filesystem::path& path, AssetTypeID typeID = {});
		/**
		 * Remove a path from the path tree.
		 * If path is a directory, it will recursively remove its child paths.
		 * Path parameter should be copied during recursion.
		 * Returns the iterator next to the removed one.
		 */
		std::vector<std::filesystem::path>::iterator RemovePathFromTree(std::filesystem::path path);
		void RenamePathInTree(const std::filesystem::path& baseDirectory, const std::filesystem::path& oldPath, const std::filesystem::path& newPath);

		/**
		 * Sorting:
		 * Directories first, assets second
		 * In alphabetical order
		 */
		void SortPathTree();

		/** Called from file watcher callback on a separate thread. */
		void OnAssetModified(const std::filesystem::path& path);

	protected:
		AssetRegistry() = default;

	private:
		/** Map from base directory to list of its direct sub-paths in order */
		std::vector<std::pair<std::filesystem::path, std::vector<std::filesystem::path>>> m_PathTree;
		/** Map from path to path metadata */
		std::unordered_map<std::filesystem::path, Ref<PathMetadata>> m_PathMetadatas;
		/** Map from asset type id to list of asset metadata of this type */
		std::unordered_map<AssetTypeID, std::vector<Ref<AssetMetadata>>> m_AssetMetadatasByID;

		Scope<FileWatcher> m_FileWatcher;
		/** Stores a series of modified assets to be processed by the main thread */
		std::set<std::filesystem::path> m_PendingModifiedAssets;
		std::mutex m_Mutex;
	};

}
