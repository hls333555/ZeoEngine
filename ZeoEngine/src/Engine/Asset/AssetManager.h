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
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;
		~AssetManager();

		static AssetManager& Get()
		{
			static AssetManager instance;
			return instance;
		}

		/** Register asset factory. Returns true if the registration succeeded, false otherwise. */
		bool RegisterAssetFactory(AssetTypeID typeID, Scope<AssetFactoryBase> factory);
		/** Register asset actions. Returns true if the registration succeeded, false otherwise. */
		bool RegisterAssetActions(AssetTypeID typeID, Scope<AssetActionsBase> actions);
		/** Register asset serializer. Returns true if the registration succeeded, false otherwise. */
		bool RegisterAssetSerializer(AssetTypeID typeID, Scope<AssetSerializerBase> serializer);

		/** Create a new empty asset file. */
		bool CreateAssetFile(AssetTypeID typeID, const std::string& path) const;
		/** Import a new resource from file dialog or dragging by copying it from srcPath to destPath. */
		bool ImportAsset(AssetTypeID typeID, const std::string& srcPath, const std::string& destPath) const;
		/** Construct a new asset. */
		Ref<IAsset> CreateAsset(const Ref<AssetMetadata>& metadata) const;
		/** Open an existing asset. */
		bool OpenAsset(const std::string& path, bool bIsFromAssetBrowser) const;
		/** Rename an existing asset. */
		bool RenameAsset(const std::string& oldPath, const std::string& newPath) const;
		/** Delete an existing asset. */
		bool DeleteAsset(const std::string& path) const;
		/** Save an existing asset. */
		bool SaveAsset(const std::string& path, const Ref<IAsset>& asset) const;
		/** Reimport an existing asset from its source place. */
		bool ReimportAsset(const std::string& path) const;

		AssetFactoryBase* GetAssetFactoryByAssetType(AssetTypeID typeID) const;
		AssetActionsBase* GetAssetActionsByAssetType(AssetTypeID typeID) const;
		AssetSerializerBase* GetAssetSerializerByAssetType(AssetTypeID typeID) const;

		template <typename Func>
		void ForEachAssetFactory(Func func) const
		{
			static_assert(std::is_invocable_v<Func, AssetFactoryBase*> || std::is_invocable_v<Func, AssetTypeID, AssetFactoryBase*>, "Failed to find the matching func for call: ForEachAssetFactory!");

			auto begin = m_AssetFactories.begin();
			auto end = m_AssetFactories.end();

			while (begin != end)
			{
				auto curr = begin++;

				if constexpr (std::is_invocable_v<Func, AssetFactoryBase*>)
				{
					func(curr->second.get());
				}
				else if constexpr (std::is_invocable_v<Func, AssetTypeID, AssetFactoryBase*>)
				{
					func(curr->first, curr->second.get());
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
		AssetManager();

	private:
		/** Map from asset type ID to asset factory */
		std::unordered_map<AssetTypeID, Scope<AssetFactoryBase>> m_AssetFactories;
		/** Map from asset type ID to asset actions */
		std::unordered_map<AssetTypeID, Scope<AssetActionsBase>> m_AssetActions;
		/** Map from asset type ID to asset serializers */
		std::unordered_map<AssetTypeID, Scope<AssetSerializerBase>> m_AssetSerializers;

		/** Map from supported file extension to asset type ID */
		std::unordered_map<std::string, AssetTypeID, CaseInsensitiveUnorderedMap::Hash, CaseInsensitiveUnorderedMap::Comp> m_SupportedFileExtensions;
	};

}
