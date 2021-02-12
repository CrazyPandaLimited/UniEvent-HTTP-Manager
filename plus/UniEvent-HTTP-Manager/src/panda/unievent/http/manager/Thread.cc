#include "Thread.h"
#include <mutex>
#include <future>
#include <thread>
#include <panda/unievent/Async.h>

namespace panda { namespace unievent { namespace http { namespace manager {

static std::mutex mutex;

struct ThreadWorker : Worker {
    std::thread thread;

    struct SharedData {
        AsyncSP                control_handle;
        AsyncSP                termination_handle;
        std::atomic<uint32_t>  active_requests;
        std::atomic<time_t>    activity_time;
        std::atomic<float>     load_average;
        std::atomic<uint32_t>  total_requests;
        std::atomic<bool>      terminate;
        std::atomic<bool>      die;
    } shared;

    ThreadWorker () {
        shared.active_requests = 0;
        shared.activity_time   = 0;
        shared.load_average    = 0;
        shared.total_requests  = 0;
        shared.terminate       = false;
        shared.die             = false;
    }

    void fetch_state () override {
        active_requests = shared.active_requests;
        load_average    = shared.load_average;
        activity_time   = shared.activity_time;
        total_requests  = shared.total_requests;
    }

    void terminate () override {
        panda_log_info("master thread: terminate worker thread=" << thread.get_id());
        shared.terminate = true;
        shared.control_handle->send();
    }

    void kill () override {
        panda_log_info("master thread: killing worker thread=" << thread.get_id());
        shared.die = true;
        shared.control_handle->send();
    }
};

struct ThreadChild : Child {
    ThreadWorker::SharedData& shared;

    ThreadChild (ThreadWorker::SharedData& shared) : shared(shared) {}

    void run () override {
        Child::run();
        panda_log_info("worker thread: finishing");

        // protect iptrs in functions
        std::lock_guard<std::mutex> lock(mutex);
        server->request_event.remove_all();
    }

    void send_active_requests (uint32_t areqs) override {
        shared.active_requests = areqs;
    }

    void send_activity (time_t now, float la, uint32_t total_requests) override {
        shared.load_average   = la;
        shared.activity_time  = now;
        shared.total_requests = total_requests;
    }
};

Thread::Thread (const Config& _c, const LoopSP& _l) : Mpm(_c, _l) {
    if (loop != Loop::default_loop()) throw exception("you must use default loop for thread worker model");
}

void Thread::run () {
    Mpm::run();
}

WorkerPtr Thread::create_worker () {
    std::lock_guard<std::mutex> lock(mutex); // sync with thread dtors

    std::promise<bool> init_promise;

    auto worker = std::make_unique<ThreadWorker>();
    worker->shared.termination_handle = new Async(loop);
    worker->shared.termination_handle->event.add([this, worker = worker.get()](auto&) {
        panda_log_info("master: worker tid=" << worker->thread.get_id() << " terminated");
        worker->thread.join();
        worker_terminated(worker);
    });

    worker->thread = std::thread([this, &shared = worker->shared, &init_promise] {
        auto loop = Loop::default_loop(); // this loop is thread-local, DO NOT use this->loop !

        ThreadChild child(shared);

        shared.control_handle = new Async(loop);
        shared.control_handle->weak(true);
        shared.control_handle->event.add([&shared, &child, &loop](auto&) {
            if (shared.die) loop->stop();
            else if (shared.terminate) child.terminate();
        });

        auto config = this->config; // copy
        if (config.bind_model == Manager::BindModel::Duplicate) { // we need to dup sockets
            for (auto& loc : config.server.locations) loc.sock = sock_dup(loc.sock.value());
        }

        child.init({loop, config, server_factory, spawn_event, request_event});
        init_promise.set_value(true);

        child.run();
        shared.termination_handle->send();
    });

    // wait until thread initializes to allow running thread-unsafe code in worker initialization callbacks
    init_promise.get_future().wait();

    return WorkerPtr(worker.release());
}

void Thread::stop () {
    Mpm::stop();
}

void Thread::stopped () {
    Mpm::stopped();
}

}}}}
