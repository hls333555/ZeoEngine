#pragma once

#include <unordered_map>

#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class IAssetActions;
	class AssetFactoryBase;

	class AssetManager
	{
		friend class EditorLayer;

	public:
		static AssetManager& Get()
		{
			static AssetManager instance;
			return instance;
		}

		/** Register asset actions. Returns true if the registration succeeded, false otherwise. */
		bool RegisterAssetActions(AssetTypeId typeId, Ref<IAssetActions> actions);
		/** Register asset factory. Returns true if the registration succeeded, false otherwise. */
		bool RegisterAssetFactory(AssetTypeId typeId, Ref<AssetFactoryBase> factory);

		/** Construct a new asset. */
		void CreateAsset(AssetTypeId typeId, const std::string& path) const;
		/** Open an existing asset. */
		bool OpenAsset(const std::string& path);

		Ref<IAssetActions> GetAssetActionsByAssetType(AssetTypeId typeId);
		Ref<AssetFactoryBase> GetAssetFactoryByAssetType(AssetTypeId typeId);

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
				else if constexpr (std::is_invocable_v<Func, Ref<AssetFactoryBase>>)
				{
					func(curr->second);
				}
				else
				{
					func(curr->first, curr->second);
				}
			}
		}

	private:
		void Init();

	protected:
		AssetManager() = default;
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

	private:
		/** Map from asset type hash to asset actions */
		std::unordered_map<AssetTypeId, Ref<IAssetActions>> m_AssetActions;
		/** Map from asset type hash to asset factory */
		std::unordered_map<AssetTypeId, Ref<AssetFactoryBase>> m_AssetFactories;
	};

}
