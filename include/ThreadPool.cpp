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