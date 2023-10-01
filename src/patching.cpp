#include "patching.hpp"

#if VITA
#define Kprintf(...)
#endif

void patchString(char *targetAddress, char *patchedString)
{
    // Make sure we dont patch a string to be too long
    if (strlen(patchedString) > strlen(targetAddress))
    {
        Kprintf("Unable to patch %s (%08x) to %s (%08x), as it will not fit\n", targetAddress, (SceSize)targetAddress, patchedString, (SceSize)patchedString);
    }

    Kprintf("Patching %s (%08x) to %s (%08x)\n", targetAddress, (SceSize)targetAddress, patchedString, (SceSize)patchedString);
    strcpy(targetAddress, patchedString);
    Kprintf("Target after patch: %s\n", targetAddress);
}