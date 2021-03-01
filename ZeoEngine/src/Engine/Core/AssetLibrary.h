#pragma once

namespace ZeoEngine {

	/**
	 * Base template class for asset management.
	 * You should add a static create function in your derived class like below:
	 * @code
	 * public:
	 * static MyAssetLibrary& Get()
	 * {
	 *     static MyAssetLibrary instance;
	 *     return instance;
	 * }
	 * @endcode
	 */
	template<typename T>
	class AssetLibrary
	{
	protected:
		AssetLibrary() = default;
	public:
		AssetLibrary(const AssetLibrary&) = delete;
		AssetLibrary& operator=(const AssetLibrary&) = delete;

		const auto& GetAssetsMap() const { return m_Assets; }

		virtual T LoadAsset(const std::string& path) = 0;
		T GetOrLoadAsset(const std::string& path)
		{
			if (DoesAssetExist(path))
			{
				return m_Assets[GetRelativePath(path)];
			}
			else
			{
				return LoadAsset(path);
			}
		}

		T GetAsset(const std::string& path)
		{
			ZE_CORE_ASSERT(DoesAssetExist(path), "{0} not found!", GetDisplayAssetName());
			return m_Assets[GetRelativePath(path)];
		}

		bool DoesAssetExist(const std::string& path) const
		{
			return m_Assets.find(GetRelativePath(path)) != m_Assets.end();
		}

	protected:
		void AddAsset(const T& asset)
		{
			const std::string& path = asset->GetPath();
			AddAsset(path, asset);
		}

	private:
		void AddAsset(const std::string& path, const T& asset)
		{
			m_Assets[GetRelativePath(path)] = asset;
		}

		virtual const char* GetDisplayAssetName() const = 0;

	private:
		std::unordered_map<std::string, T> m_Assets;

	};

}
