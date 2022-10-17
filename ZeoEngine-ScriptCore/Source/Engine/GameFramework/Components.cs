namespace ZeoEngine
{
    public abstract class IComponent
    {
        public Entity Entity { get; internal set; }
    }

    public class TransformComponent : IComponent
    {
        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
                return translation;
            }
            set => InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 rotation);
                return rotation;
            }
            set => InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
        }

        public Vector3 Scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
                return scale;
            }
            set => InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
        }
    }

    public class MeshRendererComponent : IComponent
    {
        public AssetHandle MeshAsset
        {
            get => InternalCalls.MeshRendererComponent_GetMeshAsset(Entity.ID);
            set => InternalCalls.MeshRendererComponent_SetMeshAsset(Entity.ID, value);
        }

        public MeshInstance Instance => new MeshInstance(InternalCalls.MeshRendererComponent_GetInstance(Entity.ID));
    }

}
