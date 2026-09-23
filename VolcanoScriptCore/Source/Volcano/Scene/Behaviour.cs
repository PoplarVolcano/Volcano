using System;

namespace Volcano
{
    // 可启用/禁用的组件基类
    public class Behaviour : Volcano.Component
    {
        /// <summary>启用的Behaviour可更新，禁用的不可更新</summary>
        public bool Enabled { get; set; }

        /// <summary>Behaviour是否已激活并启用（只读）</summary>
        public bool IsActiveAndEnabled { get; }
    }
}
