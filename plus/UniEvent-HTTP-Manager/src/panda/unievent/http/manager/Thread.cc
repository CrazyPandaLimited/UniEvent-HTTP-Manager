#include "Thread.h"
#include <thread>

namespace panda { namespace unievent { namespace http { namespace manager {

struct ThreadWorker : Worker {
    std::thread::id tid;
};

Thread::Thread (const Config& _c, const LoopSP& _l) : Mpm(_c, _l) {
    if (loop != Loop::default_loop()) throw exception("you must use default loop for thread worker model");
}

void Thread::run () {
    Mpm::run();
    throw "nah";
}

void Thread::fetch_state () {
    throw "nah";
}

WorkerPtr Thread::create_worker () {
    throw "nah";
}

}}}}
