#include "ThreadPool.h"
#include <iostream>

int main() {
    ThreadPool pool(4);

    std::vector<std::future<int>> results;

    for (int i = 0; i < 20; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                return i *  10;
            })
        );
    }

    for (auto& result: results) {
        std::cout << result.get() << std::endl;
    }

    return 0;
}