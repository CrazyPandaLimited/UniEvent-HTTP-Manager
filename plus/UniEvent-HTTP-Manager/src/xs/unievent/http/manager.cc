#include "manager.h"

namespace xs { namespace unievent { namespace http {

void fill (Manager::Config& cfg, const Hash& h) {
    Sv sv;
    if ((sv = h.fetch("server")))              fill(cfg.server, sv);
    if ((sv = h.fetch("min_servers")))         cfg.min_servers       = xs::in<uint32_t>(sv);
    if ((sv = h.fetch("max_servers")))         cfg.max_servers       = xs::in<uint32_t>(sv);
    if ((sv = h.fetch("min_spare_servers")))   cfg.min_spare_servers = xs::in<uint32_t>(sv);
    if ((sv = h.fetch("max_spare_servers")))   cfg.max_spare_servers = xs::in<uint32_t>(sv);
    if ((sv = h.fetch("min_load")))            cfg.min_load          = xs::in<float>(sv);
    if ((sv = h.fetch("max_load")))            cfg.max_load          = xs::in<float>(sv);
    if ((sv = h.fetch("max_requests")))        cfg.max_requests      = xs::in<uint32_t>(sv);
    if ((sv = h.fetch("min_worker_ttl")))      cfg.min_worker_ttl    = xs::in<uint32_t>(sv);
    if ((sv = h.fetch("check_interval")))      cfg.check_interval    = xs::in<uint32_t>(sv);
    if ((sv = h.fetch("activity_timeout")))    cfg.activity_timeout  = xs::in<uint32_t>(sv);
    if ((sv = h.fetch("termination_timeout"))) cfg.termination_timeout  = xs::in<uint32_t>(sv);

    if ((sv = h.fetch("worker_model"))) switch ((Manager::WorkerModel)(int)Simple(sv)) {
        case Manager::WorkerModel::PreFork : cfg.worker_model = Manager::WorkerModel::PreFork; break;
        case Manager::WorkerModel::Thread  : cfg.worker_model = Manager::WorkerModel::Thread; break;
        default: throw Simple("bad worker model supplied");
    }
    if ((sv = h.fetch("bind_model"))) switch ((Manager::BindModel)(int)Simple(sv)) {
        case Manager::BindModel::Duplicate : cfg.bind_model = Manager::BindModel::Duplicate; break;
        case Manager::BindModel::ReusePort : cfg.bind_model = Manager::BindModel::ReusePort; break;
        default: throw Simple("bad bind model supplied");
    }
}

}}}
