#include "PCH.hpp"

#include "Fixes.hpp"
#include "Utils/MemUtils.hpp"

#define ADDR_FEELINGS    ((PDWORD)0x5c63bc)
#define ADDR_STATS       ((PDWORD)(0x53c3e1 + 1))
#define ADDR_ENTER       ((PDWORD)(0x558986 + 1))

#define ADDR_LEVEL1      ((PBYTE)0x47fcdf) // equipment/goods
#define ADDR_LEVEL2      ((PBYTE)0x480499 + 1)
#define ADDR_LEVEL3      ((PBYTE)0x482e95 + 1)
#define ADDR_LEVEL4      ((PBYTE)0x4b850d + 1) // ships
#define ADDR_LEVEL5      ((PBYTE)0x4b948d + 1)

#define ADDR_SHIP1       ((PBYTE)0x4b946b + 1)
#define ADDR_SHIP2       ((PBYTE)0x4b9472 + 1)
#define ADDR_SHIP3       ((PBYTE)0x4b9495 + 1)
#define ADDR_SHIP4       ((PBYTE)0x4b94aa)
#define ADDR_SHIP5       ((PBYTE)0x4b94b6 + 2)

#define RELOFS(from, to) *(PDWORD)((DWORD)(from)) = (DWORD)(to) - (DWORD)(from) - 4

#define NEWOFS(from, to, prev)                    \
    prev = (DWORD)(from) + *((PDWORD)(from)) + 4; \
    RELOFS(from, to)

DWORD getGroupFeelingsTowardsOrg;
DWORD receivePlayerStatsOrg;
DWORD baseEnterOrg;

struct Rep
{
        UINT faction;
        float reputation;
};

Rep* vibe;
int vibeCount;

DWORD last;
bool flag;
bool* singlePlayerPtr;

__declspec(naked) void RequestPlayerStats()
{
    __asm {
	    mov	flag, 1
	    mov	ecx, ds:[0x67ecd0]
	    push	0
	    mov	eax, esp
	    push	4
	    push	eax
	    mov	eax, [ecx]
	    push	ds:[0x673344]
	    call	dword ptr [eax+0x124]

	    mov	dword ptr [esp], 0
	    call	dword ptr [time]
	    add	esp, 4
	    mov	last, eax
	    ret
    }
}

void __stdcall ReceivePlayerStats(const Rep* v, const int len)
{
    if (len != vibeCount)
    {
        delete[] vibe;
        vibe = new Rep[vibeCount = len];
    }
    memcpy(vibe, v, len * sizeof(Rep));

    last = time(nullptr);
}

__declspec(naked) void ReceivePlayerStatsHook()
{
    __asm {
	    push	[ecx-4]
	    push	edx
	    call	ReceivePlayerStats
	    cmp	flag, 0
	    je	done
	    mov	flag, 0
	    ret
      done:
	    jmp	receivePlayerStatsOrg
	    align	16
    }
}

__declspec(naked) void BaseEnterHook()
{
    __asm {
	    push	ecx
	    call	RequestPlayerStats
	    pop	ecx
	    jmp	baseEnterOrg
	    align	16
    }
}

int FactionCompare(const void* key, const void* elem) { return reinterpret_cast<UINT>(key) - static_cast<const Rep*>(elem)->faction; }

int GetGroupFeelingsTowards(const int&, const UINT& faction, float& rep)
{
    if (time(nullptr) >= last + 10)
    {
        RequestPlayerStats();
        // Due to the nature of this method, this first call misses out.
        // Just use the previous value, or neutral for the first time.
        // It does mean you could buy a ship that perhaps you shouldn't, but I
        // expect the requirement would be for a positive rep, so I'll let it be.
    }

    const auto v = static_cast<Rep*>(bsearch(reinterpret_cast<void*>(faction), vibe, vibeCount, sizeof(Rep), FactionCompare));
    rep = v == nullptr ? 0 : v->reputation;
    return 0;
}

__declspec(naked) void GetGroupFeelingsTowards_Hook()
{
    __asm {
	    mov	eax, SinglePlayer
	    cmp	byte ptr [eax], 0
	    je	mp
	    jmp	getGroupFeelingsTowardsOrg
      mp:	jmp	GetGroupFeelingsTowards
	    align	16
    }
}

void Fixes::MultiplayerReputationFixes()
{
    MemUtils::Protect(ADDR_FEELINGS, 4);
    MemUtils::Protect(ADDR_STATS, 4);
    MemUtils::Protect(ADDR_ENTER, 4);

    MemUtils::Protect(ADDR_LEVEL1, 7);
    MemUtils::Protect(ADDR_LEVEL2, 1);
    MemUtils::Protect(ADDR_LEVEL3, 1);
    MemUtils::Protect(ADDR_LEVEL4, 1);
    MemUtils::Protect(ADDR_LEVEL5, 1);

    getGroupFeelingsTowardsOrg = *ADDR_FEELINGS;
    *ADDR_FEELINGS = reinterpret_cast<DWORD>(GetGroupFeelingsTowards_Hook);
    NEWOFS(ADDR_STATS, ReceivePlayerStatsHook, receivePlayerStatsOrg);
    NEWOFS(ADDR_ENTER, BaseEnterHook, baseEnterOrg);

    memcpy(ADDR_LEVEL1,
           "\xe8\x0c\x59\x04\x00" // do both the SP
           "\xeb\x13",
           7);                                                     // and MP code
    *ADDR_LEVEL2 = *ADDR_LEVEL3 = *ADDR_LEVEL4 = *ADDR_LEVEL5 = 0; // ignore MP test

    // Select the right ship when reputation changes order.
    *ADDR_SHIP1 = 0x3f;
    *ADDR_SHIP2 = 0x38;
    *ADDR_SHIP3 += 2;
    *ADDR_SHIP5 -= 4;
    memmove(ADDR_SHIP4 + 4, ADDR_SHIP4, 0x12);
    memcpy(ADDR_SHIP4, "\xeb\x02\xdd\xd8", 4); // jmp $+2; fstp st

    singlePlayerPtr = reinterpret_cast<bool*>(reinterpret_cast<DWORD>(GetModuleHandleA("common.dll")) - 0x6260000 + 0x63ed17c);
}
