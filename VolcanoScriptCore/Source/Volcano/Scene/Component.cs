using System;

namespace Volcano
{
    // 对核心中的脚本类的读取，本质上是一个有ID的Object，所有脚本引用的字段全都通过ID访问C++获取，
    // 而且获取的都是Object(ID)构造的对应脚本类

    // 所有可挂载到 GameObject 上的组件基类
    public class Component : Volcano.Object
    {
        /// <summary>此组件附加到的游戏对象</summary>
        public GameObject GameObject
        {
            get { InternalCalls.GameObject_GetGameObject(ID, out GameObject gameObject); return gameObject; }
        }

        /// <summary>此组件附加到的游戏对象 GameObject 的 Transform</summary>
        public Transform Transform
        {
            get { Transform.Transform_GetTransform(ID, out Transform transform); return transform; }
        }

        /// <summary>此组件附加到的游戏对象 GameObject 的标签</summary>
        public string Tag { get; set; }

        /// <summary>判断标签是否匹配</summary>
        public bool CompareTag(string tag) { return default; }

        /// <summary>返回指定类型的组件</summary>
        public T GetComponent<T>() where T : Component { return default; }

        /// <summary>在子对象中查找组件</summary>
        public T GetComponentInChildren<T>(bool includeInactive = false) where T : Component { return default; }

        /// <summary>在父对象中查找组件</summary>
        public T GetComponentInParent<T>(bool includeInactive = false) where T : Component { return default; }

        /// <summary>返回所有指定类型的组件</summary>
        public T[] GetComponents<T>() where T : Component { return default; }

        /// <summary>返回此组件附加到的游戏对象 GameObject 及所有子对象中的组件</summary>
        public T[] GetComponentsInChildren<T>(bool includeInactive = false) where T : Component { return default; }

        /// <summary>返回此组件附加到的游戏对象 GameObject 及所有父对象中的组件</summary>
        public T[] GetComponentsInParent<T>(bool includeInactive = false) where T : Component { return default; }

        /// <summary>尝试获取组件</summary>
        public bool TryGetComponent<T>(out T component) where T : Component { component = default; return default; }

    }
}