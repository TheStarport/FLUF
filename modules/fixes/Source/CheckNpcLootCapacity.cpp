#include "PCH.hpp"

#include "Fixes.hpp"
#include "Utils/MemUtils.hpp"

float cargoHoldSpaceRemaining;
#define ADDR_CARGO  ((DWORD)(0x6E8E8))
#define ADDR_VOLUME ((DWORD)0x62ce9d9)
const auto holdRemaining = GetProcAddress(GetModuleHandleA("common.dll"), "?get_cargo_hold_remaining@CShip@@QBEMXZ");

__declspec(naked) void CargoHook()
{
    __asm {
        call holdRemaining
        fst	cargoHoldSpaceRemaining
        ret
    }
}

__declspec(naked) void VolumeCheckHook()
{
    __asm {
        fld	dword ptr [ecx+0x5c] // volume of the loot
        fcomp	cargoHoldSpaceRemaining
        fnstsw	ax
        test	ah, 0x41
        jz	too_big
        mov	edx, [ecx]
        call	dword ptr [edx+0x10]
        ret

      too_big:
        mov	dword ptr [esp], 0x62ceaf4
        ret
    }
}

void Fixes::FixNpcLootCapacity()
{
    MemUtils::PatchAssembly(ADDR_VOLUME, VolumeCheckHook, true);
    MemUtils::PatchCallAddr(ADDR_CARGO, reinterpret_cast<DWORD>(GetModuleHandleA("common.dll")), CargoHook);
}
