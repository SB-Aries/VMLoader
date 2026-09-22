#include "vm/config.hpp"
#if VM_INJECT_TECHNIQUE == 1

#include "vm/inject.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct FiberCtx {
    RiscVm* vm;
    LPVOID  main_fiber;
};

static VOID CALLBACK vm_fiber_proc(LPVOID param) {
    auto* ctx = static_cast<FiberCtx*>(param);
    riscvm_run(ctx->vm);
    SwitchToFiber(ctx->main_fiber);
}

void vm_execute(RiscVm* vm) {
    LPVOID main_fiber = ConvertThreadToFiber(nullptr);
    if (!main_fiber) {
        if (GetLastError() == ERROR_ALREADY_FIBER)
            main_fiber = GetCurrentFiber();
        else
            return;
    }

    FiberCtx ctx = { vm, main_fiber };
    LPVOID vm_fiber = CreateFiber(0, vm_fiber_proc, &ctx);
    if (!vm_fiber) {
        ConvertFiberToThread();
        return;
    }

    SwitchToFiber(vm_fiber);

    DeleteFiber(vm_fiber);
    ConvertFiberToThread();
}

#endif
