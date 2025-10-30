#include "PCH.hpp"

#include "Fixes.hpp"
#include "Utils/MemUtils.hpp"

#include <Fluf.hpp>

PBYTE serverDll;

// Allow negative damage values (for [Zone]).
#define ADDR_NEGDMG1  ((PBYTE)(0x6337b30 + 2)) // from ini
#define ADDR_NEGDMG2  ((PBYTE)(0x633ad42 + 2)) // internal flag

// Allow add_damage_list to repair.
#define ADDR_DMGLST   ((PBYTE)(0x62bbc24 + 2))

// Test health doesn't exceed maximum.
#define ADDR_MAXEQUIP (serverDll + 0x6cea5f1 - 0x6ce0000)
#define ADDR_MAXCGRP  (serverDll + 0x6ceac35 - 0x6ce0000)
#define ADDR_MAXHULL  (serverDll + 0x6ceed1d - 0x6ce0000)

// Test for a negative damage (replace <= 0 with == 0).
#define ADDR_ARMORNEG (serverDll + 0x6cea3af + 2 - 0x6ce0000)
#define ADDR_EQUIPNEG (serverDll + 0x6cea513 + 2 - 0x6ce0000)
#define ADDR_CGRPNEG  (serverDll + 0x6ceab23 + 2 - 0x6ce0000)
#define ADDR_HULLNEG  (serverDll + 0x6ceed08 + 2 - 0x6ce0000)
#define ADDR_ZONENEG  (serverDll + 0x6d01199 + 2 - 0x6ce0000)
#define ADDR_EXPaNEG  (serverDll + 0x6ce647a + 2 - 0x6ce0000) // not sure if
#define ADDR_EXPbNEG  (serverDll + 0x6ce8480 + 2 - 0x6ce0000) //  these are
#define ADDR_EXPcNEG  (serverDll + 0x6cf0712 + 2 - 0x6ce0000) //  actually
#define ADDR_EXPdNEG  (serverDll + 0x6cf2b92 + 2 - 0x6ce0000) //  used, but
#define ADDR_EXPeNEG  (serverDll + 0x6d04f18 + 2 - 0x6ce0000) //  may as well
#define ADDR_EXP3NEG  (serverDll + 0x6cf38fb + 2 - 0x6ce0000) // mine
#define ADDR_EXP1NEG  (serverDll + 0x6cf44f2 + 2 - 0x6ce0000) // missile
#define ADDR_EXP2NEG  (serverDll + 0x6d0b3d2 + 2 - 0x6ce0000)

#define ADDR_ZONEMIN  (serverDll + 0x6d0120a - 0x6ce0000)
#define ADDR_ZONEABS  (serverDll + 0x6d012e3 - 0x6ce0000)

// Use absolute value to apply shield damage.
#define ADDR_SHIELD   (serverDll + 0x6ce94b4 - 0x6ce0000)
#define ADDR_EXPSHLD  (serverDll + 0x6ce9bbe - 0x6ce0000)

// Change the client to display positive shield damage.
#define ADDR_DISP1    ((PBYTE)0x48423e)
#define ADDR_DISP2    ((PBYTE)0x48464b)
#define ADDR_DISP3    ((PBYTE)0x484b85)

__declspec(naked) void EquipHook()
{
    __asm {
	    test	ah, 0x41 // health <= 0
	    jnz	zero
	    mov	ecx, [ebp+0x0C] // CEquip::GetMaxHitPoints
	    fcom	dword ptr [ecx+0x1C]
	    fnstsw	ax
	    test	ah, 0x41 // health <= max
	    jnz	done // yes
	    fstp	st
	    fld	dword ptr [ecx+0x1C]
      done:
	    ret
      zero:
	    fstp	st
	    fldz
	    ret
    }
}

__declspec(naked) void GroupHook()
{
    __asm {
	    test	ah, 0x41 // health <= 0
	    jnz	zero
	    mov	ecx, [ebp+0x04] // CArchGroup::GetMaxHitPoints
	    fild	dword ptr [ecx+0x10]
	    fcom	st(1)
	    fnstsw	ax
	    test	ah, 1 // max < health
	    jz	done // no
	    fxch // discard health, keep max
      done:
	    fstp	st
	    ret
      zero:
	    fstp	st
	    fldz
	    ret
    }
}

__declspec(naked) void HullHook()
{
    __asm {
	test	ah, 0x41 // health <= 0
	jnz	zero
	mov	ecx, [esi+0x10]
	mov	ecx, [ecx+0x88] // CSimple::get_max_hit_pts
	fcom	dword ptr [ecx+0x1C]
	fnstsw	ax
	test	ah, 0x41 // health <= max
	jnz	done // yes
	fstp	st
	fld	dword ptr [ecx+0x1C]
  done:
	ret
  zero:
	fstp	st
	fldz
	ret
    }
}

__declspec(naked) void MinDamageHook()
{
    __asm {
	    pop	ecx
	    fabs
	    mov	eax, [ecx-9]
	    fcomp	dword ptr [eax]
	    fnstsw	ax
	    test	ah, 0x41 // -0.01 <= damage <= 0.01
	    jmp	ecx
    }
}

