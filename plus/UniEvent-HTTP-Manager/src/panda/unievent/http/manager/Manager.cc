#include "Manager.h"
#include <ctime>
#include <atomic>

#include "Thread.h"
#ifndef _WIN32
    #include "PreFork.h"
#endif

namespace panda { namespace unievent { namespace http { namespace manager {

log::Module panda_log_module("UniEvent::HTTP::Manager");

Manager::Manager (const Config& config, const LoopSP& loop) {
    switch (config.worker_model) {
        case WorkerModel::Thread  : mpm = new Thread(config, loop); break;
        #ifndef _WIN32
        case WorkerModel::PreFork : mpm = new PreFork(config, loop); break;
        #endif
        default : throw exception("selected worker model is not supported on current OS");
    }
}

const LoopSP& Manager::loop () const {
    return mpm->get_loop();
}

void Manager::run () {
    mpm->server_factory = server_factory;
    mpm->spawn_event    = spawn_event;
    mpm->request_event  = request_event;
    mpm->run();
}

void Manager::stop () {
    mpm->stop();
}

Manager::~Manager () {
    delete mpm;
}

}}}}
