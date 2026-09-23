using System;

namespace Volcano
{
    // 无需附加到GameObject即可独立存在的数据容器，用于创建纯数据配置对象的基类
    public class ScriptableObject : Volcano.Object
    {
        /// <summary>创建ScriptableObject实例</summary>
        public static T CreateInstance<T>() where T : ScriptableObject { return default; }

        /// <summary>通过类型创建实例</summary>
        public static ScriptableObject CreateInstance(Type type) { return default; }

        /// <summary>ScriptableObject启动时调用</summary>
        private void Awake() { }

        /// <summary>对象加载时调用</summary>
        private void OnEnable() { }

        /// <summary>对象超出范围时调用</summary>
        private void OnDisable() { }

        /// <summary>对象即将销毁时调用</summary>
        private void OnDestroy() { }

        /// <summary>编辑器专用，值变化时调用</summary>
        private void OnValidate() { }

        /// <summary>编辑器专用，重置为默认值</summary>
        private void Reset() { }
    }
}
