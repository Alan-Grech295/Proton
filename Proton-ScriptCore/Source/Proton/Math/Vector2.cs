using System;

namespace Proton
{
    public struct Vector2
    {
        public float x, y;

        public static Vector2 Zero = new Vector2(0f, 0f);

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        //Vector-Vector operations
        public static Vector2 operator +(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.x + v2.x, v1.y + v2.y);
        }

        public static Vector2 operator -(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.x - v2.x, v1.y - v2.y);
        }

        public static Vector2 operator *(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.x * v2.x, v1.y * v2.y);
        }

        public static Vector2 operator /(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.x / v2.x, v1.y / v2.y);
        }

        //Vector-Scalar operations
        public static Vector2 operator +(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x + scalar, vector.y + scalar);
        }

        public static Vector2 operator -(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x - scalar, vector.y - scalar);
        }

        public static Vector2 operator *(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x * scalar, vector.y * scalar);
        }

        public static Vector2 operator /(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x / scalar, vector.y / scalar);
        }

        //Static functions
        public static Vector2 SqrMagnitude(Vector2 vec)
        {
            vec.SqrMagnitude();
            return vec;
        }
        public static Vector2 Magnitude(Vector2 vec)
        {
            vec.Magnitude();
            return vec;
        }

        public static Vector2 Normalize(Vector2 vec)
        {
            vec.Normalize();
            return vec;
        }

        public static float Dot(Vector2 a, Vector2 b)
        {
            return a.Dot(b);
        }

        // Member functions
        public float SqrMagnitude()
        {
            return x * x + y * y;
        }

        public float Magnitude()
        {
            return (float)Math.Sqrt(SqrMagnitude());
        }

        public void Normalize()
        {
            float invMag = 1.0f / Magnitude();
            x *= invMag;
            y *= invMag;
        }

        public float Dot(Vector2 other)
        {
            return x * other.x + y * other.y;
        }
    }
}
