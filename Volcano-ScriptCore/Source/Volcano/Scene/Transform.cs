using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using Volcano.Bindings;

namespace Volcano
{
    public class Transform : Component
    {
        public Vector3 localPosition
        {
            get { Transform_GetLocalPosition(ID, out Vector3 result); return result; }
            set { Transform_SetLocalPosition(ID, ref value); }
        }

        public Vector3 position
        {
            get { Transform_GetPosition(ID, out Vector3 result); return result; }
            set { Transform_SetPosition(ID, ref value); }
        }

        // The rotation as Euler angles in degrees.
        public Vector3 localEulerAngles
        {
            get { Transform_GetLocalEulerAngles(ID, out Vector3 result); return result; }
            set { Transform_SetLocalEulerAngles(ID, ref value); }
        }

        public Vector3 eulerAngles
        {
            get { Transform_GetEulerAngles(ID, out Vector3 result); return result; }
            set { Transform_SetEulerAngles(ID, ref value); }
        }

        public Quaternion localRotation
        {
            get { Transform_GetLocalRotation(ID, out Quaternion result); return result; }
            set { Transform_SetLocalRotation(ID, ref value); }
        }

        public Quaternion rotation
        {
            get { Transform_GetRotation(ID, out Quaternion result); return result; }
            set { Transform_SetRotation(ID, ref value); }
        }
        public Vector3 scale
        {
            get { Transform_GetSacle(ID, out Vector3 result); return result; }
            set { Transform_SetSacle(ID, ref value); }
        }

        // Transforms /direction/ from local space to world space.
        Vector3 TransformSpace(Vector3 vec) { Transform_TransformSpace(ID, vec, out Vector3 result); return result; }

        // Transforms direction /x/, /y/, /z/ from local space to world space.
        public Vector3 TransformSpace(float x, float y, float z) { return TransformSpace(new Vector3(x, y, z)); }

        // Transforms /direction/ from world space to local space.
        Vector3 InverseTransformSpace(Vector3 vec) { Transform_InverseTransformSpace(ID, vec, out Vector3 result); return result; }

        // Transforms the direction /x/, /y/, /z/ from world space to local space. The opposite of Transform.TransformDirection.
        public Vector3 InverseTransformSpace(float x, float y, float z) { return InverseTransformSpace(new Vector3(x, y, z)); }


        public void Translate(float x, float y, float z, Space relativeTo) { Translate(new Vector3(x, y, z), relativeTo); }
        public void Translate(float x, float y, float z) { Translate(new Vector3(x, y, z), Space.Self); }
        public void Translate(Vector3 translation, Space relativeTo)
        {
            if (relativeTo == Space.World)
                position += translation;
            else
                position += TransformSpace(translation);
        }
        public void Translate(Vector3 translation) { Translate(translation, Space.Self); }

        public void RotateEulerAngle(float xAngle, float yAngle, float zAngle, Space relativeTo) { RotateEulerAngle(new Vector3(xAngle, yAngle, zAngle), relativeTo); }
        public void RotateEulerAngle(float xAngle, float yAngle, float zAngle) { RotateEulerAngle(new Vector3(xAngle, yAngle, zAngle), Space.Self); }
        public void RotateEulerAngle(Vector3 eulers, Space relativeTo)
        {
            Vector3 eulersRadian = eulers * MathF.Deg2Rad;
            if (relativeTo == Space.Self)
            {
                localEulerAngles += eulersRadian;
            }
            else
            {
                eulerAngles += eulersRadian;
            }
        }
        public void RotateEulerAngle(Vector3 eulers) { RotateEulerAngle(eulers, Space.Self); }

        // 参数angle为角度
        public void Rotate(float xAngle, float yAngle, float zAngle, Space relativeTo) { Rotate(new Vector3(xAngle, yAngle, zAngle), relativeTo); }
        public void Rotate(float xAngle, float yAngle, float zAngle) { Rotate(new Vector3(xAngle, yAngle, zAngle), Space.Self); }
        public void Rotate(Vector3 eulers, Space relativeTo)
        {
            // 参数eulers为角度，要转换为弧度再进行计算
            Vector3 eulersRadian = eulers * MathF.Deg2Rad;
            // 四元数相乘，一个物体先绕某个轴旋转一定角度，再绕另一个轴（或同一轴的不同角度）旋转，最终得到的位置和方向由这两个旋转的复合决定。
            Quaternion eulerRot = Quaternion.Euler(eulersRadian.x, eulersRadian.y, eulersRadian.z);
            if (relativeTo == Space.Self)
            {
                localRotation *= eulerRot;
            }
            else
            {
                rotation *= (MathF.QuaternionInverse(rotation) * eulerRot * rotation);
            }
        }
        public void Rotate(Vector3 eulers) { Rotate(eulers, Space.Self); }


        // Rotates the transform so the forward vector points at /target/'s current position.
        public void LookAt(Transform target, Vector3 worldUp) { if (target != null) LookAt(target.position, worldUp); }
        public void LookAt(Transform target) { if (target != null) LookAt(target.position, Vector3.up); }

        // Rotates the transform so the forward vector points at /worldPosition/.
        public void LookAt(Vector3 worldPosition, Vector3 worldUp) { Transform_LookAt(ID, worldPosition, worldUp); }
        public void LookAt(Vector3 worldPosition) { Transform_LookAt(ID, worldPosition, Vector3.up); }


