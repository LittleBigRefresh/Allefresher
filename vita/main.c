#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/gxm.h>
#include <psp2/io/fcntl.h>
#include <psp2kern/kernel/debug.h>
#include <taihen.h>
#include <stdio.h>

#include "reader.h"

char GAME_URL[256];

static SceUID https_hook;
static tai_hook_ref_t https_ref;

static SceUID http_hook;
static tai_hook_ref_t http_ref;

static SceUID resource_hook;
static tai_hook_ref_t resource_ref;

// This is the HTTPS url the game uses, its fine if its not actually HTTPS
char *getHttpsUrl(int arg1)
{
    return GAME_URL;
}

// This is the HTTP url the game uses, its fine if its not actually HTTP
char *getHttpUrl(int arg1)
{
    return GAME_URL;
}

// This returns the URL the game uses to fetch a specific resource
char *getResourceUrl(char *out, char *hash)
{
    // http://%s/r/%s

    // TODO: allow resource URL to be set to a different server

    // Copy the base of the URL
    sceClibMemcpy(out, GAME_URL, strlen(GAME_URL));
    // Copy the resource subdir prefix
    sceClibMemcpy(out + strlen(GAME_URL), "/r/", 3);
    // Copy the hash to the end of the URL
    sceClibMemcpy(out + strlen(GAME_URL) + 3, hash, strlen(hash));
    // Null terminate the string
    out[strlen(GAME_URL) + strlen(hash) + 3] = '\0';

    return out;
}

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *args)
{
    sceClibPrintf("allefresher module start! looking for config...\n");

    // Try to load the URL from the file, if it fails, just use the default URL
    if (readFileFirstLine("ux0:/allefresher.txt", GAME_URL) == 0)
    {
        sceClibPrintf("Failed to read allefresher.txt, using default URL\n");

        strcpy(GAME_URL, "https://lbp.littlebigrefresh.com/lbp");
    }
    else
    {
        // If it loaded correctly and the last character is a / remove it, this is to make sure the game doesn't accidentally format double //
        if (GAME_URL[strlen(GAME_URL) - 1] == '/')
        {
            GAME_URL[strlen(GAME_URL) - 1] = '\0';
        }

        sceClibPrintf("Loaded user provided URL %s\n", GAME_URL);
    }

    sceClibPrintf("Final base URL: %s\n", GAME_URL);

    sceClibPrintf("Hooking functions...\n");

    tai_module_info_t info;
    info.size = sizeof(tai_module_info_t);

    taiGetModuleInfo(TAI_MAIN_MODULE, &info);

    // Patch the game's get_https_url function to return our own URL
    https_hook = taiHookFunctionOffset(
        &https_ref,
        info.modid,
        0,        // Segment index
        0x163a7e, // 0x81163a7e
        1,        // ARM/THUMB
        getHttpsUrl);
    sceClibPrintf("Hooked HTTPS: %08x\n", https_hook);

    // Patch the game's get_http_url function to return our own URL
    http_hook = taiHookFunctionOffset(
        &http_ref,
        info.modid,
        0,        // Segment index
        0x163994, // 0x81163994
        1,        // ARM/THUMB
        getHttpUrl);
    sceClibPrintf("Hooked HTTP: %08x\n", http_hook);

    // Patch the game's get_resource_url function to return our own formatted URLs
    resource_hook = taiHookFunctionOffset(
        &resource_ref,
        info.modid,
        0,        // Segment index
        0x163914, // 0x81163914
        1,        // ARM/THUMB
        getResourceUrl);
    sceClibPrintf("Hooked resource URL: %08x\n", resource_hook);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    taiHookRelease(https_hook, https_ref);
    taiHookRelease(http_hook, http_ref);
    taiHookRelease(resource_hook, resource_ref);

    return SCE_KERNEL_STOP_SUCCESS;
}
