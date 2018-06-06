#include "concurrent.h"

static void* ThreadRun(void* ptr) {
    paxos::Thread* thread = (paxos::Thread*)ptr;
    thread->Run();
    return 0;
}

namespace paxos {

Thread::Thread() {}

Thread::~Thread() {}

void Thread::Detach() {
    thread_.detach();
}

std::thread::id Thread::GetId() const {
    return thread_.get_id();
}

void Thread::Join() {
    thread_.join();
}

void Thread::Sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void Thread::Start() {
    thread_ = std::thread(std::bind(&ThreadRun, this));
}

}
