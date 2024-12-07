#pragma once

// �߳�
#include <thread>
#include <condition_variable>
#include <mutex>

#include <vector>
#include <queue>
#include <memory>
#include <Volcano/Core/Base.h>

namespace Volcano {

    const int MAX_THREADS = 10000; //����߳���Ŀ
    template <typename T>
    class ThreadPool
    {
    public:
        // ���캯���������̣߳�Ĭ�Ͽ�һ���߳�
        ThreadPool(int number = 1);
        ~ThreadPool();
        std::queue<Ref<T>> tasks_queue; //�������
        // ���������tasks_queue���������T
        bool append(Ref<T> request);
    private:
        // �����̵߳��������еĺ���
        static void* worker(void* arg);
        void run();
    private:
        std::vector<std::thread> work_threads; //�����߳�

        std::mutex queue_mutex;
        std::condition_variable condition;  //������unique_lock���ʹ��
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
            // ע���߳�
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
        condition.notify_all(); // ���������߳�
        for (auto& work : work_threads)
            work.join();//������������join
    }

    template <typename T>
    inline bool ThreadPool<T>::append(Ref<T> request)
    {
        //�����������ʱһ��Ҫ��������Ϊ���������̹߳���
        queue_mutex.lock();
        tasks_queue.push(request);
        queue_mutex.unlock();
        condition.notify_one();  //�̳߳����������Ҫ֪ͨ�ȴ����߳�
        return true;
    }

    template <typename T>
    inline void* ThreadPool<T>::worker(void* arg)
    {
        ThreadPool* pool = (ThreadPool*)arg;
        pool->run();//�߳�����
        return pool;
    }

    template <typename T>
    inline void ThreadPool<T>::run()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex); // unique_lock() ����������Զ�����
            // �������Ϊ�գ���wait����ͣ�����ȴ����ѡ���Ҫ�����񣬲��������̣߳���Ȼ������
            this->condition.wait(lock, [this] { return !this->tasks_queue.empty() || stop; });
            if (stop) return;
            // ����Ϊ�գ�˫�ر���
            if (this->tasks_queue.empty()) { VOL_ASSERT(false, "ThreadPool::Run"); continue; }

            Ref<T> request = tasks_queue.front();
            tasks_queue.pop();
            lock.unlock();

            if (request)//�������ˣ���ʼִ��
                request->process();
        }
    }
}