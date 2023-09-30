extern "C"
{
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <systemctrl.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
}

void patchString(char *targetAddress, char *patchedString);

template <typename T>
void patchFunction(u32 address, T *function)
{
    u32 *data = (u32 *)address;
    data[0] = 0x8000000 | (((u32)function & 0xFFFFFFF) >> 2);
    data[1] = 0;
}