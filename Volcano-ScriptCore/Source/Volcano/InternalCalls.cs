using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public static class InternalCalls
    {

        public static void DebugTrace(string message) { Debug_Trace(message); }
        public static void DebugInfo(string message)  { Debug_Info(message);  }
        public static void DebugWarn(string message)  { Debug_Warn(message);  }
        public static void DebugError(string message) { Debug_Error(message); }

        // 声明为内部调用：声明这个函数的定义在cpp内部被实现
       // [MethodImplAttribute(MethodImplOptions.InternalCall)]
        //internal extern static object GetScriptInstance(ulong entityID);

        //[MethodImplAttribute(MethodImplOptions.InternalCall)]
        //internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);
        //[MethodImplAttribute(MethodImplOptions.InternalCall)]
        //internal extern static ulong Entity_FindEntityByName(string name);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_IsEntityExist(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)] // 在Scene目录下复制实例
        internal extern static Entity Entity_InstantiateSingle(Entity original, Vector3 position, Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity Entity_InstantiateSingleWithParent(Entity original, Transform parent, Vector3 position, Quaternion rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity Entity_CloneSingle(Entity original);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static Entity Entity_CloneSingleWithParent(Entity original, Transform parent, bool instantiateInWorldSpace);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_Destroy(Entity entity, float time);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool MouseBuffer_GetMouseOnActive();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MouseBuffer_SetMouseOnActive(ref bool onActive);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Debug_Trace(string message);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Debug_Info(string message);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Debug_Warn(string message);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Debug_Error(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Application_GetTargetFrameRate(out int targetFrameRate);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Application_SetTargetFrameRate(int targetFrameRate);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MonoBehaviour_InvokeDelayed(ulong entityID, string methodName, float time, float repeatRate);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool MonoBehaviour_IsInvoking(ulong entityID, string methodName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool MonoBehaviour_IsInvokingAll(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MonoBehaviour_CancelInvoke(ulong entityID, string methodName);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MonoBehaviour_CancelInvokeAll(ulong entityID);

    }
}