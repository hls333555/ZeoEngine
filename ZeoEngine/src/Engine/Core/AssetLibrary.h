#pragma once

#include <entt.hpp>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	template<typename AssetLibraryClass, typename AssetClass, typename AssetLoaderClass>
	class AssetLibrary : private entt::resource_cache<AssetClass, AssetLoaderClass>
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

			auto ret = load(path.ToId(), path.GetPath(), std::forward<Args>(args)...);
			return ret.first->second;
		}

		AssetHandle<AssetClass> ReloadAsset(AssetPath path)
		{
			ZE_CORE_ASSERT(!path.IsEmpty());

			auto ret = force_load(path.ToId(), path.GetPath());
			return ret.first->second;
		}

		void DiscardAsset(AssetPath path)
		{
			erase(path.ToId());
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
