namespace ZeoEngine
{
    public class Physics
    {
        public static Vector3 Gravity
        {
            get
            {
                InternalCalls.Physics_GetGravity(out Vector3 gravity);
                return gravity;
            }
            set => InternalCalls.Physics_SetGravity(ref value);
        }
    }
}
