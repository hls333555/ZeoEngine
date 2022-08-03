#pragma once

#include <unordered_map>

#include "Engine/Core/Core.h"
#include "Engine/Asset/Asset.h"

namespace ZeoEngine {

	class AssetFactoryBase;
	class AssetActionsBase;
	class AssetSerializerBase;
	struct AssetMetadata;

	class AssetManager
	{
		friend class EditorLayer;
		friend class FileDialogs;

	public:
		static AssetManager& Get()
		{
			static AssetManager instance;
			return instance;
		}

		/** Register asset factory. Returns true if the registration succeeded, false otherwise. */
		bool RegisterAssetFactory(AssetTypeID typeID, Ref<AssetFactoryBase> factory);
		/** Register asset actions. Returns true if the registration succeeded, false otherwise. */
		bool RegisterAssetActions(AssetTypeID typeID, Ref<AssetActionsBase> actions);
		/** Register asset serializer. Returns true if the registration succeeded, false otherwise. */
		bool RegisterAssetSerializer(AssetTypeID typeID, Ref<AssetSerializerBase> serializer);

		/** Create a new empty asset file. */
		bool CreateAssetFile(AssetTypeID typeID, const std::filesystem::path& path) const;
		/** Import a new resource from file dialog or dragging by copying it from srcPath to destPath. */
		bool ImportAsset(AssetTypeID typeID, const std::filesystem::path& srcPath, const std::filesystem::path& destPath) const;
		/** Construct a new asset. */
		Ref<IAsset> CreateAsset(const Ref<AssetMetadata>& metadata) const;
		/** Open an existing asset. */
		bool OpenAsset(const std::filesystem::path& path) const;
		/** Rename an existing asset. */
		bool RenameAsset(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) const;
		/** Delete an existing asset. */
		bool DeleteAsset(const std::filesystem::path& path) const;
		/** Save an existing asset. */
		bool SaveAsset(const std::filesystem::path& path, const Ref<IAsset>& asset) const;
		/** Reimport an existing asset from its source place. */
		bool ReimportAsset(const std::filesystem::path& path) const;

		Ref<AssetFactoryBase> GetAssetFactoryByAssetType(AssetTypeID typeID);
		Ref<AssetActionsBase> GetAssetActionsByAssetType(AssetTypeID typeID);
		Ref<AssetSerializerBase> GetAssetSerializerByAssetType(AssetTypeID typeID);

		template <typename Func>
		void ForEachAssetFactory(Func func) const
		{
			static_assert(std::is_invocable_v<Func, Ref<AssetFactoryBase>> || std::is_invocable_v<Func, AssetTypeID, Ref<AssetFactoryBase>>, "Failed to find the matching func for call: ForEachAssetFactory!");

			auto begin = m_AssetFactories.begin();
			auto end = m_AssetFactories.end();

			while (begin != end)
			{
				auto curr = begin++;

				if constexpr (std::is_invocable_v<Func, Ref<AssetFactoryBase>>)
				{
					func(curr->second);
				}
				else if constexpr (std::is_invocable_v<Func, AssetTypeID, Ref<AssetFactoryBase>>)
				{
					func(curr->first, curr->second);
				}
			}
		}

		template <typename Func>
		void ForEachAssetType(Func func) const
		{
			static_assert(std::is_invocable_v<Func, AssetTypeID>, "Failed to find the matching func for call: ForEachAssetType!");
			auto begin = m_AssetFactories.begin();
			auto end = m_AssetFactories.end();

			while (begin != end)
			{
				auto curr = begin++;
				func(curr->first);
			}
		}

		AssetTypeID GetAssetTypeFromFileExtension(const std::string& extension) const;

	private:
		void Init();
		void InitSupportedFileExtensions();

	protected:
		AssetManager() = default;
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

	private:
		/** Map from asset type ID to asset factory */
		std::unordered_map<AssetTypeID, Ref<AssetFactoryBase>> m_AssetFactories;
		/** Map from asset type ID to asset actions */
		std::unordered_map<AssetTypeID, Ref<AssetActionsBase>> m_AssetActions;
		/** Map from asset type ID to asset serializers */
		std::unordered_map<AssetTypeID, Ref<AssetSerializerBase>> m_AssetSerializers;

		/** Map from supported file extension to asset type ID */
		std::unordered_map<std::string, AssetTypeID, CaseInsensitiveUnorderedMap::Hash, CaseInsensitiveUnorderedMap::Comp> m_SupportedFileExtensions;
	};

}
