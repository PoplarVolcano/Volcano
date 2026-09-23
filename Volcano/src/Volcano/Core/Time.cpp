#include "volpch.h"
#include "Time.h"

namespace Volcano
{
    struct TimeData
    {
        std::chrono::steady_clock::time_point StartTime;
        float LastFrameTime = 0.0f;
        float DeltaTime = 0.0f;
        float TimeScale = 1.0f;
    };
    static TimeData* s_TimeData = nullptr;

    void Time::Init()
    {
        s_TimeData = new TimeData();
        s_TimeData->StartTime = std::chrono::steady_clock::now();
    }

    const std::chrono::steady_clock::time_point& Time::GetStartTime()
    {
        return s_TimeData->StartTime;
    }

    float Time::GetLastFrameTime()
    {
        return s_TimeData->LastFrameTime;
    }

    void Time::SetLastFrameTime(float newTime)
    {
        s_TimeData->LastFrameTime = newTime;
        s_TimeData->DeltaTime = GetSeconds() - s_TimeData->LastFrameTime;
        s_TimeData->DeltaTime = s_TimeData->DeltaTime > 0.1f ? 0.1f : s_TimeData->DeltaTime;// 防止 timestep 过大（比如调试断点导致的跳帧）
    }

    float Time::GetDeltaTime()
    {
        return s_TimeData->DeltaTime;
    }

    void Time::SetDeltaTime(float newDeltaTime)
    {
        s_TimeData->DeltaTime = newDeltaTime;
    }

    float Time::GetTimeScale()
    {
        return s_TimeData->TimeScale;
    }

    void Time::SetTimeScale(float newTimesScale)
    {
        s_TimeData->TimeScale = newTimesScale;
    }

    float Time::GetSeconds()
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = now - s_TimeData->StartTime;
        return std::chrono::duration<float>(duration).count();
    }

    double Time::GetSecondsDouble()
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = now - s_TimeData->StartTime;
        return std::chrono::duration<double>(duration).count();
    }

    long long Time::GetMilliseconds()
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = now - s_TimeData->StartTime;
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }

    long long Time::GetMicroseconds()
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = now - s_TimeData->StartTime;
        return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    }

    void Time::Reset()
    {
        s_TimeData->StartTime = std::chrono::steady_clock::now();
    }
}