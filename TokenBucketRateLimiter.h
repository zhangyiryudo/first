#include <iostream>
#include <algorithm>
#include <mutex>
// 2. 令牌桶 (Token Bucket) —— 面试最常考
// 核心思想：
// 系统以恒定的速率生成令牌放入桶中，桶有一个最大容量。当请求到来时，
// 必须从桶中消耗一个令牌才能被处理。如果桶空了，请求就会被拒绝。

// 实现技巧（Lazy Refill 延迟填充）：
// 我们不需要真的开一个后台线程去定时往桶里放令牌（这样太浪费资源且容易不精确）。
// 我们只需要在每次请求到来时，计算距离上次请求过去了多少时间，
// 动态计算出这段时间应该生成多少个令牌，加到桶里即可。
class TokenBucketRateLimiter {
private:
    double capacity;         // 桶的最大容量（应对突发流量的上限）
    double tokens;           // 当前桶里的令牌数
    double refill_rate;      // 补充速率（每毫秒补充多少个令牌）
    long long last_time;     // 上次补充令牌的时间戳
    std::mutex mtx;          // 互斥锁

public:
    // capacity: 桶的容量 (Burst capacity)
    // tokens_per_second: 每秒允许通过的请求数
    TokenBucketRateLimiter(double capacity, double tokens_per_second) {
        this->capacity = capacity;
        this->tokens = capacity; // 初始状态下桶是满的
        this->refill_rate = tokens_per_second / 1000.0; // 转换为每毫秒的生成速率
        this->last_time = 0;
    }

    bool allowRequest(long long now) {
        std::lock_guard<std::mutex> lock(mtx);

        // 初始化第一次请求的时间
        if (last_time == 0) {
            last_time = now;
        }

        // 1. 延迟填充：计算距离上次请求流逝的时间，并转化为生成的令牌数
        long long elapsed_time = now - last_time;
        double generated_tokens = elapsed_time * refill_rate;

        // 2. 更新当前令牌数（不能超过桶的最大容量）
        tokens = std::min(capacity, tokens + generated_tokens);
        last_time = now; // 更新时间戳

        // 3. 判断是否还有令牌可以消耗
        if (tokens >= 1.0) {
            tokens -= 1.0; // 消耗一个令牌
            return true;
        }

        // 令牌不足，拒绝请求
        return false;
    }
};