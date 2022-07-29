using System;
using System.Runtime.CompilerServices;

namespace ZeoEngine
{
    public struct Vector3
    {
        public float X, Y, Z;

        public static Vector3 Zero = new Vector3(0, 0, 0);

        public Vector3(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public static Vector3 operator+(Vector3 lhs, Vector3 rhs)
        {
            return new Vector3(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z);
        }

        public static Vector3 operator*(Vector3 vector, float scalar)
        {
            return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

    }

    public static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void NativeLog(string text, int param);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void NativeLog_Vector(ref Vector3 param, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float NativeLog_VectorDot(ref Vector3 param);

        #region Entity

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_GetTranslation(ulong entityID, out Vector3 translation);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_SetTranslation(ulong entityID, ref Vector3 translation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_GetRotation(ulong entityID, out Vector3 rotation);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_SetRotation(ulong entityID, ref Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_GetScale(ulong entityID, out Vector3 scale);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_SetScale(ulong entityID, ref Vector3 scale);

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
        
    }

    public class Entity
    {
        protected Entity() { ID = 0; }
        internal Entity(ulong id)
        {
            ID = id;
        }

        public readonly ulong ID;

        public Vector3 Translation
        {
            get
            {
                InternalCalls.Entity_GetTranslation(ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.Entity_SetTranslation(ID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.Entity_GetRotation(ID, out Vector3 rotation);
                return rotation;
            }
            set
            {
                InternalCalls.Entity_SetRotation(ID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                InternalCalls.Entity_GetScale(ID, out Vector3 scale);
                return scale;
            }
            set
            {
                InternalCalls.Entity_SetScale(ID, ref value);
            }
        }
    }
}
