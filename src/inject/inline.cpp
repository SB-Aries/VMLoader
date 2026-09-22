#include "vm/config.hpp"
#if VM_INJECT_TECHNIQUE == 0

#include "vm/inject.hpp"

void vm_execute(RiscVm* vm) {
    riscvm_run(vm);
}

#endif
