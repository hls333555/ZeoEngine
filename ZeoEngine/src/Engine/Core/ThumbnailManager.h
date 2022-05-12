#pragma once

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

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
		Ref<Texture2D> GetAssetThumbnail(const std::string& path, AssetTypeId typeId);
		std::string GetAssetThumbnailPath(const std::string& assetPath, AssetTypeId typeId) const;

		Ref<Texture2D> GetAssetTypeIcon(AssetTypeId typeId) const;
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
		std::unordered_map<AssetTypeId, Ref<Texture2D>> m_AssetTypeIcons;
		Ref<Texture2D> m_DirectoryIcon;
	};

}
