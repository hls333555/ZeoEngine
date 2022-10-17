using System;
using System.Runtime.CompilerServices;

namespace ZeoEngine
{
    public static class InternalCalls
    {
        #region Entity

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string Entity_GetName(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Entity_HasComponent(ulong entityID, Type compType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Entity_GetEntityByName(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern object GetScriptInstance(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_GetForwardVector(ulong entityID, out Vector3 forwardVector);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_GetRightVector(ulong entityID, out Vector3 rightVector);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_GetUpVector(ulong entityID, out Vector3 upVector);

        #endregion

        #region TransformComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetRotation(ulong entityID, out Vector3 rotation);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetScale(ulong entityID, out Vector3 scale);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

        #endregion

        #region MeshRendererComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern AssetHandle MeshRendererComponent_GetMeshAsset(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_SetMeshAsset(ulong entityID, AssetHandle meshAsset);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr MeshRendererComponent_GetInstance(ulong entityID);

        #endregion

        #region Input

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyPressed(KeyCode keycode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyReleased(KeyCode keycode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsMouseButtonPressed(MouseCode mousecode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsMouseButtonReleased(MouseCode mousecode);

        #endregion

        #region Asset

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr AssetLibrary_LoadAsset(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr AssetLibrary_LoadAsset(AssetHandle handle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string Asset_GetName(AssetHandle handle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Asset_GetHandle(IntPtr asset, out AssetHandle handle);

        #endregion

        #region Mesh

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern AssetHandle MeshInstance_GetMaterial(IntPtr meshInstance, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshInstance_SetMaterial(IntPtr meshInstance, uint index, AssetHandle materialAsset);

        #endregion
        
    }
}
