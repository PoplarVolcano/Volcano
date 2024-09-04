using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public static class InternalCalls
    {
        // 声明为内部调用：声明这个函数的定义在cpp内部被实现
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_FindEntityByName(string name);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool MouseBuffer_GetMouseOnActive();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void MouseBuffer_SetMouseOnActive(ref bool onActive);

    }
}