#include "roster.hpp"

extern "C"
{
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
}