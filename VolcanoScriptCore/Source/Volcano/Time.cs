
using System;
using System.Runtime.CompilerServices;

namespace Volcano
{
    public class Time
    {
        /// <summary>自游戏启动以来经过的时间（秒），受 Time.timeScale 影响（只读）</summary>
        /// <remarks>在 FixedUpdate 中调用时，返回 Time.fixedTime</remarks>
        public static float TimeAsFloat
        {
            get { Time_GetTimeAsFloat(out float result); return result * TimeScale; }
        }

        /// <summary>上一帧到当前帧的时间间隔（秒），受 Time.timeScale 影响（只读）</summary>
        /// <remarks>用于将速度从“每帧”转换为“每秒”</remarks>
        public static float DeltaTime
        {
            get { Time_GetDeltaTime(out float result); return result * TimeScale; }
        }

        /// <summary>游戏时间流速的缩放比例，默认值为 1.0</summary>
        /// <remarks>设为 0 可暂停游戏，设为 2 可让游戏加速一倍</remarks>
        public static float TimeScale
        {
            get { Time_GetTimeScale(out float result); return result; }
            set { Time_SetTimeScale(value); }
        }

        /// <summary>自游戏启动以来经过的“未缩放”时间（秒），不受 Time.timeScale 影响（只读）</summary>
        public static float UnscaledTime
        {
            get { Time_GetTimeAsFloat(out float result); return result; }
        }

        /// <summary>自上一帧完成以来经过的“未缩放”时间（秒），不受 Time.timeScale 影响（只读）</summary>
        public static float UnscaledDeltaTime
        {
            get { Time_GetDeltaTime(out float result); return result; }
        }

        /// <summary>自应用程序启动以来的实际时间（秒），不受 Time.timeScale 影响，不受暂停影响（只读）</summary>
        /// <remarks>适用于需要测量真实时间的场景，如编辑器脚本或暂停菜单计时</remarks>
        public static float RealtimeSinceStartup { get; }

        /// <summary>自当前场景加载以来经过的时间（秒），受 Time.timeScale 影响（只读）</summary>
        public static float TimeSinceLevelLoad { get; }

        /// <summary>自游戏启动以来经过的帧总数（只读）</summary>
        public static int FrameCount { get; }

        /// <summary>平滑后的 deltaTime 值，减少了帧与帧之间的剧烈波动（只读）</summary>
        /// <remarks>可用于需要更平滑动画效果的场景</remarks>
        public static float SmoothDeltaTime { get; }

        /// <summary>当前帧的 deltaTime 上限值，防止单帧耗时过长导致物理跳跃（只读）</summary>
        public static float MaximumDeltaTime { get; set; }

        /// <summary>当前帧的 deltaTime 下限值，防止单帧耗时过短导致物理异常（只读）</summary>
        public static float MinimumDeltaTime { get; set; }

        /// <summary>当前帧开始时的总时间（秒），从上次场景加载开始计算（只读）</summary>
        public static float UnscaledTimeSinceLevelLoad { get; }


        // ======================== 固定时间步长相关 ========================

        /// <summary>固定时间步长的时间间隔（秒），用于物理和 FixedUpdate 更新</summary>
        /// <remarks>默认值为 0.02 秒（即 50 FPS 的物理更新速率）</remarks>
        public static float FixedDeltaTime { get; set; }

        /// <summary>自上次 FixedUpdate 开始以来的时间（秒），受 Time.timeScale 影响（只读）</summary>
        public static float FixedTime { get; }

        /// <summary>自上次 FixedUpdate 开始以来的“未缩放”时间（秒），不受 Time.timeScale 影响（只读）</summary>
        public static float FixedUnscaledTime { get; }

        /// <summary>自上次 FixedUpdate 阶段到当前阶段的“未缩放”时间间隔（秒），不受 Time.timeScale 影响（只读）</summary>
        public static float FixedUnscaledDeltaTime { get; }

        /// <summary>当前是否在 FixedUpdate 回调（固定时间步长）中被调用（只读）</summary>
        public static bool InFixedTimeStep { get; }


        // ======================== 高精度（Double）版本 ========================

        /// <summary>自游戏启动以来经过的时间（秒），双精度版本，受 Time.timeScale 影响（只读）</summary>
        public static double TimeAsDouble { get; }

        /// <summary>自游戏启动以来经过的“未缩放”时间（秒），双精度版本，不受 Time.timeScale 影响（只读）</summary>
        public static double UnscaledTimeAsDouble { get; }

        /// <summary>自上次 FixedUpdate 开始以来的时间（秒），双精度版本，受 Time.timeScale 影响（只读）</summary>
        public static double FixedTimeAsDouble { get; }

        /// <summary>自上次 FixedUpdate 开始以来的“未缩放”时间（秒），双精度版本，不受 Time.timeScale 影响（只读）</summary>
        public static double FixedUnscaledTimeAsDouble { get; }


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Time_GetTimeAsFloat(out float result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Time_GetDeltaTime(out float result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Time_GetTimeScale(out float result);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Time_SetTimeScale(float value);

    }
}
