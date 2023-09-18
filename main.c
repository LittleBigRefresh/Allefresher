#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include "systemctrl.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define EMULATOR_DEVCTL__IS_EMULATOR 0x00000003

PSP_MODULE_INFO("imgwpatch", PSP_MODULE_KERNEL, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

#define NP_SERVICE_MODULE_NAME "sceNpService"

int sceNpRosterCreateRequestStub()
{
	Kprintf("STUB: create request\n");
	return 1;
}

int sceNpRosterGetFriendListEntryStub(s32 rosterId, s32 unknown1, s32 unknown2, u32 unknown3Addr, u32 known4Addr, u32 unknown5Addr, u32 unkown6)
{
	Kprintf("STUB: get friend list entry\n");
	return 0;
}

int sceNpRosterAbortStub(u32 rosterId)
{
	Kprintf("STUB: abort\n");
	return 0;
}

int sceNpRosterDeleteRequestStub(u32 rosterId)
{
	Kprintf("STUB: delete request\n");
	return 0;
}

STMOD_HANDLER old;

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

#define DIGEST_OFFSET_FROM_LONE_HTTPS 0x44

// Searches for, and patches the relavent strings in the binary
void patchBinary(u32 text_addr, u32 text_size)
{
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

			const u32 digestPtr = ptr - DIGEST_OFFSET_FROM_LONE_HTTPS;

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

			patchString(ptr, "tcp.beyleyisnot.moe");
		}
		// Check for the http format string
		else if (strcmp("http://%s:10060/LITTLEBIGPLANETPSP_XML%s", ptr) == 0)
		{
			Kprintf("Found http format string at addr 0x%08x\n", iter_addr);

			patchString(ptr, "http://%s:10061/lbp%s");
		}
		// Check for the https format string
		else if (strcmp("https://%s:10061/LITTLEBIGPLANETPSP_XML%s", ptr) == 0)
		{
			Kprintf("Found https format string at addr 0x%08x\n", iter_addr);

			patchString(ptr, "http://%s:10061/lbp%s");
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

int pspModuleHandler(SceModule2 *module)
{
	// Check if we are loading the sceNpService module
	if (strcmp(NP_SERVICE_MODULE_NAME, module->modname) == 0)
	{
		// Get the original syscalls
		void *sceNpRosterCreateRequest = (void *)sctrlHENFindFunction(NP_SERVICE_MODULE_NAME, NP_SERVICE_MODULE_NAME, 0xBE22EEA3);
		void *sceNpRosterGetFriendListEntry = (void *)sctrlHENFindFunction(NP_SERVICE_MODULE_NAME, NP_SERVICE_MODULE_NAME, 0x4E851B10);
		void *sceNpRosterAbort = (void *)sctrlHENFindFunction(NP_SERVICE_MODULE_NAME, NP_SERVICE_MODULE_NAME, 0x5F5E32AF);
		void *sceNpRosterDeleteRequest = (void *)sctrlHENFindFunction(NP_SERVICE_MODULE_NAME, NP_SERVICE_MODULE_NAME, 0x66C64821);

		// Patch them out
		sctrlHENPatchSyscall(sceNpRosterCreateRequest, sceNpRosterCreateRequestStub);
		sctrlHENPatchSyscall(sceNpRosterGetFriendListEntry, sceNpRosterGetFriendListEntryStub);
		sctrlHENPatchSyscall(sceNpRosterAbort, sceNpRosterAbortStub);
		sctrlHENPatchSyscall(sceNpRosterDeleteRequest, sceNpRosterDeleteRequestStub);

		Kprintf("Patched 4 sceNpService syscalls! 0x%08x 0x%08x 0x%08x 0x%08x\n", sceNpRosterCreateRequest, sceNpRosterGetFriendListEntry, sceNpRosterAbort, sceNpRosterDeleteRequest);
	}
	// Check if we are loading the LBP PSP module
	else if (strcmp("LBPPSP", module->modname) == 0)
	{
		// Patch the LBP PSP module
		patchBinary(module->text_addr, module->text_size);
	}

	return old(module);
}

int ppssppCheckModules()
{
	Kprintf("PPSSPP TODO!");
}

int module_start(SceSize args, void *argp)
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