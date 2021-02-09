#pragma once
#include "Mpm.h"

namespace panda { namespace unievent { namespace http { namespace manager {

struct Thread : Mpm {
    Thread (const Config&, const LoopSP&);

    void      run           () override;
    void      fetch_state   () override;
    WorkerPtr create_worker () override;
};

}}}}
