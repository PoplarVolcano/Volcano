#pragma once

namespace Volcano
{
    template<typename Fn>
    class EditorTimer {
    public:
        EditorTimer(const char* name, Fn&& func)
            :m_Name(name), m_Func(func), m_Stopped(false)
        {
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }
        ~EditorTimer() {
            if (!m_Stopped) {
                Stop();
            }
        }
        void Stop() {
            //销毁对象时，如果计时器尚未停止 ，它会记录结束时间点
            auto endTimepoint = std::chrono::high_resolution_clock::now();
            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
            m_Stopped = true;
            float duration = (end - start) * 0.001f;// 单位ms
            m_Func({ m_Name, duration });
            //std::cout << "Timer:"<< m_Name << "时差：" << duration << "ms" << std::endl;
        }
    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
        bool m_Stopped;
        Fn m_Func;
    };

#define PROFILE_SCOPE(name) EditorTimer timer##__LINE__(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); })
}