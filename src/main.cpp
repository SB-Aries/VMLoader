#include "vm/config.hpp"

#if VM_INJECT_TECHNIQUE == 2

#include "vm/inject.hpp"

int main() {
    vm_execute(nullptr);
    return 0;
}

#else

#include "vm/loader.hpp"
#include "vm/inject.hpp"
#include "vm/syscall.hpp"

int main() {
    RiscVm vm{};
    if (!vm_load(&vm, &g_win_syscalls))
        return 1;
    vm_execute(&vm);
    return 0;
}

#endif
