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

		Asset<AssetClass> LoadAsset(AssetPath path)
		{
			return load<AssetLoaderClass>(path.ToId(), path.ToString());
		}

		virtual Asset<AssetClass> ReloadAsset(AssetPath path)
		{
			return reload<AssetLoaderClass>(path.ToId(), path.ToString());
		}

		Asset<AssetClass> GetAsset(AssetPath path)
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
