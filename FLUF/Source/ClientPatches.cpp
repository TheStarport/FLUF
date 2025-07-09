#include "PCH.hpp"

#include "Fluf.hpp"
#include "Utils/MemUtils.hpp"
#include <msi.h>
#include <xbyak/xbyak.h>

#pragma comment(lib, "msi.lib")

#define PATCH(address, ...)                                    \
    {                                                          \
        byte patch[] = { __VA_ARGS__ };                        \
        MemUtils::WriteProcMem(address, patch, sizeof(patch)); \
    }

void* WaypointCheckDetour(int index)
{
    auto module = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    const static auto maxWaypoint = reinterpret_cast<int*>(module + 0x273374);
    const static auto playerSystem = reinterpret_cast<uint*>(module + 0x273354);
    if (index < 0 || index >= *maxWaypoint)
    {
        return nullptr;
    }

    const auto waypointSystem = reinterpret_cast<uint*>((24 * index) + 0x672984);
    if (*waypointSystem != *playerSystem)
    {
        return nullptr;
    }

    return (void*)((24 * index) + 0x672978);
}

void Fluf::ClientPatches()
{
    Log(LogLevel::Info, "Getting the info");

    const auto common = reinterpret_cast<DWORD>(GetModuleHandleA("common.dll"));
    byte oneBytePatch = 0xEB;
    MemUtils::WriteProcMem(common + 0x0E698E, &oneBytePatch, 1); // Planets respect spin value

    const float maxDockRange = 20'000;
    MemUtils::WriteProcMem(common + 0x13F48C, &maxDockRange, 4);

    const auto fl = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    PATCH(fl + 0x01AD6F, 0x00, 0x20);  // Fix low quality texture bug
    PATCH(fl + 0x02477A, 0x00, 0x00);  // Remove window borders in windowed mode
    PATCH(fl + 0x002490D, 0x00, 0x00); // Remove window borders in windowed mode

    char chatCount = 127; // Update max lines and characters for multiplayer chat
    MemUtils::WriteProcMem(fl + 0x06A440, &chatCount, 1);
    MemUtils::WriteProcMem(fl + 0x0691D1, &chatCount, 1);

    PATCH(fl + 0xD094B, 0x74, 0x04, 0xFE, 0xC8, 0x75, 0xC4);                                            // Include player ship in contact filter
    PATCH(fl + 0x0D5936, 0x90, 0xE9);                                                                   // Remove cruise display limit
    MemUtils::WriteProcMem(fl + 0x0D5984, &oneBytePatch, 1);                                            // Remove overall speed limit
    PATCH(fl + 0x158A14, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90);                                           // Prevent occasional random frame drops
    MemUtils::WriteProcMem(fl + 0x166C2B, &oneBytePatch, 1);                                            // Remove ESRB notice
    PATCH(fl + 0x1ACF6A, 0x40, 0x74, 0x63, 0x48, 0x51, 0x8D, 0x54, 0xE4, 0x20, 0x52, 0x83, 0xE9, 0x08); // Prevent ipv6 address from appearing
    oneBytePatch = 0x00;
    MemUtils::WriteProcMem(fl + 0x1E6DCC, &oneBytePatch, 1); // Allow multiple instances of Freelancer to run

    PATCH(fl + 0x08D89B, 0x83, 0xC5, 0x18, 0xEB, 0x50); // Allow the navmap to show all group members
    MemUtils::WriteProcMem(fl + 0x08D997, &oneBytePatch, 1);

    const auto rendComp = reinterpret_cast<DWORD>(GetModuleHandleA("rendcomp.dll"));
    MemUtils::WriteProcMem(rendComp + 0x00C499, &oneBytePatch, 1); // Improve planet textures at range

    // Schmack's renderer replaces this file, and we want to assert that it is a vanilla version before patching
    if (const auto rp8 = reinterpret_cast<DWORD>(GetModuleHandleA("rp8.dll")); rp8 && *PBYTE(rp8 + 0x004467) == 0x7D)
    {
        oneBytePatch = 0xEB;
        MemUtils::WriteProcMem(rp8 + 0x004467, &oneBytePatch, 1); // Remove erroneous D3DERR_INVALIDCALL message
    }

    // Disable erroneous zone warning
    oneBytePatch = 0x28;
    MemUtils::WriteProcMem(common + 0xDC34C, &oneBytePatch, 1);

    // Adjust the fate test to treat below 3 as bad, and below/equal to 5 as good
    PATCH(common + 0x33487, 0x72, 0x05, 0x83, 0xF8, 0x05, 0x76);

    // Disable ArchDB::Get warnings for random missions
    MemUtils::NopAddress(common + 0x995B6, 2);
    MemUtils::NopAddress(common + 0x995B6 + 0x46, 2);

    // Patch warning about cockpit aspect ratio
    oneBytePatch = 0xEB;
    MemUtils::WriteProcMem(0x5176cc, &oneBytePatch, 1);
    MemUtils::WriteProcMem(0x5188e7, &oneBytePatch, 1);

    // Skip hostile pick assistance message
    PATCH(0x4ede00, 0xEB, 0x26);
    PATCH(0x452648, 0x6A, 0x0, 0x8B, 0x76, 0x04, 0xFF, 0x76, 0x08, 0x90, 0x90)

    // Skip warning for invalid waypoint material
    if (const auto offset = reinterpret_cast<PDWORD>(rendComp + 0x118BE); *offset == 0x8B)
    {
        using namespace Xbyak::util;
        static Xbyak::CodeGenerator code;
        code.mov(edx, ptr[ebp]);
        code.lea(eax, ptr[esp + 0x28]);
        code.cmp(edx, 163618903);
        code.ret();

        std::array<::byte, 8> patch = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x74, 0x2D, 0x52 };
        *reinterpret_cast<PDWORD>(patch.data() + 1) = *reinterpret_cast<const unsigned long*>(code.getCode());
        MemUtils::WriteProcMem(reinterpret_cast<DWORD>(offset), &patch, patch.size());
    }

    // Ensure that waypoints are checked between systems
    MemUtils::PatchCallAddr(fl, 0xF4141, WaypointCheckDetour);
}
