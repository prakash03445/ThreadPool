#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threads) {
    for (int i= 0; i<threads; ++i) {
        workers.emplace_back(
            [this] 
            {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->cv.wait(lock,
                                      [this] { 
                                        return (this->stop || !this->tasks.empty());
                                        });

                        if (this->stop && this->tasks.empty())
                            return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            }
        );
    }
}

template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using result_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<result_type>>(
        std::bind( std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<result_type> result = task.get_future();

    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace([task] {(*task)(); });
    }

    cv.notify_one();
    return result;
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    
    cv.notify_all();
    for (std::thread& worker: workers) {
        worker.join();
    }
}