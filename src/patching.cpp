#include "patching.hpp"

void patchString(char *targetAddress, char *patchedString)
{
    // Make sure we dont patch a string to be too long
    if (strlen(patchedString) > strlen(targetAddress))
    {
        Kprintf("Unable to patch %s (%08x) to %s (%08x), as it will not fit\n", targetAddress, (u32)targetAddress, patchedString, (u32)patchedString);
    }

    Kprintf("Patching %s (%08x) to %s (%08x)\n", targetAddress, (u32)targetAddress, patchedString, (u32)patchedString);
    strcpy(targetAddress, patchedString);
    Kprintf("Target after patch: %s\n", targetAddress);
}