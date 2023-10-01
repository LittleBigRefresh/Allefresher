#include <psp2common/kernel/modulemgr.h>
#include <psp2/kernel/modulemgr.h>
#include <taihen.h>
#include <string.h>
#include "patching.hpp"
#include "re.h"

// handle to our hook
static tai_hook_ref_t app_start_ref;

#define LBPV_MODNAME "LBP2_Final"

#define url "http://refresh.jvyden.xyz:2095/lbp"

int roundUp(int num, int multiple)
{
    if (multiple == 0)
        return num;

    int remainder = num % multiple;
    if (remainder == 0)
        return num;

    return num + multiple - remainder;
}

extern "C" int lbpv_module_start(SceSize argc, const void *args)
{
    tai_module_info_t mod_info;
    mod_info.size = sizeof(tai_module_info_t);
    taiGetModuleInfo(LBPV_MODNAME, &mod_info);

    SceKernelModuleInfo sce_mod_info;
    sce_mod_info.size = sizeof(SceKernelModuleInfo);
    sceKernelGetModuleInfo(mod_info.modid, &sce_mod_info);

    int matchLength;
    re_t pattern = re_compile("^https?[^\\x00]//([0-9a-zA-Z.:].*)/?([0-9a-zA-Z_]*)$");
    for (int i = 0; i < 4; i++)
    {
        auto segment = sce_mod_info.segments[i];

        char *addr = (char *)segment.vaddr;
        char *endAddr = (char *)segment.vaddr + segment.size;
        while (addr < endAddr)
        {
            int len = strlen(addr);
            if (len < 7 || len > 100)
            {
                // Skip this whole string
                addr += roundUp(len, sizeof(SceSize));
                continue;
            }

            if (re_matchp(pattern, addr, &matchLength) != -1)
            {
                patchString(addr, url);
            }

            addr += sizeof(SceSize);
        }
    }

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
                          LBPV_MODNAME,    // Name of module being hooked
                          TAI_ANY_LIBRARY, // If there's multiple libs exporting this
                          0x935CD196,      // Special NID specifying module_start
                          (void *)&lbpv_module_start);

    return SCE_KERNEL_START_SUCCESS;
}

extern "C" int module_stop(SceSize argc, const void *args)
{
    return SCE_KERNEL_STOP_SUCCESS;
}