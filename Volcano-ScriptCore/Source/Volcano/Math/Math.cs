using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    // A collection of common math functions.
    public class MathF
    {
        public static float Radians(float degrees)
        {
            return degrees * ((float)Math.PI / 180.0f);
        }

        public static float Degrees(float radians)
        {
            return radians * (180.0f / (float)Math.PI);
        }

        public static float Clamp(float value, float min, float max)
        {
            if (value < min)
            {
                value = min;
            }
            else if (value > max)
            {
                value = max;
            }

            return value;
        }

        public static Vector3 Rotate(Quaternion q, Vector3 v)
        {
            Math_Rotate(q, v, out Vector3 result);
            return result;
        }

        public static Quaternion Quaternion(Vector3 Euler)
        {
            Math_Quaternion(Euler, out Quaternion result);
            return result;
        }

        public static Matrix4x4 Inverse(Matrix4x4 m4)
        {
            Math_Inverse(m4, out Matrix4x4 result);
            return result;
        }

        public static Matrix4x4 Transpose(Matrix4x4 m4)
        {
            Math_Transpose(m4, out Matrix4x4 result);
            return result;
        }
        public static Matrix4x4 TRS(Vector3 translation, Quaternion rotation, Vector3 scale)
        {
            Math_TRS(translation, rotation, scale, out Matrix4x4 result);
            return result;
        }

        public static Matrix4x4 Perspective(float fov, float aspect, float zNear, float zFar)
        {
            Math_Perspective(fov, aspect, zNear, zFar, out Matrix4x4 result);
            return result;
        }

        public static Vector3 Cross(Vector3 v1, Vector3 v2)
        {
            Math_Cross(v1, v2, out Vector3 result);
            return result;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_Rotate(Quaternion q, Vector3 v, out Vector3 rotate);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_Quaternion(Vector3 Euler, out Quaternion quaternion);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_Inverse(Matrix4x4 m4, out Matrix4x4 mat4);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_Transpose(Matrix4x4 m4, out Matrix4x4 mat4);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_TRS(Vector3 translation, Quaternion rotation, Vector3 scale, out Matrix4x4 TRS);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_Perspective(float fov, float aspect, float zNear, float zFar, out Matrix4x4 perspective);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_Cross(Vector3 v1, Vector3 v2, out Vector3 cross);

    }
}
