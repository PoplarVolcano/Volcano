using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public class Component : Entity
    {
        public Transform transform { get => GetComponent<Transform>(); }

        public GameObject gameObject { get => Component_GetGameObject(ID); }

        public Component GetComponent(string type)
        {
            return Component_GetComponent(ID, type);
        }

        public Component GetComponent(Type type)
        {
            return gameObject.GetComponent(type);
        }

        public unsafe T GetComponent<T>()
        {
            //var h = new CastHelper<T>();
            //GameObject_GetComponentFastPath(ID, typeof(T), new System.IntPtr(&h.onePointerFurtherThanT));
            //InternalCalls.DebugError(typeof(T).ToString());
            //return h.t;

            Component_GetComponentFastPath(ID, typeof(T), out T result);
            return result;
        }

        public bool TryGetComponent(Type type, out Component component)
        {
            return gameObject.TryGetComponent(type, out component);
        }

        public unsafe bool TryGetComponent<T>(out T component)
        {
            return gameObject.TryGetComponent(out component);
        }

        public Component GetComponentInChildren(Type t, bool includeInactive)
        {
            return gameObject.GetComponentInChildren(t, includeInactive);
        }

        public Component GetComponentInChildren(Type t)
        {
            return GetComponentInChildren(t, false);
        }

        public T GetComponentInChildren<T>(bool includeInactive)
        {
            return (T)(object)GetComponentInChildren(typeof(T), includeInactive);
        }

        public T GetComponentInChildren<T>()
        {
            return (T)(object)GetComponentInChildren(typeof(T), false);
        }

        public Component GetComponentInParent(Type t, bool includeInactive)
        {
            return gameObject.GetComponentInParent(t, includeInactive);
        }

        public Component GetComponentInParent(Type t)
        {
            return gameObject.GetComponentInParent(t, false);
        }

        public T GetComponentInParent<T>(bool includeInactive)
        {
            return (T)(object)GetComponentInParent(typeof(T), includeInactive);
        }

        public T GetComponentInParent<T>()
        {
            return (T)(object)GetComponentInParent(typeof(T), false);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static Component Component_GetComponent(ulong entityID, string type);
        [MethodImplAttribute(MethodImplOptions.InternalCall)] // 通过Type获取Entity的Component,赋给oneFurtherThanResultValue
        private extern static void Component_GetComponentFastPath<T>(ulong entityID, Type type, out T component);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static GameObject Component_GetGameObject(ulong entityID);
    }
}