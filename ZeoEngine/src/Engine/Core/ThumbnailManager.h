#pragma once

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

		/** Try to get asset thumbnail from cache. Returns null if not found. */
		Ref<Texture2D> GetAssetThumbnail(const std::string& path);

		static std::string GetAssetThumbnailPath(const std::string& assetPath);

	private:
		void Init();

	protected:
		ThumbnailManager() = default;
		ThumbnailManager(const ThumbnailManager&) = delete;
		ThumbnailManager& operator=(const ThumbnailManager&) = delete;

	};

}
