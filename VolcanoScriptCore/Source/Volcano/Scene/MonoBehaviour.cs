using System;
using System.Runtime.CompilerServices;
using System.Threading;

namespace Volcano
{
    // 用户自定义脚本的基类
    public class MonoBehaviour : Volcano.Behaviour
    {
        /// <summary>禁用可跳过GUILayout布局阶段</summary>
        public bool UseGUILayout { get; set; }

        /// <summary>允许实例在编辑模式下运行</summary>
        public bool RunInEditMode { get; set; }

        /// <summary>当MonoBehaviour被销毁时触发的取消令牌（只读）</summary>
        public CancellationToken DestroyCancellationToken { get; }

        /// <summary>在time秒后调用指定方法</summary>
        public void Invoke(string methodName, float time) { }

        /// <summary>延迟后开始重复调用</summary>
        public void InvokeRepeating(string methodName, float time, float repeatRate) { }

        /// <summary>取消所有Invoke调用</summary>
        public void CancelInvoke() { }

        /// <summary>取消指定方法的Invoke调用</summary>
        public void CancelInvoke(string methodName) { }

        /// <summary>检查指定方法是否有待执行的Invoke</summary>
        public bool IsInvoking(string methodName) { return default; }

        /// <summary>检查是否有任何待执行的Invoke</summary>
        public bool IsInvoking() { return default; }

        /// <summary>脚本实例加载时调用</summary>
        private void Awake() { }

        /// <summary>对象变为激活状态时调用</summary>
        private void OnEnable() { }

        /// <summary>首次Update之前调用</summary>
        private void Start() { }

        /// <summary>每帧调用</summary>
        private void Update() { }

        /// <summary>固定时间步长调用（物理相关）</summary>
        private void FixedUpdate() { }

        /// <summary>所有Update之后调用</summary>
        private void LateUpdate() { }

        /// <summary>对象变为非激活状态时调用</summary>
        private void OnDisable() { }

        /// <summary>对象被销毁时调用</summary>
        private void OnDestroy() { }

        /// <summary>渲染GUI时调用</summary>
        private void OnGUI() { }

        /// <summary>IK回调</summary>
        private void OnAnimatorIK(int layerIndex) { }

        /// <summary>动画移动回调</summary>
        private void OnAnimatorMove() { }

        /// <summary>编辑器专用，重置默认值</summary>
        private void Reset() { }

        /// <summary>编辑器专用，值变化时调用</summary>
        private void OnValidate() { }


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
