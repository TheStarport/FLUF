#include "PCH.hpp"

#include "Fluf.hpp"
#include "Utils/MemUtils.hpp"
#include <msi.h>

#pragma comment(lib, "msi.lib")

#define PATCH(address, ...)                                    \
    {                                                          \
        byte patch[] = { __VA_ARGS__ };                        \
        MemUtils::WriteProcMem(address, patch, sizeof(patch)); \
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

    // Get the hash of rp8.dll. Schmack's renderer replaces this file, and we want to assert that it is a vanilla version before patching
    MSIFILEHASHINFO info;
    MsiGetFileHashA("rp8.dll", 0, &info);

    if (info.dwData[0] == 2946203618 && info.dwData[1] == 2924112751 && info.dwData[2] == 2974468154 && info.dwData[3] == 3757039764)
    {
        oneBytePatch = 0xEB;
        const auto rp8 = reinterpret_cast<DWORD>(GetModuleHandleA("rp8.dll"));
        MemUtils::WriteProcMem(rp8 + 0x004467, &oneBytePatch, 1); // Remove erroneous D3DERR_INVALIDCALL message
    }
}
