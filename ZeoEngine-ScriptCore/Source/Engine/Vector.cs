namespace ZeoEngine
{
    public struct Vector2
    {
        public float X, Y;

        public static Vector2 Zero = new Vector2(0, 0);

        public Vector2(float scalar)
        {
            X = scalar;
            Y = scalar;
        }

        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }

        public static Vector2 operator+(Vector2 lhs, Vector2 rhs)
        {
            return new Vector2(lhs.X + rhs.X, lhs.Y + rhs.Y);
        }

        public static Vector2 operator-(Vector2 lhs, Vector2 rhs)
        {
            return new Vector2(lhs.X - rhs.X, lhs.Y - rhs.Y);
        }

        public static Vector2 operator*(Vector2 vector, float scalar)
        {
            return new Vector2(vector.X * scalar, vector.Y * scalar);
        }

        public override string ToString()
        {
            return "Vector2[" + X + ", " + Y + "]";
        }

    }

    public struct Vector3
    {
        public float X, Y, Z;

        public static Vector3 Zero = new Vector3(0, 0, 0);
        public static Vector3 Forward = new Vector3(0, 0, -1);
        public static Vector3 Right = new Vector3(1, 0, 0);
        public static Vector3 Up = new Vector3(0, 1, 0);

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

        public Vector3(Vector2 xy, float z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }

        public Vector2 XY
        {
            get => new Vector2(X, Y);
            set
            {
                X = value.X;
                Y = value.Y;
            }
        }

        public static Vector3 operator+(Vector3 lhs, Vector3 rhs)
        {
            return new Vector3(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z);
        }

        public static Vector3 operator-(Vector3 lhs, Vector3 rhs)
        {
            return new Vector3(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z);
        }

        public static Vector3 operator*(Vector3 vector, float scalar)
        {
            return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

        public override bool Equals(object obj) => obj is Vector3 other && this.Equals(other);

        public bool Equals(Vector3 vector, float tolerance = 1.0e-4f) => MathLib.Abs(X - vector.X) <= tolerance && MathLib.Abs(Y - vector.Y) <= tolerance && MathLib.Abs(Z - vector.Z) <= tolerance;

        public override int GetHashCode() => (X, Y, Z).GetHashCode();

        public static bool operator ==(Vector3 lhs, Vector3 rhs) => lhs.Equals(rhs);

        public static bool operator !=(Vector3 lhs, Vector3 rhs) => !(lhs == rhs);

        public override string ToString()
        {
            return "Vector3[" + X + ", " + Y + ", " + Z + "]";
        }

    }

    public struct Vector4
    {
        public float X, Y, Z, W;

        public static Vector4 Black = new Vector4(0, 0, 0, 1);
        public static Vector4 White = new Vector4(1, 1, 1, 1);
        public static Vector4 Grey = new Vector4(0.5f, 0.5f, 0.5f, 0.5f);
        public static Vector4 Red = new Vector4(1, 0, 0, 1);
        public static Vector4 Green = new Vector4(0, 1, 0, 1);
        public static Vector4 Blue = new Vector4(0, 0, 1, 1);
        public static Vector4 Yellow = new Vector4(1, 1, 0, 1);

        public Vector4(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
            W = scalar;
        }

        public Vector4(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public static Vector4 operator+(Vector4 lhs, Vector4 rhs)
        {
            return new Vector4(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z, lhs.W + rhs.W);
        }

        public static Vector4 operator-(Vector4 lhs, Vector4 rhs)
        {
            return new Vector4(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z, lhs.W - rhs.W);
        }

        public static Vector4 operator*(Vector4 vector, float scalar)
        {
            return new Vector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
        }

        public override string ToString()
        {
            return "Vector4[" + X + ", " + Y + ", " + Z + ", " + W + "]";
        }

    }

}