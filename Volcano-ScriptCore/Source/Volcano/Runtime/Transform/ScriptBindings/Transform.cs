using System;
using System.Collections;
using System.Runtime.CompilerServices;
using Volcano.Bindings;
using Volcano.Scripting;

namespace Volcano
{
    internal enum RotationOrder { OrderXYZ, OrderXZY, OrderYZX, OrderYXZ, OrderZXY, OrderZYX }

    // Position, rotation and scale of an object.
    public partial class Transform : Component
    {
        protected Transform() { }

        // The position of the transform in world space.
        public extern Vector3 position { get; set; }

        // Position of the transform relative to the parent transform.
        public extern Vector3 localPosition { get; set; }

        // The rotation of the transform in world space stored as a [[Quaternion]].
        public extern Quaternion rotation { get; set; }

        // The rotation of the transform relative to the parent transform's rotation.
        public extern Quaternion localRotation { get; set; }

        // The scale of the transform relative to the parent.
        public extern Vector3 localScale { get; set; }

        // Get local euler angles with rotation order specified
        internal extern Vector3 GetLocalEulerAngles(RotationOrder order);

        // Set local euler angles with rotation order specified
        internal extern void SetLocalEulerAngles(Vector3 euler, RotationOrder order);

        // Set local euler hint
        [NativeConditional("VOLCANO_EDITOR")]
        internal extern void SetLocalEulerHint(Vector3 euler);

        // The rotation as Euler angles in degrees.
        public Vector3 eulerAngles { get { return rotation.eulerAngles; } set { rotation = Quaternion.Euler(value); } }

        // The rotation as Euler angles in degrees relative to the parent transform's rotation.
        public Vector3 localEulerAngles { get { return localRotation.eulerAngles; } set { localRotation = Quaternion.Euler(value); } }

        // The red axis of the transform in world space.
        public Vector3 right { get { return rotation * Vector3.right; } set { rotation = MathF.QuaternionFromToRotation(Vector3.right, value); } }

        // The green axis of the transform in world space.
        public Vector3 up { get { return rotation * Vector3.up; } set { rotation = MathF.QuaternionFromToRotation(Vector3.up, value); } }

        // The blue axis of the transform in world space.
        public Vector3 forward { get { return rotation * Vector3.forward; } set { rotation = MathF.QuaternionLookRotation(value); } }



    }
}
