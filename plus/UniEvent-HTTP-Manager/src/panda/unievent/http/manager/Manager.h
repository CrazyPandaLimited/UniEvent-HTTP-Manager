#pragma once
#include <panda/unievent/http/Server.h>

namespace panda { namespace unievent { namespace http { namespace manager {

struct Mpm;

extern log::Module panda_log_module;

struct Manager : Refcnt {
    enum class WorkerModel { PreFork, Thread };
    enum class BindModel   { Duplicate, ReusePort };

    struct Config {
        Server::Config server;
        uint32_t       min_servers = 1;           // The minimum number of servers to keep running
        uint32_t       max_servers = 0;           // The maximum number of child servers to start. [min_servers*3]
        uint32_t       min_spare_servers = 0;     // The minimum number of servers to have waiting for requests.
        uint32_t       max_spare_servers = 0;     // The maximum number of servers to have waiting for requests. [min_spare_server + min_servers, if min_spare_servers]
        float          min_load = 0;              // minimum average loop load on workers {0-1} [max_load/2 if max_load]
        float          max_load = 0;              // maximum average loop load on workers {0-1} [0.7 if !min_spare_servers]
        uint32_t       load_average_period = 3;   // number of seconds to collect load average for, on workers
        uint32_t       max_requests = 0;          // max number of the requests to process per one worker process [0=unlimited]
        uint32_t       min_worker_ttl = 60;       // Minimum number of seconds between starting and killing a worker
        float          check_interval = 1;        // interval between checking to see if we can kill off some waiting servers or if we need to spawn more workers
        uint32_t       activity_timeout = 0;      // kill worker if it's not responding for this number of seconds [0=disable]
        uint32_t       termination_timeout = 0;   // kill worker if it's not terminated after this number of seconds [0=disable]
        WorkerModel    worker_model =             // Multi-processing module type
            #ifdef _WIN32
                WorkerModel::Thread;
            #else
                WorkerModel::PreFork;
            #endif
        BindModel      bind_model = BindModel::Duplicate; // how to bind http server sockets in workers
    };

    using start_fptr        = void();
    using start_fn          = function<start_fptr>;
    using start_cd          = CallbackDispatcher<start_fptr>;
    using server_factory_fn = function<ServerSP(const Server::Config&, const LoopSP&)>;
    using spawn_fptr        = void(const ServerSP&);
    using spawn_fn          = function<spawn_fptr>;
    using spawn_cd          = CallbackDispatcher<spawn_fptr>;
    using request_cd        = decltype(std::declval<Server>().request_event);

    start_cd          start_event;
    server_factory_fn server_factory;
    spawn_cd          spawn_event;
    request_cd        request_event;

    Manager (const Config&, const LoopSP& = {});

    const LoopSP& loop () const;

    void run  ();
    void stop ();

    virtual ~Manager ();

private:
    Mpm* mpm;
};

using ManagerSP = iptr<Manager>;

}}}}
