using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public class Transform : Volcano.Component
    {
        /// <summary>世界空间中的位置</summary>
        public Vector3 Position
        {
            get { Transform_GetPosition(ID, out Vector3 result); return result; }
            set { Transform_SetPosition(ID, ref value); }
        }

        /// <summary>相对于父物体的本地位置</summary>
        public Vector3 LocalPosition
        {
            get { Transform_GetLocalPosition(ID, out Vector3 result); return result; }
            set { Transform_SetLocalPosition(ID, ref value); }
        }

        /// <summary>世界空间中的旋转（四元数）</summary>
        public Quaternion Rotation
        {
            get { Transform_GetRotation(ID, out Quaternion result); return result; }
            set { Transform_SetRotation(ID, ref value); }
        }

        /// <summary>相对于父物体的本地旋转（四元数）</summary>
        public Quaternion LocalRotation
        {
            get { Transform_GetLocalRotation(ID, out Quaternion result); return result; }
            set { Transform_SetLocalRotation(ID, ref value); }
        }

        /// <summary>世界空间中的欧拉角（度）</summary>
        public Vector3 EulerAngles
        {
            get { Transform_GetEulerAngles(ID, out Vector3 result); return result; }
            set { Transform_SetEulerAngles(ID, ref value); }
        }

        /// <summary>相对于父物体的本地欧拉角（度）</summary>
        public Vector3 LocalEulerAngles
        {
            get { Transform_GetLocalEulerAngles(ID, out Vector3 result); return result; }
            set { Transform_SetLocalEulerAngles(ID, ref value); }
        }


        /// <summary>相对于父物体的缩放</summary>
        public Vector3 LocalScale
        {
            get { Transform_GetLocalScale(ID, out Vector3 result); return result; }
            set { Transform_SetLocalScale(ID, ref value); }
        }

        /// <summary>世界空间中的全局缩放（只读）</summary>
        public Vector3 LossyScale
        {
            get { Transform_GetScale(ID, out Vector3 result); return result; }
        }

        /// <summary>父级 Transform</summary>
        public Transform Parent
        {
            get { return Transform_GetParent(ID); }
            set { Transform_SetParent(ID, value);}
        }

        /// <summary>层级根节点 Transform（只读）</summary>
        public Transform Root { get; }

        /// <summary>子物体数量</summary>
        public int ChildCount
        {
            get { return Transform_GetChildrenCount(ID); }
        }

        /// <summary>世界空间中 Transform 的蓝轴（Z轴）方向</summary>
        public Vector3 Forward
        {
            get { return Rotation * Vector3.Forward; }
        }

        /// <summary>世界空间中 Transform 的红轴（X轴）方向</summary>
        public Vector3 Right
        {
            get { return Rotation * Vector3.Right; }
        }

        /// <summary>世界空间中 Transform 的绿轴（Y轴）方向</summary>
        public Vector3 Up
        {
            get { return Rotation * Vector3.Up; }
        }

        /// <summary>本地→世界变换矩阵（只读）</summary>
        public Matrix4x4 LocalToWorldMatrix { get; }

        /// <summary>世界→本地变换矩阵（只读）</summary>
        public Matrix4x4 WorldToLocalMatrix { get; }

        /// <summary>自上次重置后变换是否被修改</summary>
        public bool HasChanged { get; set; }

        /// <summary>层级视图容器的容量</summary>
        public int HierarchyCapacity { get; set; }

        /// <summary>层级视图容器中的对象数量</summary>
        public int HierarchyCount { get; }

        /// <summary>将所有子物体的父级设为 null</summary>
        public void DetachChildren() { }

        /// <summary>按名称查找子物体（返回 Transform）</summary>
        public Transform Find(string name) { return default; }

        /// <summary>按索引获取子物体 Transform</summary>
        public Transform GetChild(int index) { return default; }

        /// <summary>获取同级索引</summary>
        public int GetSiblingIndex() { return default; }

        /// <summary>移动到同级列表开头</summary>
        public void SetAsFirstSibling() { }

        /// <summary>移动到同级列表末尾</summary>
        public void SetAsLastSibling() { }

        /// <summary>设置同级索引</summary>
        public void SetSiblingIndex(int index) { }

        /// <summary>设置父级 Transform（默认保持世界位置）</summary>
        public void SetParent(Transform parent) { }

        /// <summary>设置父级 Transform，可指定是否保持世界位置</summary>
        public void SetParent(Transform parent, bool worldPositionStays) { }

        /// <summary>判断是否为指定 Transform 的子级</summary>
        public bool IsChildOf(Transform parent) { return default; }

        /// <summary>旋转变换使其 forward 指向目标（重载1）</summary>
        public void LookAt(Transform target) { }

        /// <summary>旋转变换使其 forward 指向世界位置（重载2）</summary>
        public void LookAt(Vector3 worldPosition) { }

        /// <summary>旋转变换使其 forward 指向世界位置，并指定上方向（重载3）</summary>
        public void LookAt(Vector3 worldPosition, Vector3 worldUp) { }

        /// <summary>按欧拉角旋转（默认 Space.Self）</summary>
        public void Rotate(Vector3 eulerAngles) {
            Rotate(eulerAngles, Space.Self);
        }

        /// <summary>按欧拉角旋转，指定坐标系</summary>
        public void Rotate(Vector3 eulerAngles, Space relativeTo) {
            // 参数eulerAngles为角度，要转换为弧度再进行计算
            Vector3 eulersRadian = eulerAngles * MathFloat.DegreeToRadian;
            // 四元数相乘，一个物体先绕某个轴旋转一定角度，再绕另一个轴（或同一轴的不同角度）旋转，最终得到的位置和方向由这两个旋转的复合决定。
            Quaternion eulerRotation = Quaternion.Euler(eulersRadian.x, eulersRadian.y, eulersRadian.z);
            if (relativeTo == Space.Self)
            {
                LocalRotation *= eulerRotation;
            }
            else
            {
                Rotation *= (MathFloat.QuaternionInverse(Rotation) * eulerRotation * Rotation);
            }
        }

        /// <summary>按欧拉角旋转（参数分解）</summary>
        public void Rotate(float xAngle, float yAngle, float zAngle)
        {
            Rotate(new Vector3(xAngle, yAngle, zAngle), Space.Self);
        }

        /// <summary>按欧拉角旋转（参数分解 + 坐标系）</summary>
        public void Rotate(float xAngle, float yAngle, float zAngle, Space relativeTo)
        {
            Rotate(new Vector3(xAngle, yAngle, zAngle), relativeTo);
        }

        /// <summary>绕指定轴旋转指定角度（默认 Space.Self）</summary>
        public void Rotate(Vector3 axis, float angle) { }

        /// <summary>绕指定轴旋转指定角度，指定坐标系</summary>
        public void Rotate(Vector3 axis, float angle, Space relativeTo) { }

        /// <summary>绕世界空间中的 point 点，绕 axis 轴旋转 angle 度</summary>
        public void RotateAround(Vector3 point, Vector3 axis, float angle) { }

        /// <summary>沿指定方向移动（默认 Space.Self）</summary>
        public void Translate(Vector3 translation) { }

        /// <summary>沿指定方向移动，指定坐标系</summary>
        public void Translate(Vector3 translation, Space relativeTo) { }

        /// <summary>沿各轴移动（参数分解，默认 Space.Self）</summary>
        public void Translate(float x, float y, float z) { }

        /// <summary>沿各轴移动（参数分解 + 坐标系）</summary>
        public void Translate(float x, float y, float z, Space relativeTo) { }

        /// <summary>将方向从本地→世界</summary>
        public Vector3 TransformDirection(Vector3 direction) { return default; }

        /// <summary>批量将方向从本地→世界</summary>
        public void TransformDirections(Vector3[] directions) { }

        /// <summary>将方向从世界→本地</summary>
        public Vector3 InverseTransformDirection(Vector3 direction) { return default; }

        /// <summary>批量将方向从世界→本地</summary>
        public void InverseTransformDirections(Vector3[] directions) { }

        /// <summary>将点从本地→世界</summary>
        public Vector3 TransformPoint(Vector3 position) { return default; }

        /// <summary>批量将点从本地→世界</summary>
        public void TransformPoints(Vector3[] points) { }

        /// <summary>将点从世界→本地</summary>
        public Vector3 InverseTransformPoint(Vector3 position) { return default; }

        /// <summary>批量将点从世界→本地</summary>
        public void InverseTransformPoints(Vector3[] points) { }

        /// <summary>将向量从本地→世界（不受平移影响）</summary>
        public Vector3 TransformVector(Vector3 vector) { return default; }

        /// <summary>批量将向量从本地→世界</summary>
        public void TransformVectors(Vector3[] vectors) { }

        /// <summary>将向量从世界→本地（不受平移影响）</summary>
        public Vector3 InverseTransformVector(Vector3 vector) { return default; }

        /// <summary>批量将向量从世界→本地</summary>
        public void InverseTransformVectors(Vector3[] vectors) { }

        /// <summary>一次获取世界位置和旋转（性能优化）</summary>
        public void GetPositionAndRotation(out Vector3 position, out Quaternion rotation) { position = new Vector3(); rotation = new Quaternion(); }

        /// <summary>一次设置世界位置和旋转（性能优化）</summary>
        public void SetPositionAndRotation(Vector3 position, Quaternion rotation) { }

        /// <summary>一次获取本地位置和旋转（性能优化）</summary>
        public void GetLocalPositionAndRotation(out Vector3 localPosition, out Quaternion localRotation) { localPosition = new Vector3(); localRotation = new Quaternion(); }

        /// <summary>一次设置本地位置和旋转（性能优化）</summary>
        public void SetLocalPositionAndRotation(Vector3 localPosition, Quaternion localRotation) { }

        // ========== 实现 IEnumerable 接口（用于 foreach） ==========
        public System.Collections.IEnumerator GetEnumerator() { return default; }





        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetPosition(ulong entityID, out Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetPosition(ulong entityID, ref Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetLocalPosition(ulong entityID, out Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetLocalPosition(ulong entityID, ref Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetRotation(ulong entityID, out Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetRotation(ulong entityID, ref Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetLocalRotation(ulong entityID, out Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetLocalRotation(ulong entityID, ref Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetEulerAngles(ulong entityID, out Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetEulerAngles(ulong entityID, ref Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetLocalEulerAngles(ulong entityID, out Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetLocalEulerAngles(ulong entityID, ref Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetLocalScale(ulong entityID, out Vector3 sacle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetLocalScale(ulong entityID, ref Vector3 sacle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetScale(ulong entityID, out Vector3 sacle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_VectorTransformSpace(ulong entityID, Vector3 vec, out Vector3 transformedDirections);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_PointTransformSpace(ulong entityID, Vector3 point, out Vector3 transformedDirections);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetTransform(ulong entityID, out Transform transform);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Transform Transform_GetParent(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetParent(ulong entityID, Transform parent);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetChildren(ulong entityID, out ulong[] children);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Transform Transform_GetChild(ulong entityID, int index);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Transform_GetChildrenCount(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_DetachChildren(ulong entityID);

    }
}
