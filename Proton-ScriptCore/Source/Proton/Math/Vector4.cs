using System;

namespace Proton
{
    public struct Vector4
    {
        public float x, y, z, w;

        public static Vector4 Zero = new Vector4(0f, 0f, 0f, 0f);

        public Vector4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        //Vector-Vector operations
        public static Vector4 operator +(Vector4 v1, Vector4 v2)
        {
            return new Vector4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
        }

        public static Vector4 operator -(Vector4 v1, Vector4 v2)
        {
            return new Vector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
        }

        public static Vector4 operator *(Vector4 v1, Vector4 v2)
        {
            return new Vector4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
        }

        public static Vector4 operator /(Vector4 v1, Vector4 v2)
        {
            return new Vector4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
        }

        //Vector-Scalar operations
        public static Vector4 operator +(Vector4 vector, float scalar)
        {
            return new Vector4(vector.x + scalar, vector.y + scalar, vector.z + scalar, vector.w + scalar);
        }

        public static Vector4 operator -(Vector4 vector, float scalar)
        {
            return new Vector4(vector.x - scalar, vector.y - scalar, vector.z - scalar, vector.w - scalar);
        }

        public static Vector4 operator *(Vector4 vector, float scalar)
        {
            return new Vector4(vector.x * scalar, vector.y * scalar, vector.z * scalar, vector.w * scalar);
        }

        public static Vector4 operator /(Vector4 vector, float scalar)
        {
            return new Vector4(vector.x / scalar, vector.y / scalar, vector.z / scalar, vector.w / scalar);
        }

        //Static functions
        public static Vector4 SqrMagnitude(Vector4 vec)
        {
            vec.SqrMagnitude();
            return vec;
        }
        public static Vector4 Magnitude(Vector4 vec)
        {
            vec.Magnitude();
            return vec;
        }

        public static Vector4 Normalize(Vector4 vec)
        {
            vec.Normalize();
            return vec;
        }

        public static float Dot(Vector4 a, Vector4 b)
        {
            return a.Dot(b);
        }

        // Member functions
        public float SqrMagnitude()
        {
            return x * x + y * y + z * z + w * w;
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
            z *= invMag;
            w *= invMag;
        }

        public float Dot(Vector4 other)
        {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }
    }
}
