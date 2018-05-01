#pragma once

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

}