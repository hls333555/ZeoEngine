using System;

namespace ZeoEngine
{
    public struct AssetHandle
    {
        internal AssetHandle(ulong id)
        {
            ID = id;
        }

        public static implicit operator ulong(AssetHandle handle) => handle.ID;
        public static implicit operator AssetHandle(ulong handle) => new AssetHandle(handle);

        public readonly ulong ID;
    }

    public class Asset : Managed
    {
        public AssetHandle Handle { get; internal set; }
        public string Name => InternalCalls.Asset_GetName(Handle);
    }
}
