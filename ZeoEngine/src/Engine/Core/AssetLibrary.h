#pragma once

#include <entt.hpp>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	template<typename AssetLibraryClass, typename AssetClass, typename AssetLoaderClass>
	class AssetLibrary : private entt::resource_cache<AssetClass>
	{
	public:
		static AssetLibraryClass& Get()
		{
			static AssetLibraryClass instance;
			return instance;
		}

		template<typename... Args>
		AssetHandle<AssetClass> LoadAsset(AssetPath path, Args &&... args)
		{
			ZE_CORE_ASSERT(!path.IsEmpty());

			return load<AssetLoaderClass>(path.ToId(), path.GetPath(), std::forward<Args>(args)...);
		}

		template<typename... Args>
		AssetHandle<AssetClass> ReloadAsset(AssetPath path, Args &&... args)
		{
			ZE_CORE_ASSERT(!path.IsEmpty());

			return reload<AssetLoaderClass>(path.ToId(), path.GetPath(), std::forward<Args>(args)...);
		}

		AssetHandle<AssetClass> ReloadAsset(AssetPath path)
		{
			if (path.IsEmpty()) return {};
			if (!HasAsset(path)) return {};

			auto asset = GetAsset(path);
			asset->Reload();
			return asset;
		}

		void DiscardAsset(AssetPath path)
		{
			discard(path.ToId());
		}

		AssetHandle<AssetClass> GetAsset(AssetPath path)
		{
			const auto id = path.ToId();
			ZE_CORE_ASSERT(contains(id));

			return handle(id);
		}

		bool HasAsset(AssetPath path)
		{
			return contains(path.ToId());
		}

		template<typename Func>
		void ForEach(Func func) const
		{
			each(func);
		}

	protected:
		AssetLibrary() = default;
		AssetLibrary(const AssetLibrary&) = delete;
		AssetLibrary& operator=(const AssetLibrary&) = delete;
	};

}
