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
    Queue() : size_(0) {};
    virtual ~Queue() {};

    virtual size_t add(const T& val, bool signal = true, bool append_to_back = true) {
        if (append_to_back) {
            storage_.push_back(val);
        }
        else {
            storage_.push_front(val);
        }

        if (signal) {
            cond_.notify_one();
        }

        return ++size_;
    };

    void broadcast() {
        cond_.notify_all();
    };

    void clear() {
        storage_.clear();
    };

    bool empty() {
        return storage_.empty();
    };

    T& peek() {
        while (empty()) {
            cond_.wait(lock_);
        }
        return storage_.front();
    };

    bool peek(T& val, int timeout_ms) {
         while (empty()) {
            if (cond_.wait_for(lock_, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout) {
                return false;
            }
        }
        val = storage_.front();
        return true;
    };

    size_t peek(T& val) {
         while (empty()) {
            cond_.wait(lock_);
        }
        val = storage_.front();
        return size_;
    }

    size_t pop() {
        storage_.pop_front();
        return --size_;
    };

    size_t pop(T* vals, size_t n) {
        while (empty()) {
            cond_.wait(lock_);
        }

        size_t i = 0;
        while (!storage_.empty() && i < n) {
            vals[i] = storage_.front();
            storage_.pop_front();
            --size_;
            ++i;
        }

        return i;
    };

    virtual void lock() {
        lock_.lock();
    };

    virtual void unlock() {
        lock_.unlock();
    };

    void signal() {
        cond_.notify_one();
    };

    size_t size() const {
        return storage_.size();
    };

    void swap(Queue& q) {
        storage_.swap(q.storage_);
        int size = q.size;
        q.size_ = size_;
        size_ = size;
    }

protected:
    std::mutex lock_;
    std::condition_variable_any cond_;
    std::deque<T> storage_;
    size_t size_;
};

}