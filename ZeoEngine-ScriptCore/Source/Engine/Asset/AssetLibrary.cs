using System;

namespace ZeoEngine
{
    public class AssetLibrary
    {
        public static T LoadAsset<T>(string path) where T : Asset, new()
        {
            var asset = InternalCalls.AssetLibrary_LoadAsset(path);
            InternalCalls.Asset_GetHandle(asset, out AssetHandle handle);
            return new T() { Handle = handle, m_UnmanagedInstance = asset };
        }

        public static T LoadAsset<T>(AssetHandle handle) where T : Asset, new()
        {
            var asset = InternalCalls.AssetLibrary_LoadAsset(handle.ID);
            return new T() { Handle = handle, m_UnmanagedInstance = asset };
        }
    }
}
