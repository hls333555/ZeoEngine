#pragma once

#include "Engine/Asset/Asset.h"

namespace ZeoEngine {

	struct AssetMetadata;
	class Texture2D;

	class ThumbnailManager
	{
		friend class EditorLayer;

	public:
		static ThumbnailManager& Get()
		{
			static ThumbnailManager instance;
			return instance;
		}

		/** Try to get asset thumbnail from disk or return the default type icon. */
		Ref<Texture2D> GetAssetThumbnail(const Ref<AssetMetadata>& metadata);
		std::string GetAssetThumbnailPath(const Ref<AssetMetadata>& metadata) const;

		Ref<Texture2D> GetAssetTypeIcon(AssetTypeID typeId) const;
		const Ref<Texture2D>& GetDirectoryIcon() const { return m_DirectoryIcon; }

	private:
		void Init();
		void LoadAssetTypeIcons();

	protected:
		ThumbnailManager() = default;
		ThumbnailManager(const ThumbnailManager&) = delete;
		ThumbnailManager& operator=(const ThumbnailManager&) = delete;

	private:
		/** Map from asset type id to its type icon texture */
		std::unordered_map<AssetTypeID, Ref<Texture2D>> m_AssetTypeIcons;
		Ref<Texture2D> m_DirectoryIcon;
	};

}
