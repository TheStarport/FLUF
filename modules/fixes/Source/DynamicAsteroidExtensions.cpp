#include "PCH.hpp"

#include "Fixes.hpp"
#include "Utils/MemUtils.hpp"

#define ADDR_HIT     ((PBYTE)0x5254d2)
#define ADDR_CREATE  ((PDWORD)(0x525eec + 1))
#define ADDR_RESET   ((PBYTE)0x52610e)
#define ADDR_DESTROY ((PDWORD)0x5dce38)
#define ADDR_DEFAULT ((PBYTE)0x62fa1e4)

#define ADDR_MASS    ((PDWORD)(0x62a2b2e + 1))

struct DynAstHp
{
        float hitPts;
        float health;
};

std::unordered_map<LPVOID, DynAstHp> dynamicAsteroids;

DWORD create, destroy;

void __stdcall Create(const LPVOID dynast, const float hitPts)
{
    DynAstHp da;

    da.hitPts = hitPts; // health initialised on reset

    // ReSharper disable once CppSomeObjectMembersMightNotBeInitialized
    dynamicAsteroids[dynast] = da;
}

void __stdcall Destroy(const LPVOID dynast) { dynamicAsteroids.erase(dynast); }

void __stdcall Reset(const LPVOID dynast)
{
    if (const auto iter = dynamicAsteroids.find(dynast); iter != dynamicAsteroids.end())
    {
        float var = iter->second.hitPts * 0.1f; // 10% variance
        var = 2 * var * rand() / RAND_MAX - var;
        iter->second.health = iter->second.hitPts + var;
    }
}

bool __stdcall Hit(const LPVOID dynast, const float dmg)
{
    const auto asteroid = dynamicAsteroids.find(dynast);
    if (asteroid == dynamicAsteroids.end())
    {
        return false;
    }

    asteroid->second.health -= dmg;
    return (asteroid->second.health > 0);
}

__declspec(naked) void CreateHook()
{
    __asm {
        call	create
        test	eax, eax
        jz	done
        push	eax

        mov	ecx, [eax+0x10]
        mov	ecx, [ecx+0x88] // archetype
        push	[ecx+0x1c] // hit_pts
        push	eax
        call	Create

        pop	eax
      done:
        ret
    }
}

__declspec(naked) void DestroyHook()
{
    _asm {
        push	ecx

        push	ecx
        call	Destroy

        pop	ecx
        jmp	destroy
    }
}

__declspec(naked) void ResetHook()
{
    __asm {
        mov	edx, [esi+12]
        mov	eax, [edx+edi*4]
        push	eax

        push	eax
        call	Reset

        pop	eax
        ret
    }
}

__declspec(naked) void HitHook()
{
    __asm {
        and	edx, 7
        cmp	dl, 7
        jne	done

        mov	ecx, [ecx+0x10]
        mov	ecx, [ecx+0x88]
        push	[ecx+0x80] // hull_damage of the hitting weapon
        push	edi
        call	Hit
        test	al, al
      done:
        ret
    }
}

// mass

float __stdcall AdjustMass(const float mass)
{
    float var = mass * 0.1f; // 10% variance
    var = 2 * var * rand() / RAND_MAX - var;
    return mass + var;
}

DWORD BuildIVP;

__declspec(naked) void MassHook()
{
    __asm {
        mov	eax, [esi+0x88]
        mov	eax, [eax+0x20] // mass
        push	eax
        call	AdjustMass
        fstp	dword ptr [esp+0x18+4] // reset for CreateParms
        jmp	BuildIVP
    }
}

void Fixes::EnableDynamicAsteroidExtensions()
{
    MemUtils::Protect(ADDR_HIT, 6);
    MemUtils::Protect(ADDR_CREATE, 4);
    MemUtils::Protect(ADDR_RESET, 6);
    MemUtils::Protect(ADDR_DESTROY, 4);
    MemUtils::Protect(ADDR_DEFAULT, 7);
    MemUtils::Protect(ADDR_MASS, 4);

#define ABSOFS(from) ((DWORD)(from) + *((PDWORD)(from)) + 4)

    create = ABSOFS(ADDR_CREATE);
    RELOFS(ADDR_CREATE, CreateHook);

    destroy = *ADDR_DESTROY;
    *ADDR_DESTROY = (DWORD)DestroyHook;

    MemUtils::PatchAssembly(reinterpret_cast<DWORD>(ADDR_HIT), HitHook, true);
    MemUtils::PatchAssembly(reinterpret_cast<DWORD>(ADDR_RESET), ResetHook, true);

    ADDR_HIT[5] = ADDR_RESET[5] = 0x90;

    // mov [esi+13], 40	  ; [esi+10] set to 0, so this is enough for 2.0f
    // mov [esi+1c], ebp	  ; hit points initialised to 100, set to 0
    memcpy(ADDR_DEFAULT, "\xC6\x46\x13\x40\x89\x6E\x1C", 7);

    BuildIVP = ABSOFS(ADDR_MASS);
    RELOFS(ADDR_MASS, MassHook);
}
