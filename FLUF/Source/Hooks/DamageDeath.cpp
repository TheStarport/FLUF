#include "PCH.hpp"

#include "Fluf.hpp"
#include "DamageDeath.hpp"
#include <xbyak/xbyak.h>
#include <magic_enum.hpp>
#include <FLCore/Server.hpp>

using namespace magic_enum::bitwise_operators;

void __fastcall IEngineHook::ShipDestroy(Ship* ship, DamageList* dmgList, DestroyType destroyType, Id killerId)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipDestroy, ship, dmgList, destroyType, killerId);

    using IShipDestroyType = void(__thiscall*)(Ship*, DestroyType, Id);
    static_cast<IShipDestroyType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::ObjectDestroyed)))(ship, destroyType, killerId);
}

void __fastcall IEngineHook::LootDestroy(Loot* loot, void* edx, DestroyType destroyType, Id killerId)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeLootDestroy, loot, destroyType, killerId);

    using ILootDestroyType = void(__thiscall*)(Loot*, DestroyType, Id);
    static_cast<ILootDestroyType>(iLootVTable.GetOriginal(static_cast<ushort>(ILootInspectVTable::ObjectDestroyed)))(loot, destroyType, killerId);
}

void __fastcall IEngineHook::SolarDestroy(Solar* solar, void* edx, DestroyType destroyType, Id killerId)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeSolarDestroy, solar, destroyType, killerId);

    using ISolarDestroyType = void(__thiscall*)(Solar*, DestroyType, Id);
    static_cast<ISolarDestroyType>(iSolarVTable.GetOriginal(static_cast<ushort>(ISolarInspectVTable::ObjectDestroyed)))(solar, destroyType, killerId);
}

void __fastcall IEngineHook::MineDestroy(Mine* mine, void* edx, DestroyType destroyType, Id killerId)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeMineDestroy, mine, destroyType, killerId);

    using IMineDestroyType = void(__thiscall*)(Mine*, DestroyType, Id);
    static_cast<IMineDestroyType>(iMineVTable.GetOriginal(static_cast<ushort>(IMineInspectVTable::ObjectDestroyed)))(mine, destroyType, killerId);
}

void __fastcall IEngineHook::GuidedDestroy(Guided* guided, void* edx, DestroyType destroyType, Id killerId)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeGuidedDestroy, guided, destroyType, killerId);

    using IGuidedDestroyType = void(__thiscall*)(Guided*, DestroyType, Id);
    static_cast<IGuidedDestroyType>(iGuidedVTable.GetOriginal(static_cast<ushort>(IGuidedInspectVTable::ObjectDestroyed)))(guided, destroyType, killerId);
}

void __fastcall IEngineHook::ShipMunitionHit(Ship* ship, void* edx, MunitionImpactData* impact, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipMunitionHit, ship, impact, dmgList);

    using IShipMunitionImpactType = void(__thiscall*)(Ship*, MunitionImpactData*, DamageList*);
    static_cast<IShipMunitionImpactType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::MunitionImpact)))(ship, impact, dmgList);

    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipMunitionHitAfter, ship, impact, dmgList);
}

void __fastcall IEngineHook::ShipExplosionHit(Ship* ship, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipExplosionHit, ship, explosion, dmgList);

    using IShipExplosionHitType = void(__thiscall*)(Ship*, ExplosionDamageEvent*, DamageList*);
    static_cast<IShipExplosionHitType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::ProcessExplosionDamage)))(ship, explosion, dmgList);
}

void __fastcall IEngineHook::ShipShieldDmg(Ship* ship, void* edx, CEShield* shield, float incDmg, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipShieldDmg, ship, shield, incDmg, dmgList);

    using IShipShieldDmgType = void(__thiscall*)(Ship*, CEShield*, float, DamageList*);
    static_cast<IShipShieldDmgType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::DamageShield)))(ship, shield, incDmg, dmgList);
}

