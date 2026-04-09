#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

template<typename T>
struct Node {
    T data;
    std::atomic<Node*> next;
    Node(T val) : data(val), next(nullptr) {}
};

template<typename T>
class LockFreeQueue {
private:
    std::atomic<Node<T>*> head;
    std::atomic<Node<T>*> tail;

public:
    LockFreeQueue() {
        Node<T>* dummy = new Node<T>(T{});
        head.store(dummy);
        tail.store(dummy);
    }

    ~LockFreeQueue() {
        while (Node<T>* node = head.load()) {
            head.store(node->next.load());
            delete node;
        }
    }

    void enqueue(T val) {
        Node<T>* newNode = new Node<T>(val);
        Node<T>* oldTail;
        while (true) {//循环尝试更新尾部：
            oldTail = tail.load(std::memory_order_acquire);
            Node<T>* next = oldTail->next.load(std::memory_order_acquire);
            if (oldTail == tail.load(std::memory_order_acquire)) {//检查尾部是否被其他线程修改：如果 tail 未变：
                if (next == nullptr) {//如果 next 为 nullptr（tail 是真正的尾部），尝试将 newNode 设置为 tail 的 next。
                    if (oldTail->next.compare_exchange_weak(next, newNode, std::memory_order_release, std::memory_order_relaxed)) {
                        tail.compare_exchange_weak(oldTail, newNode, std::memory_order_release, std::memory_order_relaxed);
                        return;
                    }
                } else {//如果 next 不为 nullptr，说明 tail 已经落后了，尝试将 tail 更新到 next。
                    tail.compare_exchange_weak(oldTail, next, std::memory_order_release, std::memory_order_relaxed);
                }
            }
        }
    }

    bool dequeue(T& result) {
        while (true) {
            Node<T>* oldHead = head.load(std::memory_order_acquire);
            Node<T>* oldTail = tail.load(std::memory_order_acquire);
            Node<T>* next = oldHead->next.load(std::memory_order_acquire);
            if (oldHead == head.load(std::memory_order_acquire)) {
                if (oldHead == oldTail) {
                    if (next == nullptr) {
                        return false; // empty
                    }
                    tail.compare_exchange_weak(oldTail, next, std::memory_order_release, std::memory_order_relaxed);
                } else {
                    result = next->data;
                    if (head.compare_exchange_weak(oldHead, next, std::memory_order_release, std::memory_order_relaxed)) {
                        delete oldHead;
                        return true;
                    }
                }
            }
        }
    }
};

int main() {
    LockFreeQueue<int> queue;

    // Simple test
    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);

    int val;
    while (queue.dequeue(val)) 
    {
        std::cout << (val) << std::endl;
    }

    return 0;
}