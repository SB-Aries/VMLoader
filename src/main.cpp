#include "janus/config.hpp"

#if JANUS_INJECT_TECHNIQUE == 2

#include "janus/inject.hpp"

int main() {
    janus_execute(nullptr);
    return 0;
}

#else

#include "janus/loader.hpp"
#include "janus/inject.hpp"
#include "janus/syscall.hpp"

int main() {
    RiscVm vm{};
    if (!janus_load(&vm, &g_win_syscalls))
        return 1;
    janus_execute(&vm);
    return 0;
}

#endif