void __fastcall IEngineHook::ShipEnergyDmg(Ship* ship, void* edx, float incDmg, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipEnergyDmg, ship, incDmg, dmgList);

    using IShipEnergyDmgType = void(__thiscall*)(Ship*, float, DamageList*);
    static_cast<IShipEnergyDmgType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::DamageEnergy)))(ship, incDmg, dmgList);
}

void __fastcall IEngineHook::GuidedExplosionHit(Guided* guided, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeGuidedExplosionHit, guided, explosion, dmgList);

    using IGuidedExplosionHitType = void(__thiscall*)(Guided*, ExplosionDamageEvent*, DamageList*);
    static_cast<IGuidedExplosionHitType>(iGuidedVTable.GetOriginal(static_cast<ushort>(IGuidedInspectVTable::ProcessExplosionDamage)))(
        guided, explosion, dmgList);
}

void __fastcall IEngineHook::SolarExplosionHit(Solar* solar, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeSolarExplosionHit, solar, explosion, dmgList);

    using ISolarExplosionHitType = void(__thiscall*)(Solar*, ExplosionDamageEvent*, DamageList*);
    static_cast<ISolarExplosionHitType>(iGuidedVTable.GetOriginal(static_cast<ushort>(ISolarInspectVTable::ProcessExplosionDamage)))(solar, explosion, dmgList);
}

void __fastcall IEngineHook::ShipFuse(Ship* ship, void* edx, uint fuseCause, uint& fuseId, ushort sId, float radius, float lifetime)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipFuse, ship, fuseCause, fuseId, sId, radius, lifetime);

    using IShipFuseType = void(__thiscall*)(Ship*, uint, uint&, ushort, float, float);
    static_cast<IShipFuseType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::LightFuse)))(ship, fuseCause, fuseId, sId, radius, lifetime);
}

void __fastcall IEngineHook::ShipHullDamage(Ship* ship, void* edx, float damage, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipHullDamage, ship, damage, dmgList);

    using IShipHullDmgType = void(__thiscall*)(Ship*, float, DamageList*);
    static_cast<IShipHullDmgType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::DamageHull)))(ship, damage, dmgList);
}

void __fastcall IEngineHook::SolarHullDamage(Solar* solar, void* edx, float damage, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeSolarHullDamage, solar, damage, dmgList);

    using ISolarHullDmgType = void(__thiscall*)(Solar*, float, DamageList*);
    static_cast<ISolarHullDmgType>(iSolarVTable.GetOriginal(static_cast<ushort>(ISolarInspectVTable::DamageHull)))(solar, damage, dmgList);
}

void __fastcall IEngineHook::SolarColGrpDestroy(Solar* solar, void* edx, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList, bool killParent)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeSolarColGrpDestroy, solar, colGrp, fate, dmgList, killParent);
    using ISolarColGrpDestroyType = void(__thiscall*)(Solar*, CArchGroup*, DamageEntry::SubObjFate, DamageList*, bool);
    static_cast<ISolarColGrpDestroyType>(iSolarVTable.GetOriginal(static_cast<ushort>(ISolarInspectVTable::ColGrpDeath)))(
        solar, colGrp, fate, dmgList, killParent);
}

void __fastcall IEngineHook::ShipEquipDmg(Ship* ship, void* edx, CAttachedEquip* equip, float damage, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipEquipDmg, ship, equip, damage, dmgList);

    using IShipEquipDmgType = void(__thiscall*)(Ship*, CAttachedEquip*, float, DamageList*);
    static_cast<IShipEquipDmgType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::DamageExtEq)))(ship, equip, damage, dmgList);
}

void __fastcall IEngineHook::ShipEquipDestroy(Ship* ship, void* edx, CEquip* equip, DamageEntry::SubObjFate fate, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipEquipDestroy, ship, equip, fate, dmgList);

    using IShipEquipDestroyType = void(__thiscall*)(Ship*, CEquip*, DamageEntry::SubObjFate, DamageList*);
    static_cast<IShipEquipDestroyType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::CEquipDeath)))(ship, equip, fate, dmgList);
}

