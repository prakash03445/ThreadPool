// ThreadPool class adapted from the following repository:
// Repository: https://github.com/johndoe/awesome-threadpool

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

template<class F, class... Args>
auto ThreadPool::enqueue (F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using result_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<result_type()>>(
            std::bind( std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<result_type> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace([task]() {(*task)(); });
        }

        cv.notify_one();
        return result;
    }


#endif //THREAD_POOL_H