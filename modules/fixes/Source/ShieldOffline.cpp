#include "PCH.hpp"

#include "Fixes.hpp"
#include "Utils/MemUtils.hpp"

#define ADDR_ACTIVE ((PDWORD)0x639d510)
DWORD oldAddress;

__declspec(naked) void ActiveHook()
{
    __asm {
        mov	eax, [ecx+4]		// CSolar
        cmp	dword ptr [eax+0xB8], 0 // get_hit_pts
        je	inactive
        jmp	oldAddress
      inactive:
        mov	al, 0
        ret
      }
}

void Fixes::EnsureShieldOfflineWhileInDeathFuse()
{
    MemUtils::Protect(ADDR_ACTIVE, 4, false);
    oldAddress = *ADDR_ACTIVE;
    *ADDR_ACTIVE = reinterpret_cast<DWORD>(ActiveHook);
}