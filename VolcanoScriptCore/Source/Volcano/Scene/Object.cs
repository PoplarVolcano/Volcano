using System;

namespace Volcano
{
    //  Volcano 中所有对象的基类
    public class Object
    {
        public enum FindObjectsSortMode
        {
            /// <summary>不对结果进行排序</summary>
            None,
            /// <summary>按 InstanceID 升序排序</summary>
            InstanceID
        }

        public Object() { ID = 0; }
        public Object(ulong id) { ID = id; }


        protected readonly ulong ID;

        public string Name {  get; set; }

        public ulong GetInstanceID() { return ID; }
        public override string ToString()  { return Name; }


        /// <summary>销毁 GameObject、组件或资源</summary>
        public static void Destroy(Volcano.Object obj) { }

        /// <summary>立即销毁对象（建议优先使用 Destroy）</summary>
        public static void DestroyImmediate(Volcano.Object obj) { }

        /// <summary>场景切换时不销毁该对象</summary>
        public static void DontDestroyOnLoad(Volcano.Object obj) { }

        /// <summary>克隆对象并返回副本</summary>
        public static Volcano.Object Instantiate(Volcano.Object original) { return null; }

        /// <summary>查找一个类型为 T 的已加载激活对象</summary>
        public static T FindAnyObjectByType<T>() { return default; }

        /// <summary>查找第一个类型为 T 的已加载激活对象</summary>
        public static T FindFirstObjectByType<T>() { return default; }

        public static T[] FindObjectsByType<T>(FindObjectsSortMode sortMode) { return default; }

        /// <summary>比较两个对象引用是否相同</summary>
        public static bool operator ==(Volcano.Object x, Volcano.Object y)
        {
            return x.ID == y.ID;
        }

        /// <summary>比较两个对象引用是否不同</summary>
        public static bool operator !=(Volcano.Object x, Volcano.Object y)
        {
            return x.ID != y.ID;
        }

        /// <summary>对象是否存在</summary>
        public static implicit operator bool(Volcano.Object x)
        {
            return x.ID == 0;
        }

    }
}
