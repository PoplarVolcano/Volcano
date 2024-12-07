#pragma once

// 线程
#include <thread>
#include <condition_variable>
#include <mutex>

#include <vector>
#include <queue>
#include <memory>
#include <Volcano/Core/Base.h>

namespace Volcano {

    const int MAX_THREADS = 10000; //最大线程数目
    template <typename T>
    class ThreadPool
    {
    public:
        // 构造函数，创建线程，默认开一个线程
        ThreadPool(int number = 1);
        ~ThreadPool();
        std::queue<Ref<T>> tasks_queue; //任务队列
        // 往任务队列tasks_queue中添加任务T
        bool append(Ref<T> request);
    private:
        // 工作线程的任务运行的函数
        static void* worker(void* arg);
        void run();
    private:
        std::vector<std::thread> work_threads; //工作线程

        std::mutex queue_mutex;
        std::condition_variable condition;  //必须与unique_lock配合使用
        bool stop;
    };



    template <typename T>
    inline ThreadPool<T>::ThreadPool(int number) : stop(false)
    {
        if (number <= 0 || number > MAX_THREADS)
        {
            VOL_ASSERT(false, "ThreadPool's size is too large!")
                //throw std::exception();
        }
        for (int i = 0; i < number; i++)
        {
            // 注册线程
            work_threads.emplace_back(worker, this);
        }
    }

    template <typename T>
    inline ThreadPool<T>::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
            while (!tasks_queue.empty())
                tasks_queue.pop();
        }
        condition.notify_all(); // 解锁所有线程
        for (auto& work : work_threads)
            work.join();//在析构函数中join
    }

    template <typename T>
    inline bool ThreadPool<T>::append(Ref<T> request)
    {
        //操作任务队列时一定要加锁，因为他被所有线程共享
        queue_mutex.lock();
        tasks_queue.push(request);
        queue_mutex.unlock();
        condition.notify_one();  //线程池添加了任务，要通知等待的线程
        return true;
    }

    template <typename T>
    inline void* ThreadPool<T>::worker(void* arg)
    {
        ThreadPool* pool = (ThreadPool*)arg;
        pool->run();//线程运行
        return pool;
    }

    template <typename T>
    inline void ThreadPool<T>::run()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex); // unique_lock() 出作用域会自动解锁
            // 如果任务为空，则wait，就停下来等待唤醒。需要有任务，才启动该线程，不然就休眠
            this->condition.wait(lock, [this] { return !this->tasks_queue.empty() || stop; });
            if (stop) return;
            // 任务为空，双重保障
            if (this->tasks_queue.empty()) { VOL_ASSERT(false, "ThreadPool::Run"); continue; }

            Ref<T> request = tasks_queue.front();
            tasks_queue.pop();
            lock.unlock();

            if (request)//来任务了，开始执行
                request->process();
        }
    }
}