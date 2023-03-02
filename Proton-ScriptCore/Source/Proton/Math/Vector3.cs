using System;

namespace Proton
{
    public struct Vector3
    {
        public float x, y, z;

        public static Vector3 Zero = new Vector3(0f, 0f, 0f);

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        //Vector-Vector operations
        public static Vector3 operator +(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
        }

        public static Vector3 operator -(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
        }

        public static Vector3 operator *(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
        }

        public static Vector3 operator /(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
        }

        //Vector-Scalar operations
        public static Vector3 operator +(Vector3 vector, float scalar)
        {
            return new Vector3(vector.x + scalar, vector.y + scalar, vector.z + scalar);
        }

        public static Vector3 operator -(Vector3 vector, float scalar)
        {
            return new Vector3(vector.x - scalar, vector.y - scalar, vector.z - scalar);
        }

        public static Vector3 operator *(Vector3 vector, float scalar)
        {
            return new Vector3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
        }

        public static Vector3 operator /(Vector3 vector, float scalar)
        {
            return new Vector3(vector.x / scalar, vector.y / scalar, vector.z / scalar);
        }

        //Static functions
        public static Vector3 SqrMagnitude(Vector3 vec)
        {
            vec.SqrMagnitude();
            return vec;
        }
        public static Vector3 Magnitude(Vector3 vec)
        {
            vec.Magnitude();
            return vec;
        }

        public static Vector3 Normalize(Vector3 vec)
        {
            vec.Normalize();
            return vec;
        }

        public static float Dot(Vector3 a, Vector3 b)
        {
            return a.Dot(b);
        }

        // Member functions
        public float SqrMagnitude()
        {
            return x * x + y * y + z * z;
        }

        public float Magnitude()
        {
            return (float)Math.Sqrt(SqrMagnitude());
        }

        public void Normalize()
        {
            float invMag = 1.0f / Magnitude();
            x /= invMag;
            y /= invMag;
            z /= invMag;
        }

        public float Dot(Vector3 other)
        {
            return x * other.x + y * other.y + z * other.z;
        }
    }
}