        // The parent of the transform.
        public Transform parent
        {
            get { return Transform_GetParent(ID); }
            set { Transform_SetParent(ID, value); }
        }

        public int childCount
        {
            get => Transform_GetChildrenCount(ID);
        }

        public Entity[] children
        {
            get
            {
                Transform_GetChildren(ID, out ulong[] result);
                List<ulong> ids = new List<ulong>();
                foreach (ulong id in result)
                {
                    if (Entity.IsEntityExist(this))
                        ids.Add(id);
                }
                Entity[] children = new Entity[ids.Count];
                if (result != null)
                    for (int i = 0; i < ids.Count; i++)
                    {
                        children[i] = new Entity(ids[i]);
                    }
                return children;
            }
        }

        public Transform GetChild(int index)
        {
            return Transform_GetChild(ID, index);
        }

        public void DetachChildren()
        {
            Transform_DetachChildren(ID);
        }

        // Matrix that transforms a point from world space into local space (RO).
        public Matrix4x4 worldToLocalMatrix { get => Transform_WorldToLocalMatrix(ID); }
        // Matrix that transforms a point from local space into world space (RO).
        public Matrix4x4 localToWorldMatrix { get => Transform_LocalToWorldMatrix(ID); }

        // Returns the topmost transform in the hierarchy.
        public Transform root { get { return Transform_GetRoot(ID); } }

        // Move itself to the end of the parent's array of children
        public void SetAsFirstSibling()
        {
            Transform_SetAsFirstSibling(ID);
        }

        // Move itself to the beginning of the parent's array of children
        public void SetAsLastSibling()
        {
            Transform_SetAsLastSibling(ID);
        }

        public void SetSiblingIndex(int index)
        {
            Transform_SetSiblingIndex(ID, index);
        }

        public void MoveAfterSibling(Transform transform, bool notifyEditorAndMarkDirty)
        {
            Transform_MoveAfterSibling(ID, transform, notifyEditorAndMarkDirty);
        }

        public int GetSiblingIndex()
        {
            return Transform_GetSiblingIndex(ID);
        }

        private Transform FindRelativeTransformWithPath(string path, bool isActiveOnly)
        {
            return Transform_FindRelativeTransformWithPath(ID, path, isActiveOnly);
        }

        // Finds a child by /name/ and returns it.
        public Transform Find(string name)
        {
            if (name == null)
                throw new ArgumentNullException("Name cannot be null");
            return FindRelativeTransformWithPath(name, false);
        }

        // Is this transform a child of /parent/?
        public bool IsChildOf(Transform parent)
        {
            return Transform_IsChildOf(ID, parent);
        }

        public Transform FindChild(string name) { return Find(name); }

        public IEnumerator GetEnumerator()
        {
            return new Transform.Enumerator(this);
        }

        private class Enumerator : IEnumerator
        {
            Transform outer;
            int currentIndex = -1;

            internal Enumerator(Transform outer)
            {
                this.outer = outer;
            }

            public object Current
            {
                get { return outer.GetChild(currentIndex); }
            }

            public bool MoveNext()
            {
                int childCount = outer.childCount;
                return ++currentIndex < childCount;
            }

            public void Reset() { currentIndex = -1; }
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_GetLocalPosition(ulong entityID, out Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalPosition(ulong entityID, ref Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_GetPosition(ulong entityID, out Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetPosition(ulong entityID, ref Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_GetLocalEulerAngles(ulong entityID, out Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalEulerAngles(ulong entityID, ref Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_GetEulerAngles(ulong entityID, out Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetEulerAngles(ulong entityID, ref Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_GetLocalRotation(ulong entityID, out Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetLocalRotation(ulong entityID, ref Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_GetRotation(ulong entityID, out Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetRotation(ulong entityID, ref Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_GetSacle(ulong entityID, out Vector3 sacle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetSacle(ulong entityID, ref Vector3 sacle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_TransformSpace(ulong entityID, Vector3 vec, out Vector3 transformedDirections);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_InverseTransformSpace(ulong entityID, Vector3 vec, out Vector3 transformedDirections);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_LookAt(ulong entityID, Vector3 worldPosition, Vector3 worldUp);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static Transform Transform_GetParent(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetParent(ulong entityID, Transform parent);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_GetChildren(ulong entityID, out ulong[] children);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static Transform Transform_GetChild(ulong entityID, int index);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static int Transform_GetChildrenCount(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_DetachChildren(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)] // Move itself to the end of the parent's array of children
        private extern static void Transform_SetAsFirstSibling(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)] // Move itself to the beginning of the parent's array of children
        private extern static void Transform_SetAsLastSibling(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_SetSiblingIndex(ulong entityID, int index);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void Transform_MoveAfterSibling(ulong entityID, Transform transform, bool notifyEditorAndMarkDirty);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static int Transform_GetSiblingIndex(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static Matrix4x4 Transform_WorldToLocalMatrix(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static Matrix4x4 Transform_LocalToWorldMatrix(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static Transform Transform_FindRelativeTransformWithPath(ulong entityID, string name, bool isActiveOnly);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static Transform Transform_GetRoot(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static bool Transform_IsChildOf(ulong entityID, Transform parent);
    }
}
