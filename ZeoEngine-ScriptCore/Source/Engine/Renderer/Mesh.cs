using System;

namespace ZeoEngine
{
    public class Mesh : Asset
    {

    }

    public class MeshInstance : Managed
    {
        internal MeshInstance(IntPtr unmanagedInstance)
        {
            m_UnmanagedInstance = unmanagedInstance;
        }

        public AssetHandle GetMaterial(uint index)
        {
            return InternalCalls.MeshInstance_GetMaterial(m_UnmanagedInstance, index);
        }

        public void SetMaterial(uint index, AssetHandle materialAsset)
        {
            InternalCalls.MeshInstance_SetMaterial(m_UnmanagedInstance, index, materialAsset);
        }

    }
}
