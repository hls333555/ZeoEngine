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

    }
}
