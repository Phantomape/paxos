#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace paxos {

class Thread {
public:
    Thread();
    Thread(const Thread&) = delete;
    Thread& operator = (const Thread&) = delete;
    virtual ~Thread();

    void Detach();
    std::thread::id GetId() const;
    void Join();
    virtual void Run() = 0;
    static void Sleep(int ms);
    void Start();
protected:
    std::thread thread_;
};

template <class T>
class Queue {
public:
    Queue();
    virtual ~Queue();

    virtual size_t add(const T& t, bool signal = true, bool back = true);
    void broadcast();
    void clear();
    bool empty();
    T& peek();
    bool peek(T& t, int timeout_ms);
    size_t pop();
    size_t pop(T* vals, size_t n);
    virtual void lock();
    virtual void unlock();
    void signal();
    size_t size() const;
protected:
    std::mutex lock_;
    std::condition_variable cond_;
    std::deque<T> storage_;
    size_t size_;
};

}