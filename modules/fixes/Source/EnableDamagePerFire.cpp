#include "PCH.hpp"

#include "Fixes.hpp"
#include "Utils/MemUtils.hpp"

struct Server
{
        BYTE stuff[12];
        UINT ship; // a pointer to the CShip

        virtual void fn000();
        virtual void fn004();
        virtual void fn008();
        virtual void fn00C();
        virtual void fn010();
        virtual void fn014();
        virtual void fn018();
        virtual void fn01C();
        virtual void fn020();
        virtual void fn024();
        virtual void fn028();
        virtual void fn02C();
        virtual void fn030();
        virtual void fn034();
        virtual void fn038();
        virtual void fn03C();
        virtual void fn040();
        virtual void fn044();
        virtual void fn048();
        virtual void fn04C();
        virtual void fn050();
        virtual void fn054();
        virtual void fn058();
        virtual void fn05C();
        virtual void fn060();
        virtual void fn064();
        virtual void fn068();
        virtual void fn06C();
        virtual void fn070();
        virtual void fn074();
        virtual void fn078();
        virtual void fn07C();
        virtual void fn080();
        virtual void fn084();
        virtual void fn088();
        virtual void fn08C();
        virtual void fn090();
        virtual void fn094();
        virtual void fn098();
        virtual void fn09C();
        virtual void fn0A0();
        virtual void fn0A4();
        virtual void fn0A8();
        virtual void fn0AC();
        virtual void fn0B0();
        virtual void fn0B4();
        virtual void fn0B8();
        virtual void fn0BC();
        virtual void fn0C0();
        virtual void fn0C4();
        virtual void fn0C8();
        virtual void fn0CC();
        virtual void fn0D0();
        virtual void fn0D4();
        virtual void fn0D8();
        virtual void fn0DC();
        virtual void fn0E0();
        virtual void fn0E4();
        virtual void fn0E8();
        virtual void fn0EC();
        virtual void fn0F0();
        virtual void fn0F4();
        virtual void fn0F8();
        virtual void fn0FC();
        virtual void fn100();
        virtual void fn104();
        virtual void fn108();
        virtual void fn10C();
        virtual void fn110();
        virtual void fn114();
        virtual void fn118();
        virtual void fn11C();
        virtual void fn120();
        virtual void fn124();
        virtual void fn128();
        virtual void fn12C();
        virtual void fn130();
        virtual void fn134();
        virtual void fn138();
        virtual void fn13C();
        virtual void fn140();
        virtual void fn144();
        virtual void fn148();
        virtual void fn14C();
        virtual void fn150();
        virtual void fn154();
        virtual void fn158();
        virtual void fn15C();
        virtual void fn160();
        virtual void fn164();
        virtual void fn168();
        virtual void fn16C();
        virtual void fn170();
        virtual void fn174();
        virtual void fn178();
        virtual void fn17C();
        virtual void fn180();
        virtual void fn184();
        virtual void fn188(DamageList&);
};

std::multimap<UINT, DamageEntry> damageMap;

void __stdcall StoreDamage(const PBYTE equip, const float damagePerFire, const float hitPts)
{
    DamageEntry de;

    UINT ship = *reinterpret_cast<UINT*>(equip + 4);
    de.subObj = *reinterpret_cast<USHORT*>(equip + 8);
    de.health = *reinterpret_cast<float*>(equip + 0x30);

    float min = 0;
    if (const float dmg = fabsf(damagePerFire); dmg <= 1)
    {
        de.health -= de.health * dmg;
        min = 1;
    }
    else if (dmg < 2)
    {
        de.health -= hitPts / ((dmg - 1) * 100 + 1);
    }
    else
    {
        de.health -= dmg;
    }
    if (de.health < min)
    {
        de.health = 0;
    }
    
    de.fate = de.health != 0 ? DamageEntry::SubObjFate::Alive : damagePerFire < 0 ? DamageEntry::SubObjFate::Disappear : DamageEntry::SubObjFate::Debris;
    damageMap.insert(std::pair(ship, de));
}

__declspec(naked) void FireHook()
{
    __asm {
        mov	edx, [edi+12] // Archetype
        mov	eax, [edx+0x88] // damage_per_fire
        test	eax, eax
        jz	done
        push	dword ptr [edx+0x1c] // hit_pts
        push	eax
        push	edi
        call	StoreDamage
      done:
        pop	edi
        pop	esi
        ret
    }
}

void __stdcall AddDamage(Server* data)
{
    auto iter = damageMap.lower_bound(data->ship);
    if (iter == damageMap.end())
    {
        return;
    }

    DamageList dmg;
    for (const auto end = damageMap.upper_bound(data->ship); iter != end; ++iter)
    {
        dmg.add_damage_entry(iter->second.subObj, iter->second.health, iter->second.fate);
    }

    data->fn188(dmg);
    damageMap.erase(data->ship);
}

DWORD UpdateOrg;
__declspec(naked) void UpdateHook()
{
    __asm {
        push	ecx
        call	AddDamage
        mov	ecx, esi
        jmp	UpdateOrg
    }
}

// CELauncher::ConsumeFireResources
#define ADDR_FIRE   ((PBYTE)0x6299623)

// Some sort of server update.
#define ADDR_UPDATE (server + 0x6d014bf + 1 - 0x6ce0000)

void Fixes::EnableDamagePerFire()
{
    auto server = reinterpret_cast<PBYTE>(GetModuleHandleA("server.dll"));

    // Make sure the patch hasn't already been applied, since something like
    // Console keeps server.dll loaded.
    if (server == nullptr || *reinterpret_cast<PDWORD>(ADDR_UPDATE) != 0xFFFFFC8C)
    {
        return;
    }

    MemUtils::Protect(ADDR_UPDATE, 4);
    MemUtils::Protect(ADDR_FIRE, 5);
    MemUtils::PatchAssembly(reinterpret_cast<DWORD>(ADDR_FIRE), FireHook);

    #define RELOFS(from, to) *(PDWORD)((DWORD)(from)) = (DWORD)(to) - (DWORD)(from) - 4

    #define NEWOFS(from, to, prev)                    \
        prev = (DWORD)(from) + *((PDWORD)(from)) + 4; \
        RELOFS(from, to)

    NEWOFS(ADDR_UPDATE, UpdateHook, UpdateOrg);
}
