extern "C"
{
#include <psp2common/types.h>
}

extern "C" int _start(SceSize argc, const void *args) __attribute__((weak, alias("module_start")));
extern "C" int module_start(SceSize argc, const void *args)
{
    return 0;
}

extern "C" int module_stop(SceSize argc, const void *args)
{
    return 0;
}