#pragma once

#include <entt.hpp>

#include "Engine/Asset/AssetSerializer.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Asset/AssetManager.h"

namespace ZeoEngine {

	class AssetLibrary
	{
	public:
		/** If bForceLoad is true, asset will be deserialized every time LoadAsset is called, otherwise, asset will be deserialized only the first time LoadAsset is called */
		template<typename T>
		static Ref<T> LoadAsset(AssetHandle handle, bool bForceLoad = false, void* payload = nullptr)
		{
			static_assert(std::is_same_v<IAsset, T> || std::is_base_of_v<AssetBase<T>, T>, "Asset class T must be derived from 'AssetBase'!");

			if (IsMemoryAsset(handle))
			{
				return std::dynamic_pointer_cast<T>(s_MemoryAssets[handle]);
			}

			const auto metadata = AssetRegistry::Get().GetAssetMetadata(handle);
			if (!metadata) return nullptr;

			Ref<IAsset> asset = nullptr;
			if (HasAsset(handle))
			{
				asset = s_LoadedAssets[handle];
				if (bForceLoad)
				{
					AssetManager::Get().GetAssetSerializerByAssetType(metadata->TypeID)->Deserialize(metadata, asset, payload);
				}
			}
			else
			{
				asset = AssetManager::Get().CreateAsset(metadata);
				if (asset)
				{
					asset->SetHandle(handle);
					bool res = AssetManager::Get().GetAssetSerializerByAssetType(metadata->TypeID)->Deserialize(metadata, asset, payload);
					if (res)
					{
						s_LoadedAssets[handle] = asset;
					}
				}
			}

			return std::dynamic_pointer_cast<T>(asset);
		}

		template<typename T>
		static Ref<T> LoadAsset(const std::string& path, bool bForceLoad = false, void* payload = nullptr)
		{
			return LoadAsset<T>(AssetRegistry::Get().GetAssetHandleFromPath(path), bForceLoad, payload);
		}

		template<typename T, typename... Args>
		static AssetHandle CreateMemoryOnlyAsset(Args&&... args)
		{
			static_assert(std::is_base_of_v<AssetBase<T>, T>, "Asset class T is not derived from 'AssetBase'!");

			Ref<T> asset = T::Create(std::forward<Args>(args)...);
			const AssetHandle handle = AssetHandle();
			asset->SetHandle(handle);

			s_MemoryAssets[handle] = asset;
			return handle;
		}

		static bool IsMemoryAsset(AssetHandle handle)
		{
			return s_MemoryAssets.find(handle) != s_MemoryAssets.end();
		}

		static bool ReloadAsset(AssetHandle handle)
		{
			if (!HasAsset(handle)) return false;

			const auto metadata = AssetRegistry::Get().GetAssetMetadata(handle);
			if (!metadata) return false;

			const auto& asset = s_LoadedAssets[handle];
			AssetManager::Get().GetAssetSerializerByAssetType(metadata->TypeID)->ReloadData(metadata, asset);
			asset->GetAssetReloadedDelegate().publish();
			return true;
		}

		static bool ReloadAsset(const std::string& path)
		{
			return ReloadAsset(AssetRegistry::Get().GetAssetHandleFromPath(path));
		}

		static bool UnloadAsset(AssetHandle handle)
		{
			if (!HasAsset(handle)) return false;

			s_LoadedAssets.erase(handle);
			return true;
		}

		static bool HasAsset(AssetHandle handle)
		{
			return s_LoadedAssets.find(handle) != s_LoadedAssets.end();
		}

		static bool HasAsset(const std::string& path)
		{
			return HasAsset(AssetRegistry::Get().GetAssetHandleFromPath(path));
		}

	private:
		static std::unordered_map<AssetHandle, Ref<IAsset>> s_LoadedAssets;
		static std::unordered_map<AssetHandle, Ref<IAsset>> s_MemoryAssets;
	};

}
