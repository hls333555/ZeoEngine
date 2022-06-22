#pragma once

#include <optional>

#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Core/FileWatcher.h"
#include "Engine/Core/DeltaTime.h"

namespace ZeoEngine {

	class Texture2D;

	enum EPathFlag : U8
	{
		PathFlag_None = 0,
		PathFlag_HasResource = ZE_BIT(0),
		PathFlag_Importable = ZE_BIT(1),
	};

	struct PathSpec
	{
		PathSpec(const std::string& path, const std::string& pathName)
			: Path(path), PathName(pathName) {}
		virtual ~PathSpec() = default;

		virtual bool IsAsset() const = 0;
		virtual bool IsResourceAsset() const = 0;
		virtual bool IsImportableAsset() const = 0;
		virtual AssetTypeId GetAssetTypeId() const = 0;
		virtual std::string GetSourcePath() const = 0;

		std::string Path;
		std::string PathName;
		U8 Flags = 0;
		Ref<Texture2D> ThumbnailTexture;
	};

	struct DirectorySpec : public PathSpec
	{
		using PathSpec::PathSpec;

		virtual bool IsAsset() const override { return false; }
		virtual bool IsResourceAsset() const override { return false; }
		virtual bool IsImportableAsset() const override { return false; }
		virtual AssetTypeId GetAssetTypeId() const override { return {}; }
		virtual std::string GetSourcePath() const override { return {}; }

		bool bIsTreeExpanded = false;
		U32 TreeNodeId = 0;
		bool bHasAnySubDirectory = false;
	};

	struct AssetSpec : public PathSpec
	{
		using PathSpec::PathSpec;

		virtual bool IsAsset() const override { return true; }
		virtual bool IsResourceAsset() const override { return Flags & PathFlag_HasResource; }
		virtual bool IsImportableAsset() const override { return Flags & PathFlag_Importable; }
		virtual AssetTypeId GetAssetTypeId() const override { return TypeId; }
		virtual std::string GetSourcePath() const override { return SourcePath; }

		void UpdateThumbnail();
		
		AssetTypeId TypeId;
		std::string SourcePath; // Can be empty if this is not an imported asset
	};

	class AssetRegistry
	{
		friend class EditorLayer;
		friend class ThumbnailManager;

	public:
		static AssetRegistry& Get()
		{
			static AssetRegistry instance;
			return instance;
		}

		static constexpr const char* GetAssetRootDirectory() { return "assets"; }
		static constexpr const char* GetEngineAssetExtension() { return ".zasset"; }
		static constexpr const char* GetEditorRootDirectory() { return "assets/editor"; } // TODO:

		void OnUpdate(DeltaTime dt);

		/** Get directory/asset spec of the specific path. The path should be relative. */
		template<typename T = PathSpec>
		Ref<T> GetPathSpec(const std::string& path) const
		{
			if (auto it = m_PathSpecs.find(path); it != m_PathSpecs.cend())
			{
				if constexpr (std::is_same<T, PathSpec>::value)
				{
					return it->second;
				}
				else
				{
					return std::dynamic_pointer_cast<T>(it->second);
				}
			}

			return {};
		}

		bool ContainsPathInDirectory(const std::string& baseDirectory, const std::string& path);
		const std::vector<std::string>& GetPathsInDirectory(const std::string& directory);

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

		/** Returns the created directory/asset spec. */
		Ref<PathSpec> OnPathCreated(const std::string& path, AssetTypeId typeId);
		void OnPathRemoved(const std::string& path);
		/** NOTE: Here we pass string by value because we will then modify values in container directly which will affect these strings if passed by reference. */
		void OnPathRenamed(const std::string oldPath, const std::string newPath, bool bIsAsset);

	private:
		void Init();

		void ConstructPathTree();
		void ConstructPathTreeRecursively(const std::string& baseDirectory);

		/**
		 * Add a directory to the path tree.
		 * 
		 * @param baseDirectory - Parent path
		 * @param path - Path to add
		 * @return - Created directory spec
		 */
		Ref<DirectorySpec> AddDirectoryToTree(const std::string& baseDirectory, const std::string& path);
		/**
		 * Add an asset to the path tree.
		 * 
		 * @param baseDirectory - Parent path
		 * @param path - Path to add
		 * @param optionalAssetTypeId - If not set, type id is retrieved from file
		 * @return - Created asset spec
		 */
		Ref<AssetSpec> AddAssetToTree(const std::string& baseDirectory, const std::string& path, std::optional<AssetTypeId> optionalAssetTypeId);
		/**
		 * Remove a path from the path tree.
		 * If path is a directory, it will recursively remove its child paths.
		 * Path parameter should be copied during recursion.
		 * Returns the iterator next to the removed one.
		 */
		std::vector<std::string>::iterator RemovePathFromTree(std::string path);
		void RenamePathInTree(const std::string& baseDirectory, const std::string& oldPath, const std::string& newPath, bool bIsAsset);

		/**
		 * Sorting:
		 * Directories first, assets second
		 * In alphabetical order
		 */
		void SortPathTree();

		/** Called from file watcher callback on a separate thread. */
		void OnAssetModified(const std::string& path);

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

		Scope<FileWatcher> m_FileWatcher;
		/** Stores a series of modified assets to be processed by the main thread */
		std::set<std::string> m_PendingModifiedAssets;
		std::mutex m_Mutex;
	};

}
