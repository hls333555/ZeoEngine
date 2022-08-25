using System;

namespace ZeoEngine
{
    public class Entity
    {
        protected Entity() { ID = 0; }
        internal Entity(ulong id)
        {
            ID = id;
        }

        public readonly ulong ID;

        public string Name => InternalCalls.Entity_GetName(ID);

        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 translation);
                return translation;
            }
            set => InternalCalls.TransformComponent_SetTranslation(ID, ref value);
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(ID, out Vector3 rotation);
                return rotation;
            }
            set => InternalCalls.TransformComponent_SetRotation(ID, ref value);
        }

        public Vector3 Scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(ID, out Vector3 scale);
                return scale;
            }
            set => InternalCalls.TransformComponent_SetScale(ID, ref value);
        }

        public Vector3 GetForwardVector()
        {
            InternalCalls.Entity_GetForwardVector(ID, out Vector3 forwardVector);
            return forwardVector;
        }

        public Vector3 GetRightVector()
        {
            InternalCalls.Entity_GetRightVector(ID, out Vector3 rightVector);
            return rightVector;
        }

        public Vector3 GetUpVector()
        {
            InternalCalls.Entity_GetUpVector(ID, out Vector3 upVector);
            return upVector;
        }

        public bool HasComponent<T>() where T : IComponent, new()
        {
            Type compType = typeof(T);
            return InternalCalls.Entity_HasComponent(ID, compType);
        }

        public T GetComponent<T>() where T : IComponent, new()
        {
            if (!HasComponent<T>()) return null;

            return new T() { Entity = this };
        }
    }
}
