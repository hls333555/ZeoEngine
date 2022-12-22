using System;
using System.Runtime.CompilerServices;

namespace ZeoEngine
{
    // NOTE: In most cases, if the function has a custom return type such as AssetHandle,
    // you should use an "out" parameter to return it instead!
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

        #region WorldTransformComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void WorldTransformComponent_GetTranslation(ulong entityID, out Vector3 translation);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void WorldTransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void WorldTransformComponent_GetRotation(ulong entityID, out Vector3 rotation);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void WorldTransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void WorldTransformComponent_GetScale(ulong entityID, out Vector3 scale);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void WorldTransformComponent_SetScale(ulong entityID, ref Vector3 scale);

        #endregion

        #region MeshRendererComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_GetMeshAsset(ulong entityID, out AssetHandle meshAsset);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_SetMeshAsset(ulong entityID, AssetHandle meshAsset);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr MeshRendererComponent_GetInstance(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_GetMaterialAsset(ulong entityID, uint index, out AssetHandle materialAsset);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_SetMaterialAsset(ulong entityID, uint index, AssetHandle materialAsset);

        #endregion

        #region RigidBodyComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool CharacterControllerComponent_IsGrounded(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void CharacterControllerComponent_Move(ulong entityID, ref Vector3 displacement);

        #endregion

        #region CharacterControllerComponent

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidBodyComponent_SetKinematicTarget(ulong entityID, ref Vector3 targetPosition, ref Vector3 targetRotation);

        #endregion

        #region Math

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float Math_FInterpTo(float current, float target, float dt, float interpSpeed);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Math_VInterpTo(ref Vector3 current, ref Vector3 target, float dt, float interpSpeed, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Math_VInterpConstantTo(ref Vector3 current, ref Vector3 target, float dt, float interpSpeed, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Math_FindLookAtRotation(ref Vector3 from, ref Vector3 to, ref Vector3 up, out Vector3 result);

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
        internal static extern IntPtr AssetLibrary_LoadAsset(string path, bool bForceLoad);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr AssetLibrary_LoadAsset(AssetHandle handle, bool bForceLoad);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string Asset_GetName(AssetHandle handle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Asset_GetHandle(IntPtr asset, out AssetHandle handle);

        #endregion

        #region Mesh



        #endregion

        #region Physics

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Physics_GetGravity(out Vector3 gravity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Physics_SetGravity(ref Vector3 gravity);

        #endregion

        #region Misc

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Log_LogMessage(Log.Level level, string formattedMessage);

        #endregion
        
    }
}
