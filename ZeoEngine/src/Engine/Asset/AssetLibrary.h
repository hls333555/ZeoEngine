#pragma once

#include <entt.hpp>

#include "Engine/Asset/AssetSerializer.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Asset/AssetManager.h"

namespace ZeoEngine {

	class AssetLibrary
	{
	public:
		template<typename T>
		static Ref<T> LoadAsset(AssetHandle handle, void* payload = nullptr)
		{
			static_assert(std::is_same_v<IAsset, T> || std::is_base_of_v<AssetBase<T>, T>, "Asset class T is not derived from 'AssetBase'!");

			if (IsMemoryAsset(handle))
			{
				return std::dynamic_pointer_cast<T>(s_MemoryAssets[handle]);
			}

			Ref<IAsset> asset = nullptr;
			if (HasAsset(handle))
			{
				asset = s_LoadedAssets[handle];
			}
			else
			{
				const auto metadata = AssetRegistry::Get().GetAssetMetadata(handle);
				if (!metadata) return nullptr;

				asset = AssetManager::Get().CreateAsset(metadata);
				if (asset)
				{
					asset->SetHandle(handle);
					const bool res = AssetManager::Get().GetAssetSerializerByAssetType(metadata->TypeID)->Deserialize(metadata, asset, payload);
					if (res)
					{
						s_LoadedAssets[handle] = asset;
					}
				}
			}

			return std::dynamic_pointer_cast<T>(asset);
		}

		template<typename T>
		static Ref<T> LoadAsset(const std::filesystem::path& path, void* payload = nullptr)
		{
			return LoadAsset<T>(AssetRegistry::Get().GetAssetHandleFromPath(path), payload);
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

		static bool ReloadAsset(const std::filesystem::path& path)
		{
			return ReloadAsset(AssetRegistry::Get().GetAssetHandleFromPath(path));
		}

		static bool HasAsset(AssetHandle handle)
		{
			return s_LoadedAssets.find(handle) != s_LoadedAssets.end();
		}

	private:
		static std::unordered_map<AssetHandle, Ref<IAsset>> s_LoadedAssets;
		static std::unordered_map<AssetHandle, Ref<IAsset>> s_MemoryAssets;
	};

}
