extern "C"
{
#include <systemctrl.h>
}

extern "C"
{
    int sceNpRosterCreateRequestStub();
    int sceNpRosterGetFriendListEntryStub(s32 rosterId, s32 unknown1, s32 unknown2, u32 unknown3Addr, u32 known4Addr, u32 unknown5Addr, u32 unkown6);
    int sceNpRosterAbortStub(u32 rosterId);
    int sceNpRosterDeleteRequestStub(u32 rosterId);
}