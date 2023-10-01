#include <psp2common/kernel/modulemgr.h>
#include <taihen.h>

// handle to our hook
static tai_hook_ref_t app_start_ref;

extern "C" int lbpv_module_start(SceSize argc, const void *args)
{
    struct _tai_hook_user *cur, *next;
    cur = (struct _tai_hook_user *)(app_start_ref);
    next = (struct _tai_hook_user *)cur->next;
    return (next == NULL) ? ((int (*)(SceSize, const void *))cur->old)(argc, args)
                          : ((int (*)(SceSize, const void *))next->func)(argc, args);
}

extern "C" int _start(SceSize argc, const void *args) __attribute__((weak, alias("module_start")));
extern "C" int module_start(SceSize argc, const void *args)
{
    taiHookFunctionExport(&app_start_ref,  // Output a reference
                          "LBP2_Final",    // Name of module being hooked
                          TAI_ANY_LIBRARY, // If there's multiple libs exporting this
                          0x935CD196,      // Special NID specifying module_start
                          (void *)&lbpv_module_start);

    return SCE_KERNEL_START_SUCCESS;
}

extern "C" int module_stop(SceSize argc, const void *args)
{
    return SCE_KERNEL_STOP_SUCCESS;
}