void __fastcall IEngineHook::ShipColGrpDmg(Ship* ship, void* edx, CArchGroup* colGrp, float damage, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipColGrpDmg, ship, colGrp, damage, dmgList);

    using IShipColGrpDmgType = void(__thiscall*)(Ship*, CArchGroup*, float, DamageList*);
    static_cast<IShipColGrpDmgType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::DamageColGrp)))(ship, colGrp, damage, dmgList);
}

void __fastcall IEngineHook::ShipColGrpDestroy(Ship* ship, void* edx, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList,
                                               const bool killLinkedElements)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipColGrpDestroy, ship, colGrp, fate, dmgList, killLinkedElements);

    using IShipColGrpDmgType = void(__thiscall*)(Ship*, CArchGroup*, DamageEntry::SubObjFate, DamageList*, bool);
    static_cast<IShipColGrpDmgType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::ColGrpDeath)))(
        ship, colGrp, fate, dmgList, killLinkedElements);
}

void __fastcall IEngineHook::ShipDropAllCargo(Ship* ship, void* edx, const char* hardPoint, DamageList* dmgList)
{
    Fluf::instance->CallModuleEvent(&FlufModule::BeforeShipDropAllCargo, ship, hardPoint, dmgList);

    using IShipColGrpDmgType = void(__thiscall*)(Ship*, const char*, DamageList*);
    static_cast<IShipColGrpDmgType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::DropAllCargo)))(ship, hardPoint, dmgList);
}

void __fastcall IEngineHook::CShipInit(CShip* ship, void* edx, CShip::CreateParms* creationParams)
{
    using CShipInitType = void(__thiscall*)(CShip*, CShip::CreateParms*);
    static_cast<CShipInitType>(cShipVTable.GetOriginal(static_cast<ushort>(CShipVTable::InitCShip)))(ship, creationParams);

    Fluf::instance->CallModuleEvent(&FlufModule::OnCShipInit, ship);
}

void __thiscall IEngineHook::CSolarInit(CSolar* solar, CSolar::CreateParms* creationParams)
{
    Fluf::instance->solarInitDetour.UnDetour();
    Fluf::instance->solarInitDetour.GetOriginalFunc()(solar, creationParams);
    Fluf::instance->solarInitDetour.Detour(IEngineHook::CSolarInit);

    Fluf::instance->CallModuleEvent(&FlufModule::OnCSolarInit, solar);
}

void __fastcall IEngineHook::CLootInit(CLoot* loot, void* edx, CLoot::CreateParms* creationParams)
{
    using CLootInitType = void(__thiscall*)(CLoot*, CLoot::CreateParms*);
    static_cast<CLootInitType>(cLootVTable.GetOriginal(static_cast<ushort>(CLootVTable::InitCLoot)))(loot, creationParams);

    Fluf::instance->CallModuleEvent(&FlufModule::OnCLootInit, loot);
}

void __fastcall IEngineHook::CGuidedInit(CGuided* guided, void* edx, CGuided::CreateParms* creationParams)
{
    using CGuidedInitType = void(__thiscall*)(CGuided*, CGuided::CreateParms*);
    static_cast<CGuidedInitType>(cGuidedVTable.GetOriginal(static_cast<ushort>(CGuidedVTable::InitCEquipObject)))(guided, creationParams);

    Fluf::instance->CallModuleEvent(&FlufModule::OnCGuidedInit, guided);
}

