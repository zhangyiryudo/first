#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <iomanip>

// 辅助函数：打印当前时间（含毫秒）
void printTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::cout << std::put_time(std::localtime(&time_t_now), "%H:%M:%S") 
              << "." << std::setfill('0') << std::setw(3) << ms.count();
}

// C++ 实现原理解析
// std::condition_variable 的阻塞：
// 在 Take() 方法中，cv_take_.wait() 会检查 current_tokens_ > 0。
// 如果条件不满足，该线程会立即释放锁并进入休眠状态。实现了题目的“超出部分自动排队”。

// notify_all() 实现最快速度处理：
// 在 tick() 后台线程中，每秒钟会将令牌补满（current_tokens_ = limit_），
// 随后调用 cv_take_.notify_all()。这会瞬间唤醒所有休眠的线程。
// 操作系统底层的调度器会立刻分配 CPU 时间片给这些线程，让它们去抢占锁和令牌，
// 实现了跨窗口时“以最快速度完成申报”。

// 安全的中断睡眠：
// 这里使用了 cv_stop_.wait_for。当程序准备退出调用 Stop() 时，可以通过 notify_all 立即打断睡眠，
// 保证析构和退出的干净利落，不发生死锁或内存泄漏。

class WindowLimiter {
private:
    int limit_;                     // 窗口最大允许数量
    int current_tokens_;            // 当前窗口剩余令牌数
    
    std::mutex mtx_;                // 保护共享数据的互斥锁
    std::condition_variable cv_take_; // 用于阻塞和唤醒 Take() 的条件变量
    std::condition_variable cv_stop_; // 用于中断 tick 线程的条件变量
    
    bool stop_flag_;                // 停止标志位
    std::thread ticker_thread_;     // 后台定时轮转线程

    // tick 负责每秒刷新窗口状态（定时窗口轮转）
    void tick() {
        std::unique_lock<std::mutex> lock(mtx_);
        while (!stop_flag_) {
            // 等待 1 秒，或者直到收到停止信号
            // cv_stop_.wait_for 会在等待期间自动释放 lock，唤醒后重新获取 lock
            if (cv_stop_.wait_for(lock, std::chrono::seconds(1), [this]() { return stop_flag_; })) {
                // 如果 wait_for 返回 true，说明是被 stop_flag_ 唤醒的，直接退出
                break; 
            }

            // 1 秒时间到，重置当前窗口的令牌数量
            current_tokens_ = limit_;

            // 唤醒所有正在排队等待（阻塞）的 Take() 线程
            cv_take_.notify_all();
        }
    }

public:
    // 初始化并启动限流器
    WindowLimiter(int limit) : limit_(limit), current_tokens_(limit), stop_flag_(false) {
        // 启动后台定时线程
        ticker_thread_ = std::thread(&WindowLimiter::tick, this);
    }

    ~WindowLimiter() {
        Stop();
    }

    // Take 阻塞式获取执行权限（支持自动排队）
    void Take() {
        std::unique_lock<std::mutex> lock(mtx_);
        
        // 等待条件：当前令牌数 > 0。如果为 0，线程会在这里阻塞挂起，进入排队状态
        cv_take_.wait(lock, [this]() { return current_tokens_ > 0; });
        
        // 获取到执行权限，消耗一个令牌
        current_tokens_--;
    }

    // Stop 停止限流器的后台线程，清理资源
    void Stop() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (stop_flag_) return; // 防止重复停止
            stop_flag_ = true;
        }
        
        cv_stop_.notify_all(); // 立即唤醒正在 sleep 的 ticker 线程
        
        if (ticker_thread_.joinable()) {
            ticker_thread_.join(); // 等待后台线程安全退出
        }
    }
};

// ---------------- 测试代码 ----------------
// int main() {
//     // 限制每秒最多 10 笔订单
//     WindowLimiter limiter(10);
//     std::vector<std::thread> workers;
//     std::mutex cout_mtx; // 保护 std::cout 避免多线程打印混乱

//     std::cout << "["; printTime(); std::cout << "] 开始接收 15 笔突发订单...\n";

//     // 模拟在极短时间内突然涌入 15 笔订单（产生排队）
//     for (int i = 1; i <= 15; ++i) {
//         workers.emplace_back([&limiter, &cout_mtx, i]() {
//             // 调用 Take() 进行限流控制
//             limiter.Take();
            
//             // 打印通过限流的时间
//             std::lock_guard<std::mutex> lock(cout_mtx);
//             std::cout << "["; printTime(); std::cout << "] 订单 " << i << " 申报成功\n";
//         });
//     }

//     // 等待所有订单处理完毕
//     for (auto& w : workers) {
//         w.join();
//     }

//     std::cout << "所有订单申报完成。\n";
//     return 0;
// }