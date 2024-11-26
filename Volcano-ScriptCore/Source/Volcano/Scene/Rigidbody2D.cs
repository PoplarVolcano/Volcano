using System.Runtime.CompilerServices;

namespace Volcano
{
    public class Rigidbody2D : Component
    {
        public enum BodyType { Static = 0, Dynamic, Kinematic }

        public Vector2 LinearVelocity
        {
            get
            {
                Rigidbody2D_GetLinearVelocity(ID, out Vector2 velocity);
                return velocity;
            }
        }

        public BodyType Type
        {
            get => Rigidbody2D_GetType(ID);
            set => Rigidbody2D_SetType(ID, value);
        }

        public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake)
        {
            Rigidbody2D_ApplyLinearImpulse(ID, ref impulse, ref worldPosition, wake);
        }

        public void ApplyLinearImpulse(Vector2 impulse, bool wake)
        {
            Rigidbody2D_ApplyLinearImpulseToCenter(ID, ref impulse, wake);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2D_GetLinearVelocity(ulong entityID, out Vector2 linearVelocity);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Rigidbody2D.BodyType Rigidbody2D_GetType(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2D_SetType(ulong entityID, Rigidbody2D.BodyType type);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2D_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point, bool wake);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2D_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse, bool wake);

    }
}