void __fastcall IEngineHook::ShipRadiationDamage(Ship* ship, void* edx, float deltaTime, DamageList* dmgList)
{
    if (ship->cship()->hitPoints <= 0.0f || !ship->cship()->currentDamageZone)
    {
        return;
    }

    auto zoneDataIter = zoneSpecialData.find(ship->cship()->currentDamageZone->zoneId);
    if (zoneDataIter == zoneSpecialData.end())
    {
        return;
    }
    const ZoneSpecialData& zd = zoneDataIter->second;
    uint zoneType = zd.dmgType;

    CShip* cship = ship->cship();

    if (zoneType & ZONEDMG_CRUISE)
    {
        dmgList->damageCause = DamageCause::CruiseDisrupter;
        dmgList->add_damage_entry(1, cship->hitPoints, DamageEntry::SubObjFate(0));
        zoneType -= ZONEDMG_CRUISE;
        if (!zoneType)
        {
            return;
        }
    }

    float damage = zd.flatDamage;

    if (damage <= 0.0f)
    {
        return;
    }

    float dmgMultiplier = 1.0f;

    using IZoneGetDistanceFunc = float(__thiscall*)(Universe::IZone*, Vector & pos);
    IZoneGetDistanceFunc GetZoneDistance = (IZoneGetDistanceFunc)(DWORD(GetModuleHandleA("common")) + 0xD9B00);

    if (zd.distanceScaling != 0.0f)
    {
        float edgeDistance = -GetZoneDistance(cship->currentDamageZone, cship->position);
        if (edgeDistance < 0.0f)
        {
            return;
        }
        if (zd.distanceScaling > 0.0f)
        {
            if (edgeDistance <= zd.distanceScaling)
            {
                dmgMultiplier = powf(edgeDistance / zd.distanceScaling, zd.logScale);
            }
        }
        else
        {
            if (edgeDistance <= -zd.distanceScaling)
            {
                dmgMultiplier = powf(1.0f - (edgeDistance / -zd.distanceScaling), zd.logScale);
            }
            else
            {
                return;
            }
        }
    }

    dmgMultiplier *= ship->pendingEnvironmentalDamage; // assembly hacked to instead store time spent in the zone.

    if (zoneType & ZONEDMG_SHIELD)
    {
        CEShield* shield = reinterpret_cast<CEShield*>(cship->equipManager.FindFirst((uint)EquipmentClass::Shield));
        if (shield)
        {
            float shielddamage = (damage + zd.percentageDamage * shield->maxShieldHitPoints) * dmgMultiplier * zd.shieldMult;
            ship->damage_shield_direct(shield, shielddamage, dmgList);
            zoneType -= ZONEDMG_SHIELD;
            if (!zoneType)
            {
                return;
            }
        }
    }

    if (zoneType & ZONEDMG_ENERGY)
    {
        float energydamage = (damage + zd.percentageDamage * cship->maxPower) * dmgMultiplier * zd.energyMult;
        ship->damage_energy(energydamage, dmgList);
        zoneType -= ZONEDMG_ENERGY;
        if (!zoneType)
        {
            return;
        }
    }

    CArchGroupManager& carchMan = cship->archGroupManager;
    CArchGrpTraverser tr2;

    CArchGroup* carch = nullptr;
    uint colGrpCount = 1;
    while (carch = carchMan.Traverse(tr2))
    {
        if (carch->colGrp->hitPts < 100.f)
        {
            continue;
        }
        colGrpCount++;
    }
    tr2.Restart();

    damage /= colGrpCount;

    while (carch = carchMan.Traverse(tr2))
    {
        if (carch->colGrp->hitPts < 100)
        {
            continue;
        }
        float colGrpDamage = std::min(carch->hitPts, ((damage + (carch->colGrp->hitPts * zd.percentageDamage)) * dmgMultiplier) / colGrpCount);
        if (colGrpDamage <= 0.0f)
        {
            continue;
        }

        ship->damage_col_grp(carch, colGrpDamage, dmgList);
    }

    float hulldamage = std::min(cship->hitPoints, dmgMultiplier * (damage + (zd.percentageDamage * ship->cobj->archetype->hitPoints)));

    if (hulldamage > 0.0f)
    {
        ship->damage_hull(hulldamage, dmgList);
    }
}