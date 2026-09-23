#pragma once
#include <chrono>

#include "Volcano/Core/Core.h"

namespace Volcano
{
    class VOL_API Time
    {
    public:
        // 初始化计时器（在程序启动时调用）
        static void Init();

        // 获取开始时间
        static const std::chrono::steady_clock::time_point& GetStartTime();

        // 获取上一帧时间
        static float GetLastFrameTime();

        // 修改上一帧时间
        static void SetLastFrameTime(float newTime);

        // 获取时间间隔
        static float GetDeltaTime();

        // 设置时间间隔
        static void SetDeltaTime(float newDeltaTime);

        // 获取时间流速的缩放比例
        static float GetTimeScale();

        // 修改时间流速的缩放比例
        static void SetTimeScale(float newTimesScale);

        // 获取从 Init() 开始经过的时间，秒float
        static float GetSeconds();

        // 获取从 Init() 开始经过的时间，高精度秒double
        static double GetSecondsDouble();

        // 获取从 Init() 开始经过的时间，毫秒
        static long long GetMilliseconds();

        // 获取从 Init() 开始经过的时间，微秒
        static long long GetMicroseconds();

        // 重置计时器
        static void Reset();
    };
}