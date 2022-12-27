using System;

namespace ZeoEngine
{
    public class MathLib
    {
        public const float PI = (float)Math.PI;
        public const float TwoPI = (float)(Math.PI * 2.0);

        public static float Deg2Rnd(float angle) => PI / 180.0f * angle;
        public static float Rnd2Deg(float angle) => 180.0f / PI * angle;

        public static float Abs(float value)
        {
            return Math.Abs(value);
        }

        public static float Sin(float value)
        {
            return (float)Math.Sin(value);
        }

        public static float Cos(float value)
        {
            return (float)Math.Cos(value);
        }

        public static float InterpTo(float current, float target, float dt, float interpSpeed)
        {
            return InternalCalls.Math_FloatInterpTo(current, target, dt, interpSpeed);
        }

        public static Vector2 InterpTo(Vector2 current, Vector2 target, float dt, float interpSpeed)
        {
            InternalCalls.Math_Vector2InterpTo(ref current, ref target, dt, interpSpeed, out Vector2 result);
            return result;
        }

        public static Vector3 InterpTo(Vector3 current, Vector3 target, float dt, float interpSpeed)
        {
            InternalCalls.Math_Vector3InterpTo(ref current, ref target, dt, interpSpeed, out Vector3 result);
            return result;
        }

        public static float InterpConstantTo(float current, float target, float dt, float interpSpeed)
        {
            return InternalCalls.Math_FloatInterpConstantTo(current, target, dt, interpSpeed);
        }

        public static Vector2 InterpConstantTo(Vector2 current, Vector2 target, float dt, float interpSpeed)
        {
            InternalCalls.Math_Vector2InterpConstantTo(ref current, ref target, dt, interpSpeed, out Vector2 result);
            return result;
        }

        public static Vector3 InterpConstantTo(Vector3 current, Vector3 target, float dt, float interpSpeed)
        {
            InternalCalls.Math_Vector3InterpConstantTo(ref current, ref target, dt, interpSpeed, out Vector3 result);
            return result;
        }

    }
}
