#include "janus/config.hpp"
#if JANUS_INJECT_TECHNIQUE == 0

#include "janus/inject.hpp"

void janus_execute(RiscVm* vm) {
    riscvm_run(vm);
}

#endif
