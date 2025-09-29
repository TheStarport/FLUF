#include "PCH.hpp"

#include "Fixes.hpp"
#include "Utils/Detour.hpp"
#include "Utils/MemUtils.hpp"

DWORD ConnectOld, ConnectDblOld;

__declspec(naked) void ConnectHook()
{
    __asm {
        push	eax
        call	ConnectOld
        test	al, al
        jz	done // nothing selected
        mov	ecx, [esi+0xC4]
        mov	eax, 0x58d0b0
        push	dword ptr [esp+4+8]
        call	eax // get the selected server
        mov	eax, [eax+8]
        test	eax, eax
        jz	done
        mov	eax, [eax+0x94] // its version
        cmp	eax, ds:[0x67ecb8] // client's version
        je	done
        mov	dword ptr [esp], 0x56f78a
      done:
        ret	4
    }
}

__declspec(naked) void ConnectDblHook()
{
    __asm {
        mov	eax, [eax+0x8c]
        cmp	eax, ds:[0x67ecb8]
        jne	done
        jmp	ConnectDblOld
      done:
        xor	al, al
        ret	4
    }
}

void Fixes::PatchSectorLetters()
{
    // Change the dash arrow to an en-dash and arrow, but only if it hasn't been patched by something else
    if (auto* arrowStringAddress = reinterpret_cast<wchar_t*>(0x5cef58); arrowStringAddress[0] == '-' && arrowStringAddress[1] == '>')
    {
        MemUtils::Protect(arrowStringAddress, 2, false);
        *arrowStringAddress = 0x2013;
    }

// Place the sector letter before the number
#define ADDR_LETTER1a ((PBYTE)0x48dcf5) // object in space
#define ADDR_LETTER1b ((PBYTE)0x48dd01)
#define ADDR_LETTER2a ((PBYTE)0x41d1e6) // mission description
#define ADDR_LETTER2b ((PBYTE)0x41d250)
#define ADDR_LETTER3a ((PBYTE)0x4baee8) // Neural Net
#define ADDR_LETTER3b ((PBYTE)0x4baf52)

    MemUtils::Protect(ADDR_LETTER1a, 0x0C);
    MemUtils::Protect(ADDR_LETTER2a, 0x70);
    MemUtils::Protect(ADDR_LETTER3a, 0x70);
    *ADDR_LETTER1a = 0x4c;
    *ADDR_LETTER1b = 0x30;
    *ADDR_LETTER2a = *ADDR_LETTER3a = 0x57;
    *ADDR_LETTER2b = *ADDR_LETTER3b = 0x53;
}

#define ADDR_ROLL ((DWORD*)0x4c7f14)
void __thiscall Fixes::OnRollHook(IBehaviorManager* behaviour, Vector& vec)
{
    if (vec.x != 0.f || vec.y != 0.f)
    {
        *ADDR_ROLL = 0;
    }
    else
    {
        *ADDR_ROLL = *reinterpret_cast<DWORD*>(&vec.z);
    }

    behaviour->update_current_behavior_direction(vec);
}

void Fixes::SupportRollingDuringMouseFlight()
{
    MemUtils::PatchCallAddr(DWORD(GetModuleHandleA(nullptr)), 0xc7940, &Fixes::OnRollHook);
    MemUtils::Protect(ADDR_ROLL, 4);
}

void Fixes::AllowViewingOfServerInfoForDifferentVersions()
{
#define ADDR_SELECT     ((PWORD)0x5711a1)
#define ADDR_CONNECTDBL ((PDWORD)(0x571aea + 1))
#define ADDR_CONNECT    ((PDWORD)(0x56f778 + 1))
    MemUtils::Protect(ADDR_SELECT, 2);
    MemUtils::Protect(ADDR_CONNECT, 4);
    *ADDR_SELECT = 0x01B0;

    NEWOFS(ADDR_CONNECT, ConnectHook, ConnectOld);
    NEWOFS(ADDR_CONNECTDBL, ConnectDblHook, ConnectDblOld);
}

void Fixes::ForceWeaponPrimaryWeaponGroupOnLaunch()
{
#define ADDR_WG1 ((PBYTE)0x4db6a4 + 1)
#define ADDR_WG2 ((PWORD)0x4db6b7)

    MemUtils::Protect(ADDR_WG1, 1);
    *ADDR_WG1 = 7;
    *ADDR_WG2 = 0x006A;
}

void Fixes::FixSoundsNotBeingPlayed()
{
    const auto fl = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));

    // Enable sounds with thrusters that go backwards
    constexpr std::array<byte, 6> thrusterSounds1{ 0xD9, 0xE1, 0xD9, 0x5C, 0xE4, 0x08 };
    constexpr std::array<byte, 1> thrusterSounds2{};
    MemUtils::WriteProcMem(fl + 0x012F217, thrusterSounds1.data(), thrusterSounds1.size());
    MemUtils::WriteProcMem(fl + 0x012F221, thrusterSounds2.data(), thrusterSounds2.size());

    // Fix extra HP_Fire audio being
    const auto common = reinterpret_cast<DWORD>(GetModuleHandleA("common.dll"));
    const std::array<byte, 9> hpFirePatch = { 0x83, 0xFA, 0xFF, 0xBA, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3 };
    MemUtils::WriteProcMem(common + 0x039F77, hpFirePatch.data(), hpFirePatch.size());
}

__declspec(naked) void ManualLaunchOnSpecialBase()
{
    __asm
    {
        // call original set_directive_callback
        // the behaviour manager is in ecx
        // the directive is on the stack and in edi
        mov eax, 0x62d3EB0
        call eax

            // Find the object type that the ship is launching from
        mov eax, edi
        sub eax, 0x18
        mov eax, [eax+0x28]
        test eax, 0x200100
        jz pbase_launch

            // This is a station or capitalship
        push 0x62b0197
        ret

pbase_launch:
        // This is a NON_TARGETABLE object so do manual launch
        // otherwise call the original function
        push 6
        push 0
        mov ecx, edi
        mov eax, 0x62b01f0
        call eax
        push 0x62b01dc
        ret
    }
}

void Fixes::AllowUndockingFromNonTargetableObject()
{
    const auto common = DWORD(GetModuleHandleA("common.dll"));
    constexpr BYTE patch[] = { 0xFF, 0xFF, 0xFF, 0xFF };
    MemUtils::WriteProcMem(common + 0x5013F, patch, sizeof(patch));

    constexpr BYTE patch1[] = { 0xe9 }; // jmpr
    MemUtils::WriteProcMem(common + 0x50192, patch1, sizeof(patch1));
    MemUtils::PatchCallAddr(common, 0x50192, &ManualLaunchOnSpecialBase);
}
