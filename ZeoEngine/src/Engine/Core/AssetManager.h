#pragma once

#include <unordered_map>
#include <optional>

#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class IAssetFactory;
	class IAssetActions;
	class Texture2D;

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
		bool RegisterAssetFactory(AssetTypeId typeId, Ref<IAssetFactory> factory);
		/** Register asset actions. Returns true if the registration succeeded, false otherwise. */
		bool RegisterAssetActions(AssetTypeId typeId, Ref<IAssetActions> actions);

		/** Construct a new asset. */
		void CreateAsset(AssetTypeId typeId, const std::string& path) const;
		/** Import a new asset from file dialog by copying it from srcPath to destPath. */
		void ImportAsset(AssetTypeId typeId, const std::string& srcPath, const std::string& destPath) const;
		/** Open an existing asset. */
		bool OpenAsset(const std::string& path);

		Ref<IAssetFactory> GetAssetFactoryByAssetType(AssetTypeId typeId);
		Ref<IAssetActions> GetAssetActionsByAssetType(AssetTypeId typeId);

		template <typename Func>
		void ForEachAssetFactory(Func func) const
		{
			auto begin = m_AssetFactories.begin();
			auto end = m_AssetFactories.end();

			while (begin != end)
			{
				auto curr = begin++;

				if constexpr (std::is_invocable_v<Func, AssetTypeId>)
				{
					func(curr->first);
				}
				else if constexpr (std::is_invocable_v<Func, Ref<IAssetFactory>>)
				{
					func(curr->second);
				}
				else
				{
					func(curr->first, curr->second);
				}
			}
		}

		Ref<Texture2D> GetAssetTypeIcon(AssetTypeId typeId) const;
		Ref<Texture2D> GetFolderIcon() const { return m_FolderIcon; }

		std::optional<AssetTypeId> GetTypdIdFromFileExtension(const std::string& extension);

	private:
		void Init();
		void LoadAssetTypeIcons();
		void InitSupportedFileExtensions();

	protected:
		AssetManager() = default;
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

	private:
		/** Map from asset type id to asset factory */
		std::unordered_map<AssetTypeId, Ref<IAssetFactory>> m_AssetFactories;
		/** Map from asset type id to asset actions */
		std::unordered_map<AssetTypeId, Ref<IAssetActions>> m_AssetActions;

		/** Map from asset type id to its type icon texture */
		std::unordered_map<AssetTypeId, Ref<Texture2D>> m_AssetTypeIcons;
		Ref<Texture2D> m_FolderIcon;

		/** Map from supported file extension to asset type Id */
		std::unordered_map<std::string, AssetTypeId> m_SupportedFileExtensions;
	};

}
