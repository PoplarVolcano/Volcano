using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    // A collection of common math functions.
    public class MathF
    {
        public static float Radians(float degrees) { return degrees * ((float)Math.PI / 180.0f); }

        public static float Degrees(float radians) { return radians * (180.0f / (float)Math.PI); }

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

        // 将向量v按四元数q旋转
        public static Vector3 Rotate(Quaternion q, Vector3 v) { Math_Rotate(q, v, out Vector3 result); return result; }

        public static Quaternion QuaternionFromEuler(Vector3 euler) { Math_QuaternionFromEuler(euler, out Quaternion result); return result; }

        public static Quaternion QuaternionFromToRotation(Vector3 fromDirection, Vector3 toDirection) { Math_QuaternionFromToRotation(fromDirection, toDirection, out Quaternion result); return result; }

        public static Quaternion QuaternionLookRotation(Vector3 forward, Vector3 upward) { Math_QuaternionLookRotation(forward, upward, out Quaternion result); return result; }

        public static Quaternion QuaternionLookRotation(Vector3 forward) { return QuaternionLookRotation(forward, Vector3.up); }

        public static Quaternion QuaternionInverse(Quaternion quaternion) { Math_QuaternionInverse(quaternion, out Quaternion result); return result; }

        public static Vector3 EulerFromQuaternion(Quaternion quaternion) { Math_EulerFromQuaternion(quaternion, out Vector3 result); return result; }

        public static Matrix4x4 Inverse(Matrix4x4 m4) { Math_Inverse(m4, out Matrix4x4 result); return result; }

        public static Matrix4x4 Transpose(Matrix4x4 m4) { Math_Transpose(m4, out Matrix4x4 result); return result; }

        public static Matrix4x4 TRS(Vector3 translation, Quaternion rotation, Vector3 scale) { Math_TRS(translation, rotation, scale, out Matrix4x4 result); return result; }

        public static Matrix4x4 Perspective(float fov, float aspect, float zNear, float zFar) { Math_Perspective(fov, aspect, zNear, zFar, out Matrix4x4 result); return result; }

        public static Vector3 Cross(Vector3 v1, Vector3 v2) { Math_Cross(v1, v2, out Vector3 result); return result; }

        public static Matrix4x4 LookAt(Vector3 eye, Vector3 center, Vector3 up) { Math_LookAt(eye, center, up, out Matrix4x4 result); return result; }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_Rotate(Quaternion q, Vector3 v, out Vector3 rotate);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_QuaternionFromEuler(Vector3 euler, out Quaternion quaternion);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_QuaternionFromToRotation(Vector3 fromDirection, Vector3 toDirection, out Quaternion quaternion);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_QuaternionLookRotation(Vector3 forward, Vector3 upwards, out Quaternion quaternion);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_QuaternionInverse(Quaternion quaternion, out Quaternion inversedQuaternion);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_EulerFromQuaternion(Quaternion quaternion, out Vector3 Euler);
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
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Math_LookAt(Vector3 eye, Vector3 center, Vector3 up, out Matrix4x4 lookAt);

        // The infamous 3.14159265358979... value.
        public const float PI = (float)Math.PI;

        // A representation of positive infinity. 正无穷的表示。
        public const float Infinity = Single.PositiveInfinity;

        // A representation of negative infinity. 负无穷的表示。
        public const float NegativeInfinity = Single.NegativeInfinity;

        // Degrees-to-radians conversion constant. 度数到弧度的转换常数。
        public const float Deg2Rad = PI * 2F / 360F;

        // Radians-to-degrees conversion constant. 弧度到度数的转换常数。
        public const float Rad2Deg = 1F / Deg2Rad;

    }
}
