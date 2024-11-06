using System.Runtime.CompilerServices;

namespace Volcano
{
    public class TransformComponent : Component
    {
        public Vector3 localPosition
        {
            get { TransformComponent_GetLocalPosition(Entity.ID, out Vector3 result); return result; }
            set { TransformComponent_SetLocalPosition(Entity.ID, ref value); }
        }

        public Vector3 position
        {
            get { TransformComponent_GetPosition(Entity.ID, out Vector3 result); return result; }
            set { TransformComponent_SetPosition(Entity.ID, ref value); }
        }

        // The rotation as Euler angles in degrees.
        public Vector3 localEulerAngles
        {
            get { TransformComponent_GetLocalEulerAngles(Entity.ID, out Vector3 result); return result; }
            set { TransformComponent_SetLocalEulerAngles(Entity.ID, ref value); }
        }

        public Vector3 eulerAngles
        {
            get { TransformComponent_GetEulerAngles(Entity.ID, out Vector3 result); return result; }
            set { TransformComponent_SetEulerAngles(Entity.ID, ref value); }
        }

        public Quaternion localRotation
        {
            get { TransformComponent_GetLocalRotation(Entity.ID, out Quaternion result); return result; }
            set { TransformComponent_SetLocalRotation(Entity.ID, ref value); }
        }

        public Quaternion rotation
        {
            get { TransformComponent_GetRotation(Entity.ID, out Quaternion result); return result; }
            set { TransformComponent_SetRotation(Entity.ID, ref value); }
        }
        public Vector3 scale
        {
            get { TransformComponent_GetSacle(Entity.ID, out Vector3 result); return result; }
            set { TransformComponent_SetSacle(Entity.ID, ref value); }
        }

        // The parent of the transform.
        public TransformComponent parent
        {
            get { return Entity.parent.transform; }
            set { Entity.parent.transform = value; }
        }


        // Transforms /direction/ from local space to world space.
        Vector3 TransformDirection(Vector3 direction) { TransformComponent_TransformDirection(Entity.ID, direction, out Vector3 result); return result; }

        // Transforms /direction/ from world space to local space.
        Vector3 InverseTransformDirection(Vector3 direction) { TransformComponent_InverseTransformDirection(Entity.ID, direction, out Vector3 result); return result; }

        public void Translate(float x, float y, float z, Space relativeTo) { Translate(new Vector3(x, y, z), relativeTo); }
        public void Translate(float x, float y, float z) { Translate(new Vector3(x, y, z), Space.Self); }
        public void Translate(Vector3 translation, Space relativeTo)
        {
            if (relativeTo == Space.World)
                position += translation;
            else
                position += TransformDirection(translation);
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
        public void LookAt(TransformComponent target, Vector3 worldUp) { if (target != null) LookAt(target.position, worldUp); }
        public void LookAt(TransformComponent target) { if (target != null) LookAt(target.position, Vector3.up); }

        // Rotates the transform so the forward vector points at /worldPosition/.
        public void LookAt(Vector3 worldPosition, Vector3 worldUp) { TransformComponent_LookAt(Entity.ID, worldPosition, worldUp); }
        public void LookAt(Vector3 worldPosition) { TransformComponent_LookAt(Entity.ID, worldPosition, Vector3.up); }



        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetLocalPosition(ulong entityID, out Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetLocalPosition(ulong entityID, ref Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetPosition(ulong entityID, out Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetPosition(ulong entityID, ref Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetLocalEulerAngles(ulong entityID, out Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetLocalEulerAngles(ulong entityID, ref Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetEulerAngles(ulong entityID, out Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetEulerAngles(ulong entityID, ref Vector3 eulerAngles);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetLocalRotation(ulong entityID, out Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetLocalRotation(ulong entityID, ref Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(ulong entityID, out Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(ulong entityID, ref Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetSacle(ulong entityID, out Vector3 sacle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetSacle(ulong entityID, ref Vector3 sacle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_TransformDirection(ulong entityID, Vector3 direction, out Vector3 transformedDirections);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_InverseTransformDirection(ulong entityID, Vector3 direction, out Vector3 transformedDirections);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_LookAt(ulong entityID, Vector3 worldPosition, Vector3 worldUp);

    }
}
