#include "Manager.h"
#include <ctime>
#include <atomic>

#include "Thread.h"
#ifndef _WIN32
    #include "PreFork.h"
#endif

namespace panda { namespace unievent { namespace http { namespace manager {

log::Module panda_log_module("UniEvent::HTTP::Manager");

Manager::Manager (const Config& config, const LoopSP& loop, Mpm* custom_mpm) {
    #ifdef _WIN32
        if (config.bind_model == BindModel::ReusePort) {
            panda_log_warning("reuse port is not supported on windows, falling back to duplicate model");
            config.bind_model = BindModel::Duplicate;
        }
    #endif

    if (custom_mpm) {
        mpm = custom_mpm;
    } else {
        switch (config.worker_model) {
            case WorkerModel::Thread  : mpm = new Thread(config, loop); break;
            #ifndef _WIN32
            case WorkerModel::PreFork : mpm = new PreFork(config, loop); break;
            #endif
            default : throw exception("selected worker model is not supported on current OS");
        }
    }
}

const LoopSP& Manager::loop () const {
    return mpm->get_loop();
}

const Manager::Config& Manager::config () const {
    return mpm->get_config();
}

void Manager::run () {
    mpm->server_factory = server_factory;
    mpm->start_event    = start_event;
    mpm->spawn_event    = spawn_event;
    mpm->request_event  = request_event;
    mpm->run();
}

void Manager::stop () {
    mpm->stop();
}

void Manager::restart_workers () {
    mpm->restart_workers();
}

excepted<void, string> Manager::reconfigure (const Config& cfg) {
    return mpm->reconfigure(cfg);
}

Manager::~Manager () {
    delete mpm;
}

}}}}
