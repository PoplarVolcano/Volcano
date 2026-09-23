using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public class MathFloat
    {
        // The infamous 3.14159265358979... value.
        public const float PI = (float)Math.PI;

        // A representation of positive infinity. 正无穷的表示。
        public const float PositiveInfinity = Single.PositiveInfinity;

        // A representation of negative infinity. 负无穷的表示。
        public const float NegativeInfinity = Single.NegativeInfinity;

        // Degrees-to-radians conversion constant. 度数到弧度的转换常数。
        public const float DegreeToRadian = PI * 2F / 360F;

        // Radians-to-degrees conversion constant. 弧度到度数的转换常数。
        public const float RadianToDegree = 1F / DegreeToRadian;

        public static float Radians(float degrees) { return degrees * ((float)Math.PI / 180.0f); }

        public static float Degrees(float radians) { return radians * (180.0f / (float)Math.PI); }


        /// <summary>区间约束函数</summary>
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

        /// <summary>取 x 的绝对值，套上 y 的符号</summary>
        public static float CopySign(float x, float y)
        {
            return (y >= 0f) ? Math.Abs(x) : -Math.Abs(x);
        }
        /// <summary>将向量v按四元数q旋转</summary>
        public static Vector3 Rotate(Quaternion q, Vector3 v) {
            MathFloat_Rotate(q, v, out Vector3 result);
            return result;
        }

        /// <summary>将欧拉角转换为四元数</summary>
        public static Quaternion QuaternionFromEuler(Vector3 euler) {
            MathFloat_QuaternionFromEuler(euler, out Quaternion result);
            return result;
        }

        /// <summary>将fromDirection转向toDirection的四元数</summary>
        public static Quaternion QuaternionFromToRotation(Vector3 fromDirection, Vector3 toDirection) {
            MathFloat_QuaternionFromToRotation(fromDirection, toDirection, out Quaternion result);
            return result;
        }

        /// <summary>
        /// 通过前方向和上方向构建观察矩阵（View Matrix），该矩阵将世界空间中的坐标转换到局部空间（View Space）。
        /// 等价于inverse(WorldTransform)
        /// </summary>
        public static Quaternion QuaternionLookRotation(Vector3 forward, Vector3 upward) {
            MathFloat_QuaternionLookRotation(forward, upward, out Quaternion result);
            return result;
        }

        /// <summary>
        /// 通过前方向和上方向构建观察矩阵（View Matrix），该矩阵将世界空间中的坐标转换到局部空间（View Space）。
        /// 上方向默认(0,0,1)。
        /// 等价于inverse(WorldTransform)，但特殊上方向。
        /// </summary>
        public static Quaternion QuaternionLookRotation(Vector3 forward) {
            return QuaternionLookRotation(forward, Vector3.Up);
        }

        /// <summary>四元数取逆</summary>
        public static Quaternion QuaternionInverse(Quaternion quaternion) {
            MathFloat_QuaternionInverse(quaternion, out Quaternion result);
            return result;
        }

        /// <summary>四元数转换成欧拉角</summary>
        public static Vector3 EulerFromQuaternion(Quaternion quaternion) {
            MathFloat_EulerFromQuaternion(quaternion, out Vector3 result);
            return result;
        }

        /// <summary>矩阵取逆</summary>
        public static Matrix4x4 Inverse(Matrix4x4 m4) {
            MathFloat_Inverse(m4, out Matrix4x4 result);
            return result;
        }

        /// <summary>转置（Transpose），即交换矩阵的行和列</summary>
        public static Matrix4x4 Transpose(Matrix4x4 m4) {
            MathFloat_Transpose(m4, out Matrix4x4 result);
            return result;
        }

        /// <summary>获取Transform矩阵</summary>
        public static Matrix4x4 TRS(Vector3 translation, Quaternion rotation, Vector3 scale) {
            MathFloat_TRS(translation, rotation, scale, out Matrix4x4 result);
            return result;
        }

        /// <summary>
        /// 透视投影矩阵（Perspective Projection Matrix）
        /// 将摄像机空间（View Space）中的 3D 坐标转换为裁剪空间（Clip Space）
        /// </summary>
        /// <param name="fov">垂直方向的视野角度（Field of View）。值越大，视野越广（类似广角镜头）。单位是弧度。</param>
        /// <param name="aspect">屏幕或视口的宽高比。通常为 screenWidth / screenHeight。如果宽高比不匹配，画面会被拉伸。</param>
        /// <param name="zNear">近裁剪平面距离摄像机的位置。必须为正数。太小的值会导致深度精度问题（Z - fighting）。</param>
        /// <param name="zFar">远裁剪平面距离摄像机的位置。必须大于 near。太大的值会导致深度精度下降，建议根据场景视距谨慎设置。</param>
        /// <returns></returns>
        public static Matrix4x4 Perspective(float fov, float aspect, float zNear, float zFar) {
            MathFloat_Perspective(fov, aspect, zNear, zFar, out Matrix4x4 result);
            return result;
        }

        /// <summary>
        /// 叉乘结果向量C垂直于A和B构成的平面，方向由右手定则确定。
        /// 模长等于以A、B为邻边的平行四边形面积 |C|=|A||B|sinθ
        /// </summary>
        public static Vector3 Cross(Vector3 v1, Vector3 v2) {
            MathFloat_Cross(v1, v2, out Vector3 result);
            return result;
        }

        /// <summary>
        /// 构建观察矩阵（View Matrix），将世界空间中的坐标转换到摄像机空间（View Space）
        /// 等价于glm::inverse(cameraEntity->GetWorldTransform())
        /// 优势是无需维护旋转四元数
        /// </summary>
        /// <param name="eye">摄像机在世界空间中的位置</param>
        /// <param name="center">摄像机注视的目标点（世界坐标）</param>
        /// <param name="up">世界空间中的“上方向”参考向量（通常为 (0, 1, 0)）</param>
        /// <returns></returns>
        public static Matrix4x4 LookAt(Vector3 eye, Vector3 center, Vector3 up) {
            MathFloat_LookAt(eye, center, up, out Matrix4x4 result);
            return result;
        }

        /// <summary>单位化Vector3</summary>
        public static Vector3 NormalizedVector3(Vector3 vec)
        {
            MathFloat_NormalizedVector3(vec, out Vector3 result);
            return result;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_Rotate(Quaternion q, Vector3 v, out Vector3 rotate);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_QuaternionFromEuler(Vector3 euler, out Quaternion quaternion);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_QuaternionFromToRotation(Vector3 fromDirection, Vector3 toDirection, out Quaternion quaternion);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_QuaternionLookRotation(Vector3 forward, Vector3 upwards, out Quaternion quaternion);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_QuaternionInverse(Quaternion quaternion, out Quaternion inversedQuaternion);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_EulerFromQuaternion(Quaternion quaternion, out Vector3 Euler);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_Inverse(Matrix4x4 m4, out Matrix4x4 mat4);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_Transpose(Matrix4x4 m4, out Matrix4x4 mat4);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_TRS(Vector3 translation, Quaternion rotation, Vector3 scale, out Matrix4x4 TRS);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_Perspective(float fov, float aspect, float zNear, float zFar, out Matrix4x4 perspective);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_Cross(Vector3 v1, Vector3 v2, out Vector3 cross);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_LookAt(Vector3 eye, Vector3 center, Vector3 up, out Matrix4x4 lookAt);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MathFloat_NormalizedVector3(Vector3 vector3, out Vector3 result);

    }
}
