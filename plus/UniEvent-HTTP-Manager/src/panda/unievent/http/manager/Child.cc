#include "Child.h"
#include <ios>
#include <iomanip>
#include <thread>

namespace panda { namespace unievent { namespace http { namespace manager {

void Child::init (ServerParams p) {
    panda_log_debug("worker: creating server");
    loop = p.loop;
    loop->track_load_average(3);

    if (p.server_factory) server = p.server_factory(p.config.server, loop);
    else {
        server = new Server(loop);
        server->configure(p.config.server);
    }

    if (p.request_event.has_listeners()) server->request_event = p.request_event;

    p.spawn_event(server);

    server->route_event.add([this](auto& req) {
        ++reqcnt.total;
        ++reqcnt.recent;
        send_active_requests(++reqcnt.active);

        req->finish_event.add([this](auto&) {
            send_active_requests(--reqcnt.active);
        });
    });

    la_timer = Timer::start(1000, [this](auto&) {
        panda_log_debug("worker: load average=" << std::setprecision(2) << std::fixed << loop->get_load_average() << " " << reqcnt.recent << " req/s, total " << reqcnt.total << " reqs");
        reqcnt.recent = 0;
        send_activity(std::time(NULL), loop->get_load_average(), reqcnt.total);
    }, loop);
    la_timer->weak(true);
}

void Child::run () {
    panda_log_info("worker: running");
    server->run();
    send_activity(std::time(NULL), 0, 0);
    send_ready();
    loop->run();
    panda_log_info("worker: end running, total requests served: " << reqcnt.total);
}

void Child::terminate () {
    panda_log_info("worker: terminating...");
    if (terminating) return;
    terminating = true;
    server->stop_event.add([this]() {
        panda_log_debug("worker: server stopped. unblocking loop...");
        loop->stop();
    });
    server->graceful_stop();
}

}}}}
