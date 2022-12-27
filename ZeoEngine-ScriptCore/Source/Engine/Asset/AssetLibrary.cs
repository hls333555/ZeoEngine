using System;

namespace ZeoEngine
{
    public class AssetLibrary
    {
        public static T LoadAsset<T>(string path, bool bForceLoad = false) where T : Asset, new()
        {
            var asset = InternalCalls.AssetLibrary_LoadAssetByPath(path, bForceLoad);
            InternalCalls.Asset_GetHandle(asset, out AssetHandle handle);
            return new T() { Handle = handle, m_UnmanagedInstance = asset };
        }

        public static T LoadAsset<T>(AssetHandle handle, bool bForceLoad = false) where T : Asset, new()
        {
            var asset = InternalCalls.AssetLibrary_LoadAssetByHandle(handle.ID, bForceLoad);
            return new T() { Handle = handle, m_UnmanagedInstance = asset };
        }
    }
}
