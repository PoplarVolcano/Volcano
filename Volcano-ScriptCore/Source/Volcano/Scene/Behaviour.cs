using System;

namespace Volcano
{
    // Behaviours are Components that can be enabled or disabled.
    public class Behaviour : Component
    {
        // Enabled Behaviours are Updated, disabled Behaviours are not. 启用的Behaviour会更新，禁用的Behaviour不会更新。
        // GetFixedBehaviourManager is directly used by fixed update in the player loop. GetFixedBehaviourManager直接用于播放器循环中的固定更新
        extern public bool enabled { get; set; }

        // 是否活动并启动
        extern public bool isActiveAndEnabled
        {
            get;
        }

    }
}
