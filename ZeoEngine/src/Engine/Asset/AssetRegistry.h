#pragma once

#include <optional>

#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Asset/Asset.h"
#include "Engine/Utils/FileSystemUtils.h"

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
		explicit PathMetadata(std::string path)
			: Path(std::move(path)), PathName(FileSystemUtils::GetPathName(Path)) {}
		virtual ~PathMetadata() = default;

		virtual bool IsAsset() const = 0;
		virtual AssetTypeID GetAssetTypeID() const = 0;

		std::string Path;
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

	// TODO: No shared_ptr
	struct AssetMetadata : public PathMetadata, public std::enable_shared_from_this<AssetMetadata>
	{
		using PathMetadata::PathMetadata;

		virtual bool IsAsset() const override { return true; }
		virtual AssetTypeID GetAssetTypeID() const override { return TypeID; }

		bool IsResourceAsset() const { return Flags & PathFlag_HasResource; }
		bool IsImportableAsset() const { return Flags & PathFlag_Importable; }
		bool IsTemplateAsset() const { return Flags & PathFlag_Template; }
		std::string GetResourceFileSystemPath() const { return IsResourceAsset() ? fmt::format("{}/{}", FileSystemUtils::GetParentPath(FileSystemUtils::GetFileSystemPath(Path)), PathName) : ""; }

		void UpdateThumbnail();

		AssetHandle Handle = 0;
		AssetTypeID TypeID = {};
		std::string SourcePath; // Can be empty if this is not an imported asset
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
		static constexpr const char* GetEngineAssetExtension() { return ".zasset"; }

		static std::string GetEnginePathPrefix() { return "Engine"; }
		static std::string GetProjectPathPrefix() { return "Game"; }
		static std::string GetEngineAssetDirectory() { return "assets"; } // TODO:
		static std::string GetProjectDirectory() { return "SandboxProject"; } // TODO:
		static std::string GetProjectAssetDirectory() { return "SandboxProject/Assets"; } // TODO:

		static std::string GetCPUProfileOutputDirectory() { return "saved/profiling/CPU/"; }
		static std::string GetGPUProfileOutputDirectory() { return "saved/profiling/GPU/"; }

		/** Get directory/asset metadata of the specific path. The path should be relative. */
		template<typename MetadataClass = PathMetadata>
		Ref<MetadataClass> GetPathMetadata(const std::string& path) const
		{
			static_assert(std::is_base_of_v<PathMetadata, MetadataClass>, "MetadataClass must be derived from 'PathMetadata'!");

			if (const auto it = m_PathMetadatas.find(path); it != m_PathMetadatas.cend())
			{
				return std::dynamic_pointer_cast<MetadataClass>(it->second);
			}

			return nullptr;
		}

		Ref<AssetMetadata> GetAssetMetadata(const std::string& path) const;
		Ref<AssetMetadata> GetAssetMetadata(AssetHandle handle) const;

		AssetHandle GetAssetHandleFromPath(const std::string& path) const;

		bool ContainsPathInDirectory(const std::string& baseDirectory, const std::string& path);
		const std::vector<std::string>& GetPathsInDirectory(const std::string& baseDirectory);

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

		/** If typeID is 0, all assets will be traversed. The signature of func should be "void(const Ref<AssetMetadata>&)". */
		template<typename Func>
		void ForEachAssetByTypeID(AssetTypeID typeID, Func func)
		{
			if (typeID)
			{
				if (const auto it = m_AssetMetadatasByID.find(typeID); it != m_AssetMetadatasByID.cend())
				{
					for (const auto& metadata : it->second)
					{
						func(metadata);
					}
				}
			}
			else
			{
				for (const auto& pair : m_AssetMetadatasByID)
				{
					for (const auto& metadata : pair.second)
					{
						func(metadata);
					}
				}
			}
		}

		Ref<PathMetadata> OnPathCreated(const std::string& path, bool bIsAsset);
		void OnTempPathCreated(const std::string& path, AssetTypeID typeID);
		void OnPathRemoved(const std::string& path);
		/** NOTE: Here we pass path by value because we will then modify values in container directly which will affect these paths if passed by reference. */
		void OnPathRenamed(const std::string oldPath, const std::string newPath);

	private:
		void Init();

		void ConstructPathTree(const std::filesystem::path& rootDirectory);
		void ConstructPathTreeRecursively(const std::filesystem::path& baseDirectory);

		/**
		 * Add a directory to the path tree.
		 * 
		 * @param baseDirectory - Parent path
		 * @param path - Path to add
		 * @return - Created directory metadata
		 */
		Ref<DirectoryMetadata> AddDirectoryToTree(const std::string& baseDirectory, const std::string& path);
		/**
		 * Add an asset to the path tree.
		 * 
		 * @param baseDirectory - Parent path
		 * @param path - Path to add
		 * @param typeID - If set, the asset is created in the Content Browser Panel
		 * @return - Created asset metadata
		 */
		Ref<AssetMetadata> AddAssetToTree(const std::string& baseDirectory, const std::string& path, AssetTypeID typeID = {});
		/**
		 * Remove a path from the path tree.
		 * If path is a directory, it will recursively remove its child paths.
		 * Path parameter should be copied during recursion.
		 * Returns the iterator next to the removed one.
		 */
		std::vector<std::string>::iterator RemovePathFromTree(std::string path);
		void RenamePathInTree(const std::string& baseDirectory, const std::string& oldPath, const std::string& newPath);

		/**
		 * Sorting:
		 * Directories first, assets second
		 * In alphabetical order
		 */
		void SortPathTree();

	protected:
		AssetRegistry() = default;

	private:
		/** Map from base directory to list of its direct sub-paths in order */
		std::vector<std::pair<std::string, std::vector<std::string>>> m_PathTree;
		/** Map from path to path metadata */
		std::unordered_map<std::string, Ref<PathMetadata>> m_PathMetadatas;
		/** Map from asset type id to list of asset metadata of this type */
		std::unordered_map<AssetTypeID, std::vector<Ref<AssetMetadata>>> m_AssetMetadatasByID;
	};

}
