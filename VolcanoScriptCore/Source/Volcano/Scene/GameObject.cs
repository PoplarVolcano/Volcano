using System;
using System.Collections.Generic;

namespace Volcano
{
    // 游戏对象，即场景中的“实体Entity”容器
    public class GameObject : Volcano.Object
    {
        //public GameObject(string name) { this.Name = name; }

        /// <summary>附加到此 GameObject 的 Transform 组件</summary>
        public Volcano.Transform Transform
        {
            get { Transform.Transform_GetTransform(ID, out Transform transform); return transform; }
        }

        /// <summary>游戏对象所在的层（范围 0-31）</summary>
        public int Layer { get; set; }

        // C# 官方规范：公开的成员（属性/方法/事件）使用 PascalCase（大写开头），即 Tag。
        // 私有字段：使用 _camelCase（如下划线开头）或 m_CamelCase，但现代风格推荐 _tag。
        /// <summary>游戏对象的标签</summary>
        public string Tag { get; set; }

        /// <summary>本地激活状态（只读）</summary>
        public bool ActiveSelf { get; set; }

        /// <summary>游戏对象在场景中是否处于活动状态</summary>
        public bool ActiveInHierarchy { get; set; }

        /// <summary>编辑器 API，指定游戏对象是否为静态</summary>
        public bool IsStatic { get; set; }

        /// <summary>游戏对象所属的场景</summary>
        public Scene Scene { get; set; }

        /// <summary>向 GameObject 添加指定类型的组件</summary>
        public T AddComponent<T>() where T : Component { return default; }

        /// <summary>返回指定类型的组件，不存在则返回 null</summary>
        public T GetComponent<T>() where T : Component { return default; }

        /// <summary>在 GameObject 或其子对象中查找组件</summary>
        public T GetComponentInChildren<T>(bool includeInactive = false) where T : Component 
        { 
            return default;
        }

        /// <summary>在 GameObject 或其父对象中查找组件</summary>
        public T GetComponentInParent<T>(bool includeInactive = false) where T : Component
        {
            return default;
        }

        /// <summary>返回 GameObject 中所有指定类型的组件</summary>
        public T[] GetComponents<T>(List<T> results) where T : Component { return default; }

        /// <summary>返回 GameObject 或其子对象中所有指定类型的组件</summary>
        public T[] GetComponentsInChildren<T>(bool includeInactive = false) where T : Component
        {
            return default;
        }

        /// <summary>返回 GameObject 或其父对象中所有指定类型的组件</summary>
        public T[] GetComponentsInParent<T>(bool includeInactive = false) where T : Component
        {
            return default;
        }

        /// <summary>尝试获取组件，不存在时不产生 GC 分配</summary>
        public bool TryGetComponent<T>(out T component) { component = default;  return default; }

        /// <summary>判断游戏对象是否匹配指定标签</summary>
        public bool CompareTag(string tag) { return default; }

        /// <summary>设置游戏对象的激活状态</summary>
        public void SetActive(bool value) { }

        /// <summary>返回第一个带有指定标签的活动 GameObject</summary>
        public static GameObject FindWithTag(string tag) { return default; }

        /// <summary>返回所有带有指定标签的活动 GameObject</summary>
        public static GameObject[] FindGameObjectsWithTag(string tag) { return default; }
    }
}
