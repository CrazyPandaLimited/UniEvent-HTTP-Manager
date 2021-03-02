#pragma once
#include "Mpm.h"

namespace panda { namespace unievent { namespace http { namespace manager {

struct Thread : Mpm {
    Thread (const Config&, const LoopSP&, const LoopSP&);

    void      run           () override;
    WorkerPtr create_worker () override;
    void      stop          () override;
    void      stopped       () override;
};

}}}}
