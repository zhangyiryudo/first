#include "WindowLimiter.h"

int main() {
    // 限制每秒最多 10 笔订单
    WindowLimiter limiter(10);
    std::vector<std::thread> workers;
    std::mutex cout_mtx; // 保护 std::cout 避免多线程打印混乱

    std::cout << "["; printTime(); std::cout << "] 开始接收 15 笔突发订单...\n";

    // 模拟在极短时间内突然涌入 15 笔订单（产生排队）
    for (int i = 1; i <= 15; ++i) {
        workers.emplace_back([&limiter, &cout_mtx, i]() {
            // 调用 Take() 进行限流控制
            limiter.Take();
            
            // 打印通过限流的时间
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cout << "["; printTime(); std::cout << "] 订单 " << i << " 申报成功\n";
        });
    }

    // 等待所有订单处理完毕
    for (auto& w : workers) {
        w.join();
    }

    std::cout << "所有订单申报完成。\n";
    return 0;
}