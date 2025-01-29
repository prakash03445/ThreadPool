#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <future>

class ThreadPool {
    public:
        ThreadPool(size_t);
        ~ThreadPool();

        template<class F, class... Args>
        auto enqueue (F&& f, Args&&... args) 
            -> std::future<typename std::result_of<F(Args...)>::type>;

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;

        std::mutex queue_mutex;
        std::condition_variable cv;
        bool stop;
};

#endif //THREAD_POOL_H