void Fixes::AllowNegativeDamage()
{
    serverDll = reinterpret_cast<PBYTE>(GetModuleHandleA("server.dll"));
    if (!serverDll)
    {
        return;
    }

    MemUtils::Protect(ADDR_DMGLST, 1);
    MemUtils::Protect(ADDR_NEGDMG1, 1);
    MemUtils::Protect(ADDR_NEGDMG2, 1);
    MemUtils::Protect(ADDR_EXPaNEG, 1);
    MemUtils::Protect(ADDR_EXPbNEG, 1);
    MemUtils::Protect(ADDR_SHIELD, 0x18);
    MemUtils::Protect( ADDR_EXPSHLD-14, 1 );
    MemUtils::Protect( ADDR_EXPSHLD-3,  1 );
    MemUtils::Protect( ADDR_EXPSHLD, 31 );
    MemUtils::Protect(ADDR_ARMORNEG, 1);
    MemUtils::Protect( ADDR_EQUIPNEG, 1 );
    MemUtils::Protect( ADDR_MAXEQUIP, 5 );
    MemUtils::Protect( ADDR_CGRPNEG,  1 );
    MemUtils::Protect( ADDR_MAXCGRP,  5 );
    MemUtils::Protect( ADDR_MAXHULL,  5 );
    MemUtils::Protect(ADDR_HULLNEG, 1);
    MemUtils::Protect(ADDR_EXPcNEG, 1);
    MemUtils::Protect(ADDR_EXPdNEG, 1);
    MemUtils::Protect(ADDR_EXP3NEG, 1);
    MemUtils::Protect(ADDR_EXP1NEG, 1);
    MemUtils::Protect(ADDR_ZONENEG, 1);
    MemUtils::Protect( ADDR_ZONEMIN-5,1 );
    MemUtils::Protect( ADDR_ZONEMIN,  5 );
    MemUtils::Protect( ADDR_ZONEABS, 11 );
    MemUtils::Protect(ADDR_EXPeNEG, 1);
    MemUtils::Protect(ADDR_EXP2NEG, 1);

    *ADDR_DMGLST = 0x93; // skip new_damage > current_damage test

    // *ADDR_NEGDMG1  =			// uncomment to show radiation alert
    *ADDR_NEGDMG2 = *ADDR_ARMORNEG = *ADDR_EQUIPNEG = *ADDR_CGRPNEG = *ADDR_HULLNEG = *ADDR_EXP1NEG = // these explosions should really be
        *ADDR_EXP2NEG =                                                                               //  testing abs(damage) <= 0.0001
        *ADDR_EXP3NEG = *ADDR_EXPaNEG = *ADDR_EXPbNEG = *ADDR_EXPcNEG = *ADDR_EXPdNEG = *ADDR_EXPeNEG = *ADDR_ZONENEG = 0x40; // <= 0 --> != 0

    MemUtils::NopAddress(DWORD(serverDll) + 0xA7C2, 6); // Remove negative shield damage clamp

    ADDR_ZONEMIN[-5] = 0x15; // fcomp --> fcom
    MemUtils::PatchAssembly(reinterpret_cast<DWORD>(ADDR_ZONEMIN), MinDamageHook, true);

    memcpy(ADDR_ZONEABS,
           "\xd9\xe8"         // fld1
           "\xD9\x44\x24\x24" // fld  dword[esp+24]
           "\xd9\xe1"         // fabs
           "\xde\xd9"         // fcompp
           "\x53",
           11); // push ebx

    MemUtils::PatchAssembly(reinterpret_cast<DWORD>(ADDR_MAXEQUIP), EquipHook, true);
    MemUtils::PatchAssembly(reinterpret_cast<DWORD>(ADDR_MAXCGRP), GroupHook, true);
    MemUtils::PatchAssembly(reinterpret_cast<DWORD>(ADDR_MAXHULL), HullHook, true);

    memcpy(ADDR_SHIELD,
           "\x40"         // inc eax
           "\xd9\x40\x7f" // fld dword[eax+7f]
           "\xd9\xe1",
           6);           // fabs
    --ADDR_SHIELD[0x17]; // compensate for the inc

    ADDR_EXPSHLD[-14] = 0x24;
    ADDR_EXPSHLD[-3] = 0x0A;
    memcpy(ADDR_EXPSHLD,
           "\x43" // inc ebx
           "\xeb\x10"
           "\x48\x02\x00\x00\x00"
           "\xD8\x5C\x24\x1C"
           "\xDF\xE0"
           "\xF6\xC4\x05"
           "\x7A\x4b"
           "\x89\x5C\x24\x48"
           "\x8B\x47\x18"
           "\xD9\x40\x0C"
           "\xd9\xe1",
           31); // fabs

    if (Fluf::IsRunningOnClient())
    {
        MemUtils::Protect(ADDR_DISP1, 10);
        MemUtils::Protect(ADDR_DISP2, 8);
        MemUtils::Protect(ADDR_DISP3, 9);

        memcpy(ADDR_DISP1,
               "\xd9\xe1"         // fabs
               "\xd8\x44\x24\x14" // fadd  dword[esp+14]
               "\x50"             // push  eax
               "\xdb\x1c\xe4",
               10); // fistp dword[esp]

        memcpy(ADDR_DISP2,
               "\xd9\xe1" // fabs
               "\xde\xc1" // faddp st(1), st(0)
               "\x50"     // push  eax
               "\xdb\x1c\xe4",
               8); // fistp dword[esp]

        memcpy(ADDR_DISP3,
               "\xd9\xe1"     // fabs
               "\xd8\x41\x10" // fadd  dword[ecx+10]
               "\x50"         // push  eax
               "\xdb\x1c\xe4",
               9); // fistp dword[esp]
    }
}
