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

        public static float FInterpTo(float current, float target, float dt, float interpSpeed)
        {
            return InternalCalls.Math_FInterpTo(current, target, dt, interpSpeed);
        }

        public static Vector3 VInterpTo(Vector3 current, Vector3 target, float dt, float interpSpeed)
        {
            InternalCalls.Math_VInterpTo(ref current, ref target, dt, interpSpeed, out Vector3 result);
            return result;
        }

        public static Vector3 VInterpConstantTo(Vector3 current, Vector3 target, float dt, float interpSpeed)
        {
            InternalCalls.Math_VInterpConstantTo(ref current, ref target, dt, interpSpeed, out Vector3 result);
            return result;
        }

        public static Vector3 FindLookAtRotation(Vector3 from, Vector3 to, Vector3 up)
        {
            InternalCalls.Math_FindLookAtRotation(ref from, ref to, ref up, out Vector3 result);
            return result;
        }
    }
}
