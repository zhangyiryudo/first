#include <iostream>
#include <vector>
using namespace std;

class HitCounter {
private:
    vector<int> times; // 记录时间戳
    vector<int> hits;  // 记录点击数
    int windowSize;

public:
    HitCounter() {
        windowSize = 300; // 统计过去 300 秒
        times.resize(windowSize, 0);
        hits.resize(windowSize, 0);
    }

    // 记录一次命中
    void hit(int timestamp) {
        int idx = timestamp % windowSize;
        if (times[idx] != timestamp) {
            // 如果这个位置存的是"老时间"，说明已经过了一轮，重置它
            times[idx] = timestamp;
            hits[idx] = 1;
        } else {
            // 如果时间戳一致，累加点击
            hits[idx]++;
        }
    }

    // 获取过去 300 秒内的点击总数
    int getHits(int timestamp) {
        int total = 0;
        for (int i = 0; i < windowSize; i++) {
            // 只累加在当前时间窗口（timestamp - 300）之内的点击
            if (timestamp - times[i] < windowSize) {
                total += hits[i];
            }
        }
        return total;
    }
};

int main() {
    cout << "=== Hit Counter / Rate Limiter (Sliding Window) ===" << endl << endl;

    HitCounter counter;

    // 测试用例 1: 基础测试
    cout << "Test Case 1: Basic Usage" << endl;
    counter.hit(1);
    counter.hit(1);
    counter.hit(1);
    cout << "At timestamp 1, after 3 hits: " << counter.getHits(1) << " (expected: 3)" << endl;
    
    counter.hit(2);
    cout << "At timestamp 2, after 1 hit: " << counter.getHits(2) << " (expected: 4)" << endl;

    counter.hit(3);
    cout << "At timestamp 3, after 1 hit: " << counter.getHits(3) << " (expected: 5)" << endl;

    cout << "At timestamp 300: " << counter.getHits(300) << " (expected: 5)" << endl;
    cout << "At timestamp 301: " << counter.getHits(301) << " (expected: 4, timestamp 1 is outside window)" << endl << endl;

    // 测试用例 2: 大时间戳和环形覆盖
    cout << "Test Case 2: Large Timestamps and Circular Overlap" << endl;
    HitCounter counter2;
    
    counter2.hit(100);
    counter2.hit(100);
    counter2.hit(100);
    cout << "At timestamp 100: " << counter2.getHits(100) << " (expected: 3)" << endl;
    
    counter2.hit(400); // 100 is now outside the window (400 - 100 = 300)
    cout << "At timestamp 400: " << counter2.getHits(400) << " (expected: 1, only timestamp 400)" << endl;
    
    counter2.hit(400);
    cout << "At timestamp 400, after another hit: " << counter2.getHits(400) << " (expected: 2)" << endl << endl;

    // 测试用例 3: 贝壳状态检查
    cout << "Test Case 3: Empty Case" << endl;
    HitCounter counter3;
    cout << "At timestamp 0, no hits: " << counter3.getHits(0) << " (expected: 0)" << endl;
    cout << "At timestamp 100, no hits: " << counter3.getHits(100) << " (expected: 0)" << endl << endl;

    // 测试用例 4: 连续命中
    cout << "Test Case 4: Continuous Hits" << endl;
    HitCounter counter4;
    for (int i = 1; i <= 10; i++) {
        counter4.hit(i);
    }
    cout << "After 10 hits (timestamps 1-10): " << counter4.getHits(10) << " (expected: 10)" << endl;
    cout << "At timestamp 310: " << counter4.getHits(310) << " (expected: 0, all outside window)" << endl;

    return 0;
}