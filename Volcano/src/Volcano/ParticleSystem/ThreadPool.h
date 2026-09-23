#pragma once

#include "Volcano/Core/Core.h"

namespace Volcano
{
    /*
    std::lock_guard
    构造即锁，析构即解，不能中途解锁。
    零额外开销，与手动加锁一样快。
    不可以配合条件变量
    适用于简单的、生命周期固定的临界区保护

    std::unique_lock：
    可随时 unlock()，也可随时重新 lock()。
    有轻微额外开销（维护锁状态标志）
    必须配合条件变量（需要传入wait(lock, predicate)）
    适用于需要提前解锁（如你代码中执行 process 前需释放锁）、或需要配合条件变量。

    RAII（Resource Acquisition Is Initialization，资源获取即初始化）
    RAII锁：
    将“加锁”操作交给构造函数，将“解锁”操作交给析构函数
    RAII锁的黄金法则：凡是涉及多线程互斥量，绝对禁止手动调用 .lock() 和 .unlock()。请将所有互斥量包裹在RAII锁对象中。
    
    示例：
    {
        std::lock_guard<std::mutex> lock(queue_mutex); // 构造函数中调用 lock()
        tasks_queue.push(request); 
        // 此处无需手动 unlock
    } // 离开作用域，lock对象的析构函数被自动调用，内部执行 unlock()

    RAII不仅应用于锁，还广泛应用于：
    动态内存（std::shared_ptr / unique_ptr 自动释放堆内存）
    文件句柄（std::fstream 析构时自动 close）
    数据库连接（析构时自动断开）

    */
    template <typename T>
    class ThreadPool
    {
    public:
        ThreadPool(uint32_t number = 1);
        ~ThreadPool();
        std::queue<Ref<T>> m_TasksQueue; //任务队列
        bool Append(Ref<T> request);    // 往任务队列m_TasksQueue中添加任务T
    private:
        void Run();
    private:
        std::vector<std::thread> m_WorkThreads; // 工作线程容器，线程数量在构造时固定

        std::mutex m_QueueMutex;              // 互斥锁，保护任务队列的读写操作
        std::condition_variable m_Condition;  // 条件变量，用于线程间同步（唤醒等待任务的线程），必须与unique_lock配合使用
        bool m_Stop;                          // 生命周期控制标志，析构时置为 true，通知所有线程退出
    };

    template <typename T>
    inline ThreadPool<T>::ThreadPool(uint32_t number) : m_Stop(false)
    {
        // 如果该值“无法计算”或“未明确定义”，hardware_concurrency() 返回 0（常见于嵌入式系统或某些虚拟化环境）
        uint32_t max_threads = std::thread::hardware_concurrency();
        if (max_threads == 0)
            max_threads = 4;

        if (number > max_threads)
        {
            VOL_ASSERT(false, "ThreadPool's size is too large!")
            throw std::exception();
        }

        // 注册number个线程执行Run，注册后该线程立刻开始运行
        for (uint32_t i = 0; i != number; i++)
        {
            m_WorkThreads.emplace_back(&ThreadPool::Run(), this);
        }
    }

    template <typename T>
    inline ThreadPool<T>::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Stop = true;
            while (!m_TasksQueue.empty())
                m_TasksQueue.pop();
        }

        // 解锁所有线程
        m_Condition.notify_all();

        // 挨个调用 join() 等待线程完全退出，保证资源安全释放
        for (auto& work : m_WorkThreads)
            work.join();
    }

    template <typename T>
    inline bool ThreadPool<T>::Append(Ref<T> request)
    {
        // 手动锁，若 push 抛出异常（如 bad_alloc），锁永远不会释放
        // queue_mutex.lock();
        // tasks_queue.push(request);
        // queue_mutex.unlock();

        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_TasksQueue.push(request);
        }
        m_Condition.notify_one();  //线程池添加了任务，解锁一个等待的线程
        return true;
    }

    template <typename T>
    inline void ThreadPool<T>::Run()
    {
        while (true)
        {
            // unique_lock() 出作用域会自动解锁
            std::unique_lock<std::mutex> lock(this->m_QueueMutex);

            // 等待条件：任务队列非空 或 收到停止信号
            this->m_Condition.wait(lock, [this] { return !this->m_TasksQueue.empty() || m_Stop; });

            // 停止信号，退出线程
            if (m_Stop)
                return;
            // 双重检查队列（防止虚假唤醒）
            if (this->m_TasksQueue.empty())
                continue;

            // 取出队首任务，并立即出队
            Ref<T> request = m_TasksQueue.front();
            m_TasksQueue.pop();

            // 处理任务前释放锁，避免长时间占用互斥量
            lock.unlock();

            // 执行任务（此时队列已解锁，其他线程可并行取任务）
            if (request)
                request->process();
        }
    }
}