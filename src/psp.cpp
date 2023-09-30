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
#include "roster.hpp"
#include "reader.hpp"
#include "patching.hpp"

#define EMULATOR_DEVCTL__IS_EMULATOR 0x00000003

#if defined(USER_SPACE)
PSP_MODULE_INFO("Allefresher_user", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
#elif defined(KERNEL_SPACE)
PSP_MODULE_INFO("Allefresher_kernel", PSP_MODULE_KERNEL, 1, 0);
PSP_MAIN_THREAD_ATTR(0);
#else
#error Neither kernel nor user space specified
#endif

#define NP_SERVICE_MODULE_NAME "sceNpService"

STMOD_HANDLER old;

#define DIGEST_OFFSET_FROM_LONE_HTTPS 0x44

char domain[MAX_LINE_SIZE];
char format[MAX_LINE_SIZE];

#define DEFAULT_PATH "ef0:SEPLUGINS/"
#define MEMORY_STICK_PATH "ms0:SEPLUGINS/"
#define DOMAIN_FILE "Allefresher_domain.txt"
#define FORMAT_FILE "Allefresher_format.txt"

#if defined(USER_SPACE) && defined(KERNEL_SPACE)
#error Both user and kernel space cannot be used at once!
#endif

#ifdef USER_SPACE
extern "C" char *customEula(u32 unknown, u32 unknown2)
{
	return "THIS IS A TEST WAAAAA";
}

void patchLBPPSP(u32 text_addr, u32 text_size)
{
	// Patch the EULA function
	patchMIPSFunction(text_addr + 0x1FE338, &customEula);
}
#endif

#ifdef KERNEL_SPACE
// Searches for, and patches the relevant strings in the binary
void patchLBPPSP(u32 text_addr, u32 text_size)
{
	// Try to read the domain string
	if (!readFileFirstLine(DEFAULT_PATH DOMAIN_FILE, domain) && !readFileFirstLine(MEMORY_STICK_PATH DOMAIN_FILE, domain))
	{
		return;
	}

	// Try to read the format string
	if (!readFileFirstLine(DEFAULT_PATH FORMAT_FILE, format) && !readFileFirstLine(MEMORY_STICK_PATH FORMAT_FILE, format))
	{
		return;
	}

	u32 end_addr = text_addr + text_size;

	Kprintf("Patching LBPPSP binary (0x%08x 0x%08x)", text_addr, text_size);

	const char originalNpBranch[] = {0x4C, 0x00, 0x40, 0x04, 0x21, 0x18, 0x00, 0x00};
	const char patchedNpBranch[] = {0x4C, 0x00, 0x41, 0x04, 0x21, 0x18, 0x00, 0x01};

	u32 iter_addr = text_addr;
	while (iter_addr < end_addr - 8)
	{
		// Get the current machine word as a pointer
		char *ptr = (char *)iter_addr;

		// Check for the lone `https` string
		if (strcmp("https", ptr) == 0)
		{
			Kprintf("Found lone HTTPS string at addr 0x%08x\n", iter_addr);

			patchString(ptr, "http");

			char *digestPtr = ptr - DIGEST_OFFSET_FROM_LONE_HTTPS;

			if (strlen(digestPtr) == 18)
			{
				// Patch the digest, with a fixed offset
				patchString(digestPtr, "CustomServerDigest");
			}
			else
			{
				Kprintf("Invalid digest %s!\n", digestPtr);
			}
		}
		// Check for the domain
		else if (strcmp("lbppsp.online.scee.com", ptr) == 0)
		{
			Kprintf("Found domain at addr 0x%08x\n", iter_addr);

			patchString(ptr, domain);
		}
		// Check for the http format string
		else if (strcmp("http://%s:10060/LITTLEBIGPLANETPSP_XML%s", ptr) == 0)
		{
			Kprintf("Found http format string at addr 0x%08x\n", iter_addr);

			patchString(ptr, format);
		}
		// Check for the https format string
		else if (strcmp("https://%s:10061/LITTLEBIGPLANETPSP_XML%s", ptr) == 0)
		{
			Kprintf("Found https format string at addr 0x%08x\n", iter_addr);

			patchString(ptr, format);
		}
		// Check for the np roster branching code
		else if (memcmp(ptr, originalNpBranch, 8) == 0)
		{
			Kprintf("Found NP roster branching code at addr 0x%08x\n", iter_addr);

			memcpy(ptr, patchedNpBranch, 8);
		}

		// Go forward to the next machine word
		iter_addr += sizeof(u32);
	}
}
#endif

int pspModuleHandler(SceModule2 *module)
{
	// Check if we are loading the sceNpService module
	if (strcmp(NP_SERVICE_MODULE_NAME, module->modname) == 0)
	{
#if defined(KERNEL_SPACE)
		// Get the original syscalls
		void *sceNpRosterCreateRequest = (void *)sctrlHENFindFunction(NP_SERVICE_MODULE_NAME, NP_SERVICE_MODULE_NAME, 0xBE22EEA3);
		void *sceNpRosterGetFriendListEntry = (void *)sctrlHENFindFunction(NP_SERVICE_MODULE_NAME, NP_SERVICE_MODULE_NAME, 0x4E851B10);
		void *sceNpRosterAbort = (void *)sctrlHENFindFunction(NP_SERVICE_MODULE_NAME, NP_SERVICE_MODULE_NAME, 0x5F5E32AF);
		void *sceNpRosterDeleteRequest = (void *)sctrlHENFindFunction(NP_SERVICE_MODULE_NAME, NP_SERVICE_MODULE_NAME, 0x66C64821);

		// Patch them out
		sctrlHENPatchSyscall(sceNpRosterCreateRequest, (void *)&sceNpRosterCreateRequestStub);
		sctrlHENPatchSyscall(sceNpRosterGetFriendListEntry, (void *)&sceNpRosterGetFriendListEntryStub);
		sctrlHENPatchSyscall(sceNpRosterAbort, (void *)&sceNpRosterAbortStub);
		sctrlHENPatchSyscall(sceNpRosterDeleteRequest, (void *)&sceNpRosterDeleteRequestStub);

		Kprintf("Patched 4 sceNpService syscalls! 0x%08x 0x%08x 0x%08x 0x%08x\n", sceNpRosterCreateRequest, sceNpRosterGetFriendListEntry, sceNpRosterAbort, sceNpRosterDeleteRequest);

		// Flush memory caches
		sceKernelDcacheWritebackAll();
#endif
	}
	// Check if we are loading the LBP PSP module
	else if (strcmp("LBPPSP", module->modname) == 0)
	{
		// Patch the LBP PSP module
		patchLBPPSP(module->text_addr, module->text_size);

		// Flush memory caches
		sceKernelDcacheWritebackAll();
	}

	return old(module);
}

int ppssppCheckModules()
{
	Kprintf("PPSSPP TODO!");
	return 0;
}

extern "C" int module_start(SceSize args, void *argp)
{
	// Detect PPSSPP
	if (sceIoDevctl("kemulator:", EMULATOR_DEVCTL__IS_EMULATOR, NULL, 0, NULL, 0) == 0)
	{
		Kprintf("Detected PPSSPP, doing manual module check/patch");
		ppssppCheckModules();
	}
	else
	{
		Kprintf("Added module handler");
		old = sctrlHENSetStartModuleHandler(pspModuleHandler);
	}

	return 0;
}
