using System.Runtime.CompilerServices;

namespace Volcano
{
    public class TransformComponent : Component
    {
        public Vector3 translation
        {
            get
            {
                TransformComponent_GetTranslation(Entity.ID, out Vector3 result);
                return result;
            }
            set
            {
                TransformComponent_SetTranslation(Entity.ID, ref value);
            }
        }

        // The rotation as Euler angles in degrees.
        public Vector3 rotation
        {
            get
            {
                TransformComponent_GetRotation(Entity.ID, out Vector3 result);
                return result;
            }
            set
            {
                TransformComponent_SetRotation(Entity.ID, ref value);
            }
        }
        public Vector3 scale
        {
            get
            {
                TransformComponent_GetSacle(Entity.ID, out Vector3 result);
                return result;
            }
            set
            {
                TransformComponent_SetSacle(Entity.ID, ref value);
            }
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(ulong entityID, out Vector3 rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetSacle(ulong entityID, out Vector3 sacle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetSacle(ulong entityID, ref Vector3 sacle);
    }
}
