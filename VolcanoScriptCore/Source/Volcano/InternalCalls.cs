using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public static class InternalCalls
    {

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool MouseBuffer_GetMouseOnActive();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MouseBuffer_SetMouseOnActive(bool onActive);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void Debug_Trace(string message);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void Debug_Info(string message);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void Debug_Warn(string message);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern static void Debug_Error(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GameObject_Destroy(GameObject gameObject, float time);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GameObject_GetGameObject(ulong entityID, out GameObject gameObject);

    }
}
