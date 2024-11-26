using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public class GameObject : Entity
    {
        public Component GetComponent(Type type)
        {
            return GameObject_GetComponent(ID, type);
        }

        public unsafe T GetComponent<T>()
        {
            //var h = new CastHelper<T>();
            //GameObject_GetComponentFastPath(ID, typeof(T), new System.IntPtr(&h.onePointerFurtherThanT));
            //InternalCalls.DebugError(typeof(T).ToString());
            //return h.t;

            GameObject_GetComponentFastPath(ID, typeof(T), out T result);
            return result;
        }

        public Component GetComponent(string type)
        {
            return GameObject_GetComponentByName(ID, type);
        }

        public Component GetComponentInChildren(Type type, bool includeInactive)
        {
            return GameObject_GetComponentInChildren(ID, type, includeInactive);
        }

        public Component GetComponentInChildren(Type type)
        {
            return GetComponentInChildren(type, false);
        }

        public T GetComponentInChildren<T>()
        {
            bool includeInactive = false;
            return GetComponentInChildren<T>(includeInactive);
        }

        public T GetComponentInChildren<T>(bool includeInactive)
        {
            return (T)(object)GameObject_GetComponentInChildren(ID, typeof(T), includeInactive);
        }

        public Component GetComponentInParent(Type type, bool includeInactive)
        {
            return GameObject_GetComponentInParent(ID, type, includeInactive);
        }

        public Component GetComponentInParent(Type type)
        {
            return GetComponentInParent(type, false);
        }

        public T GetComponentInParent<T>()
        {
            bool includeInactive = false;
            return GetComponentInParent<T>(includeInactive);
        }

        public T GetComponentInParent<T>(bool includeInactive)
        {
            return (T)(object)GameObject_GetComponentInParent(ID, typeof(T), includeInactive);
        }

        public unsafe bool TryGetComponent<T>(out T component)
        {
            var h = new CastHelper<T>();
            GameObject_TryGetComponentFastPath(ID, typeof(T), new System.IntPtr(&h.onePointerFurtherThanT));
            component = h.t;
            return h.t != null;
        }

        public bool TryGetComponent(Type type, out Component component)
        {
            component = GameObject_TryGetComponentInternal(ID, type);
            return component != null;
        }

        public Component AddComponent(Type componentType)
        {
            return GameObject_AddComponentWithType(ID, componentType);
        }

        public T AddComponent<T>() where T : Component
        {
            return AddComponent(typeof(T)) as T;
        }

        public Component GetComponentAtIndex(int index)
        {
            if (index < 0 || index >= GameObject_GetComponentCount(ID)) throw new ArgumentOutOfRangeException(nameof(index), "Valid range is 0 to GetComponentCount() - 1.");
            return GameObject_QueryComponentAtIndex(ID, index);
        }

        public T GetComponentAtIndex<T>(int index) where T : Component
        {
            T component = (T)GetComponentAtIndex(index);
            if (component == null) throw new InvalidCastException();
            return component;
        }

        /*
        public GameObject(string name)
        {
            GameObject_CreateGameObject(this, name);
        }

        public GameObject()
        {
            GameObject_CreateGameObject(this, null);
        }

        public GameObject(string name, params Type[] components)
        {
            GameObject_CreateGameObject(this, name);
            foreach (Type t in components)
                AddComponent(t);
        }
        */
        public GameObject gameObject { get { return this; } }

        public static GameObject Find(string name)
        {
            return GameObject_Find(name);
        }

        public Transform transform {  get { return GetComponent<Transform>(); } }

        //public extern int layer { get; set; }

        public bool active
        {
            get => GameObject_GetActive(ID);
            set => GameObject_SetActive(ID, value);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)] // 通过Type获取Entity的Component
        private extern static Component GameObject_GetComponent(ulong entityID, Type type);
        //[MethodImplAttribute(MethodImplOptions.InternalCall)] // 通过Type获取Entity的Component,赋给oneFurtherThanResultValue
        //private extern static void GameObject_GetComponentFastPath(ulong entityID, Type type, IntPtr oneFurtherThanResultValue);
        [MethodImplAttribute(MethodImplOptions.InternalCall)] // 通过Type获取Entity的Component,赋给oneFurtherThanResultValue
        private extern static void GameObject_GetComponentFastPath<T>(ulong entityID, Type type, out T component);
        [MethodImplAttribute(MethodImplOptions.InternalCall)] // 通过组件名获取Entity的Component
        private extern static Component GameObject_GetComponentByName(ulong entityID, string type);
        [MethodImplAttribute(MethodImplOptions.InternalCall)] // 通过Type获取Entity子节点的Component,includeInactive:是否包含未启动子节点
        private extern static Component GameObject_GetComponentInChildren(ulong entityID, Type type, bool includeInactive);
        [MethodImplAttribute(MethodImplOptions.InternalCall)] // 通过Type获取Entity父节点的Component,includeInactive:是否包含未启动父节点
        private extern static Component GameObject_GetComponentInParent(ulong entityID, Type type, bool includeInactive);
        [MethodImplAttribute(MethodImplOptions.InternalCall)] // Entity是否有type类型的Component
        private extern static Component GameObject_TryGetComponentInternal(ulong entityID, Type type);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void GameObject_TryGetComponentFastPath(ulong entityID, Type type, IntPtr oneFurtherThanResultValue);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static Component GameObject_AddComponentInternal(ulong entityID, string className);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static Component GameObject_AddComponentWithType(ulong entityID, Type componentType);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static int GameObject_GetComponentCount(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static Component GameObject_QueryComponentAtIndex(ulong entityID, int index);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static int GameObject_GetComponentIndex(ulong entityID, Component component);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static bool GameObject_GetActive(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void GameObject_SetActive(ulong entityID, bool value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static void GameObject_CreateGameObject(GameObject self, string name);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern static GameObject GameObject_Find(string name);

    }
}
