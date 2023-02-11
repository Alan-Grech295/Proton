using System;
using System.Runtime.CompilerServices;

namespace Proton
{
    public struct Vector3
    {
        public float x, y, z;

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }
    }

    public static class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog(string message, int parameter);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vec(ref Vector3 param, out Vector3 result);
    }

    public class Entity
    {
        public float FloatVar { get; set; }



        public Entity()
        {
            Console.WriteLine("Main constructor!");
            InternalCalls.NativeLog("Hello world", 16);

            Vector3 pos = new Vector3(5, 2.5f, 1);
            Vector3 result;
            InternalCalls.NativeLog_Vec(ref pos, out result);
            Console.WriteLine(result.x + ", " + result.y + ", " + result.z);
        }

        public void PrintMessage()
        {
            Console.WriteLine("Hello world from C#!");
        }

        public void PrintInt(int value)
        {
            Console.WriteLine("C# says: " + value);
        }

        public void PrintCustomMessage(string message)
        {
            Console.WriteLine("C# says: " + message);
        }
    }
}
