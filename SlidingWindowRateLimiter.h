#include <iostream>
#include <queue>
#include <mutex>

// 核心思想：
// 维护一个队列来记录每个请求的时间戳。当新请求到来时，先清理队列中超出“当前时间窗口”的旧时间戳，然后判断队列当前的长度是否超过了限流阈值。

// 优点： 能够极其精确地控制任意时间窗口内的请求数。
// 缺点： 空间复杂度较高，因为需要保存窗口内的所有请求记录。
class SlidingWindowRateLimiter {
private:
    int limit;               // 窗口内允许的最大请求数
    long long window_size;   // 窗口大小（例如：1000毫秒）
    std::queue<long long> q; // 存储请求的时间戳
    std::mutex mtx;          // 互斥锁，保证多线程安全

public:
    SlidingWindowRateLimiter(int max_requests, long long window_ms) 
        : limit(max_requests), window_size(window_ms) {}

    bool allowRequest(long long now) {
        std::lock_guard<std::mutex> lock(mtx);

        // 1. 清理过期的时间戳：只保留 [now - window_size + 1, now] 范围内的记录
        while (!q.empty() && (now - q.front() >= window_size)) {
            q.pop();
        }

        // 2. 判断当前窗口内的请求数是否超过限制
        if (q.size() < limit) {
            q.push(now); // 允许请求，记录当前时间戳
            return true;
        }

        // 超过限制，拒绝请求
        return false;
    }
};