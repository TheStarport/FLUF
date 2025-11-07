#include <PCH.hpp>

#include <Fluf.hpp>

#include "Retold.hpp"

// Reimplementation of common.dll version

FireResult Retold::CanGunFire(const CEGun* gun, const Vector& target)
{
    const auto arch = gun->GunArch();
    //
    if (gun->IsDestroyed())
    {
        return FireResult::ObjectIsDestroyed;
    }

    if (gun->IsDisabled() && !arch->autoTurret)
    {
        return FireResult::ObjectIsDisabled;
    }

    if (!gun->RefireDelayElapsed())
    {
        return FireResult::RefireDelayNotElapsed;
    }

    if (!gun->AmmoNeedsMet())
    {
        return FireResult::AmmoRequirementsNotMet;
    }

    if (!gun->PowerNeedsMet())
    {
        return FireResult::PowerRequirementsNotMet;
    }

    if (gun->owner->is_cloaked())
    {
        return FireResult::FailureCloakActive;
    }

    if (gun->owner->objectClass == CObject::CSHIP_OBJECT)
    {
        const auto ship = dynamic_cast<CShip*>(gun->owner);
        if (ship->is_using_tradelane())
        {
            return FireResult::FailureTradelane;
        }

        if (ship->is_cruise_active())
        {
            return FireResult::FailureCruiseActive;
        }
    }

    const auto muzzleConeAngle = instance->defaultMuzzleCone;
    const auto barrelPos = gun->GetBarrelPosWS(0);
    const auto relativeTargetPos = target - barrelPos;
    const auto mod = 1.0f / sqrtf(length(relativeTargetPos));

    const auto resultAngle = relativeTargetPos * mod;
    const auto direction = gun->GetBarrelDirWS(0);
    if (cos(muzzleConeAngle) >= length(resultAngle * direction))
    {
        return FireResult::FailureGunAngle;
    }

    return FireResult::Success;
}

FireResult __thiscall Retold::GunCanFireDetour(CEGun* gun, Vector& target)
{
    const auto canFire = CanGunFire(gun, target);

    const auto gunInfo = instance->extraWeaponData.find(gun->archetype->archId);
    if (canFire != FireResult::Success || gunInfo == instance->extraWeaponData.end())
    {
        return canFire;
    }

    auto& em = gun->owner->equipManager;

    if (const CEShield* shield = dynamic_cast<CEShield*>(em.FindFirst(static_cast<uint>(EquipmentClass::Shield)));
        !shield || !shield->IsFunctioning() || shield->currShieldHitPoints < gunInfo->second.shieldPowerUsage)
    {
        return FireResult::PowerRequirementsNotMet;
    }

    return FireResult::Success;
}

void __thiscall Retold::LauncherConsumeFireResourcesDetour(CELauncher* launcher)
{
    RetoldHooks::consumeFireResourcesDetour.UnDetour();
    RetoldHooks::consumeFireResourcesDetour.GetOriginalFunc()(launcher);
    RetoldHooks::consumeFireResourcesDetour.Detour(LauncherConsumeFireResourcesDetour);

    const auto gunInfo = instance->extraWeaponData.find(launcher->archetype->archId);
    if (gunInfo == instance->extraWeaponData.end())
    {
        return;
    }

    auto& em = launcher->owner->equipManager;
    if (const auto shield = dynamic_cast<CEShield*>(em.FindFirst(static_cast<uint>(EquipmentClass::Shield))))
    {
        shield->currShieldHitPoints = std::clamp(shield->currShieldHitPoints - gunInfo->second.shieldPowerUsage, 0.f, shield->maxShieldHitPoints);
    }
}

void Retold::BeforeShipDestroy(Ship* ship, DamageList* dmgList, DestroyType destroyType, Id killerId)
{
    const auto id = ship->get_id();

    shipHullVulnerabilities.erase(id);
    shipShieldRechargeDebuffs.erase(id);
    shipDots.erase(id);

    autoTurretTargets.remove(ship);

    objectShieldHitEffectMap.erase(id);
}

void Retold::BeforeSolarDestroy(Solar* solar, DestroyType destroyType, Id killerId)
{
    objectShieldHitEffectMap.erase(solar->get_id());
}

void Retold::BeforeShipMunitionHit(Ship* ship, MunitionImpactData* impact, DamageList* dmgList)
{
    const auto munitionData = extraMunitionData.find(impact->munitionArch->archId);
    if (munitionData == extraMunitionData.end())
    {
        return;
    }

    if (impact->subObjId > 1 && munitionData->second.equipmentMultiplier != 0.f)
    {
        equipmentMultipliersToApply = munitionData->second.equipmentMultiplier;
    }
}

void Retold::BeforeShipMunitionHitAfter(Ship* ship, MunitionImpactData* impact, DamageList* dmgList)
{
    float currentShields = 0.f;
    float maxShields = 0.f;
    bool shieldActive = true;
    pub::SpaceObj::GetShieldHealth(ship->get_id(), currentShields, maxShields, shieldActive);

    const auto munitionData = extraMunitionData.find(impact->munitionArch->archId);
    if (munitionData == extraMunitionData.end())
    {
        return;
    }

    ApplyShieldReductionStacks(ship, impact, munitionData->second);
    if (shieldActive)
    {
        return;
    }

    ApplyShipDotStacks(ship, impact, munitionData->second);
    ApplyShipVulnerabilityStacks(ship, impact, munitionData->second);
}

void Retold::BeforeShipEquipDmg(Ship* ship, CAttachedEquip* equip, float& damage, DamageList* dmgList)
{
    if (equipmentMultipliersToApply != 0.f)
    {
        damage *= equipmentMultipliersToApply;
        equipmentMultipliersToApply = 0.f;
    }
}

void Retold::BeforeShipColGrpDmg(Ship* ship, CArchGroup* colGrp, float& incDmg, DamageList* dmg)
{
    if (equipmentMultipliersToApply != 0.f)
    {
        incDmg *= equipmentMultipliersToApply;
        equipmentMultipliersToApply = 0.f;
    }
}

void Retold::BeforeShipHullDamage(Ship* ship, float& damage, DamageList* dmgList)
{
    if (const auto vulnList = shipHullVulnerabilities.find(ship->get_id()); vulnList != shipHullVulnerabilities.end())
    {
        float plasmaModifier = 1.f;
        for (const auto& plasma : vulnList->second)
        {
            plasmaModifier += plasma.modifier;
        }

        damage *= plasmaModifier;
    }
}

bool Retold::BeforeShipUseItem(Ship* ship, ushort sId, uint count, ClientId clientId)
{
    static auto NANOBOT_ARCH = CreateID("ge_s_repair_01");
    static auto BATTERY_ARCH = CreateID("ge_s_battery_01");

    auto cship = ship->cship();

    auto cargo = CECargo::cast(cship->equipManager.FindByID(sId));
    if (!cargo)
    {
        return true;
    }

    if (cargo->archetype->archId == NANOBOT_ARCH)
    {
        if (cship->hitPoints > (cship->archetype->hitPoints * 0.95f))
        {
            return true;
        }
        shipHealing[ship->get_id()].push_back({ nanobotHealingDuration, false });
    }
    else if (cargo->archetype->archId == BATTERY_ARCH)
    {
        CEShield* shield = reinterpret_cast<CEShield*>(cship->equipManager.FindFirst((int)EquipmentClass::Shield));
        if (!shield || (shield->currShieldHitPoints > shield->maxShieldHitPoints * 0.95f))
        {
            return true;
        }
        shipHealing[ship->get_id()].push_back({ batteryHealingDuration, true });
    }
    else
    {
        return true;
    }

    cargo->RemoveFromStack(1);

    //TODO: make this work when server.dll reloads at the different address
    using BroadcastRemovalOfItemType = void(__thiscall*)(StarSystem * starSystem, Ship * ship, ushort sId, uint count);
    static BroadcastRemovalOfItemType broadcastRemovalOfItemFunc = (BroadcastRemovalOfItemType)(DWORD(GetModuleHandleA("server")) + 0x2EFE0);

    broadcastRemovalOfItemFunc(ship->starSystem, ship, sId, 1);

    return false;
}

bool Retold::BeforeBaseEnter(uint baseId, uint client) { 

    if (SinglePlayer())
    {
        objectShieldHitEffectMap.clear();
    }

    return true;
}

void Retold::ProcessShipDotStacks(float delta)
{
    for (auto dots = shipDots.begin(); dots != shipDots.end();)
    {
        const auto obj = static_cast<EqObj*>(Fluf::GetObjInspect(dots->first));
        if (!obj || obj->ceqobj()->hitPoints == 0.0f)
        {
            dots = shipDots.erase(dots);
            continue;
        }

        std::unordered_map<CArchGroup*, float> groupDamage;
        auto& agm = obj->ceqobj()->archGroupManager;
        DamageList list;
        list.set_cause(DamageCause::Gun);
        for (auto stack = dots->second.begin(); stack != dots->second.end();)
        {
            const auto part = agm.FindByID(stack->targetSId);
            if ((!part && stack->targetSId != 1) || stack->timeLeft <= 0.f)
            {
                stack = dots->second.erase(stack);
                continue;
            }

            auto& entry = groupDamage[part];

            entry += stack->damageToApply * std::min(delta, stack->timeLeft);
            list.inflictorId = stack->inflicterId;

            stack->timeLeft -= delta;
            ++stack;
        }

        for (auto& [part, damage] : groupDamage)
        {
            if (!part)
            {
                obj->damage_hull(damage, &list);
            }
            else
            {
                obj->damage_col_grp(part, damage, &list);
            }
        }

        obj->apply_damage_list(&list);

        if (dots->second.empty())
        {
            dots = shipDots.erase(dots);
            continue;
        }

        dots++;
    }
}

void Retold::ProcessShipHealingStacks(float delta)
{
    for (auto healingData = shipHealing.begin(); healingData != shipHealing.end();)
    {
        const auto obj = static_cast<EqObj*>(Fluf::GetObjInspect(healingData->first));
        if (!obj)
        {
            continue;
        }

        float healingHull = 0.0f;
        float healingShield = 0.0f;

        for (auto stackIter = healingData->second.begin(); stackIter != healingData->second.end();)
        {
            float timeToHeal = std::min(delta, stackIter->timeLeft);

            if (!stackIter->isShield)
            {
                healingHull += delta * obj->ceqobj()->archetype->hitPoints * nanobotHealingPerSecond;
            }
            else
            {
                CEShield* shield = reinterpret_cast<CEShield*>(obj->ceqobj()->equipManager.FindFirst((int)EquipmentClass::Shield));
                if (!shield)
                {
                    stackIter = healingData->second.erase(stackIter);
                    continue;
                }
                healingShield += delta * shield->maxShieldHitPoints * batteryHealingPerSecond;
            }

            stackIter->timeLeft -= delta;
            if (stackIter->timeLeft <= 0.0f)
            {
                stackIter = healingData->second.erase(stackIter);
                continue;
            }
            stackIter++;
        }

        if (healingHull || healingShield)
        {
            DamageList dmg;
            if (healingHull)
            {
                obj->damage_hull(-healingHull, &dmg);
            }

            if (healingShield)
            {
                CEShield* shield = reinterpret_cast<CEShield*>(obj->ceqobj()->equipManager.FindFirst((int)EquipmentClass::Shield));
                if (shield)
                {
                    obj->damage_shield_direct(shield, -healingShield, &dmg);
                }
            }

            obj->apply_damage_list(&dmg);
        }

        if (healingData->second.empty())
        {
            healingData = shipHealing.erase(healingData);
            continue;
        }

        healingData++;
    }
}

void Retold::RemoveShieldReductionStacks(float delta)
{
    // TODO: This function and the one below it are near identical, merge?
    for (auto& reductions : shipShieldRechargeDebuffs | std::views::values)
    {
        for (auto stack = reductions.begin(); stack != reductions.end();)
        {
            stack->first -= delta;
            if (stack->first <= 0.f)
            {
                stack = reductions.erase(stack);
            }
            else
            {
                ++stack;
            }
        }
    }
}

void Retold::RemoveShipVulnerabilityStacks(float delta)
{
    for (auto& [id, vulnerabilities] : shipHullVulnerabilities)
    {
        for (auto stack = vulnerabilities.begin(); stack != vulnerabilities.end();)
        {
            stack->duration -= delta;
            if (stack->duration <= 0.f)
            {
                if (const auto obj = dynamic_cast<EqObj*>(Fluf::GetObjInspect(id)); obj && stack->fuseId)
                {
                    obj->unlight_fuse(stack->fuseId, stack->hardPoint, 0.f);
                }

                stack = vulnerabilities.erase(stack);
            }
            else
            {
                ++stack;
            }
        }
    }
}

void __fastcall Retold::ShieldRegenerationPatch(CEShieldGenerator* generator, CEShield* shield, float delta)
{
    float regenRate = 0.f;
    const auto arch = generator->ShieldGenArch();

    if (shield->IsFunctioning())
    {
        regenRate = arch->regenerationRate;
    }
    else
    {
        const auto customShield = instance->extraShieldData.find(arch->archId);
        if (customShield != instance->extraShieldData.end() && customShield->second.offlineRegenerationRate != 0.f)
        {
            regenRate = customShield->second.offlineRegenerationRate;
        }
        else
        {
            regenRate = arch->regenerationRate;
        }
    }

    if (regenRate < 0.f)
    {
        return;
    }

    const auto hitPts = shield->GetHitPoints();
    shield->SetHitPoints(std::clamp(hitPts + regenRate * delta, 0.f, 1'000'000'000.f));
}

void __declspec(naked) Retold::ShieldRegenerationPatchNaked()
{
    static constexpr DWORD returnAddress = 0x629CF11;
    __asm
    {
        mov edx, ecx // CEShield
        mov ecx, esi // CEShieldGenerator
        mov eax, [esp+0xC+0x4] // frame delta
        push eax
        call Retold::ShieldRegenerationPatch
        pop edi
        jmp returnAddress
    }
}

void __thiscall Retold::ShieldSetHealthDetour(CEShield* shield, float hitPts)
{
    const auto owner = shield->GetOwner();
    const auto currentHitPts = shield->GetHitPoints();

    // We are increasing our shields, lets apply any needed debufs
    if (currentHitPts < hitPts)
    {
        auto diff = hitPts - currentHitPts;
        const auto activeDebuffs = instance->shipShieldRechargeDebuffs.find(owner->id);
        if (activeDebuffs != instance->shipShieldRechargeDebuffs.end())
        {
            float reduction = 1.f;
            for (const auto val : activeDebuffs->second | std::views::values)
            {
                reduction -= val;
            }

            reduction = std::clamp(reduction, 0.f, 1.f);
            diff *= reduction;
            hitPts = currentHitPts + diff;
        }
    }

    auto shieldOnlineState = shield->internalActivationState;

    RetoldHooks::shieldSetHealthDetour.UnDetour();
    RetoldHooks::shieldSetHealthDetour.GetOriginalFunc()(shield, hitPts);
    RetoldHooks::shieldSetHealthDetour.Detour(ShieldSetHealthDetour);

    //TODO: Move to client specific function
    if (Fluf::GetPlayerIObj() && shieldOnlineState != shield->internalActivationState)
    {
        const auto data = instance->extraShipData.find(shield->GetOwner()->archetype->archId);
        const auto inspect = dynamic_cast<EqObj*>(Fluf::GetObjInspect(shield->GetOwner()->id));
        if (!inspect || data == instance->extraShipData.end() || data->second.shieldOfflineFuse == 0)
        {
            return;
        }

        if (shieldOnlineState)
        {
            // Shield was on, now off
            inspect->light_fuse(0, data->second.shieldOfflineFuse, 0, 0.f, 0.f);
        }
        else
        {
            // Shield was off, now on
            inspect->unlight_fuse(data->second.shieldOfflineFuse, 0, 0.f);
        }
    }
}

void Retold::ApplyShipDotStacks(Ship* ship, MunitionImpactData* impact, const ExtraMunitionData& munitionData)
{
    if (munitionData.hullDot == 0.f)
    {
        return;
    }

    auto& agm = ship->cship()->archGroupManager;
    float totalDamage = 0.f;
    auto& dotInfo = shipDots[ship->get_id()];
    for (const auto& data : dotInfo)
    {
        totalDamage += data.damageToApply;
    }

    const auto shipData = extraShipData.find(ship->ceqobj()->archetype->archId);
    const auto curHullDotMax = (shipData != extraShipData.end() && shipData->second.hullDotMax != 0.f) ? shipData->second.hullDotMax : hullDotMax;

    auto& em = ship->cship()->equipManager;
    CEquipTraverser trav{ static_cast<int>(EquipmentClass::Armor) };
    CEArmor* armor;
    float hitPtsScale = 1.f;
    while ((armor = dynamic_cast<CEArmor*>(em.Traverse(trav))))
    {
        hitPtsScale += armor->ArmorArch()->hitPointsScale - 1.f;
    }

    if (totalDamage >= curHullDotMax * hitPtsScale && totalDamage != 0.f)
    {
        return;
    }

    if (impact->subObjId != 1)
    {
        const auto* arch = agm.FindByID(impact->subObjId);
        if (!arch)
        {
            return;
        }
    }

    const auto maxStackDamage = curHullDotMax - totalDamage;
    float damage = std::clamp(munitionData.hullDot, 0.f, maxStackDamage);
    dotInfo.emplace_back(hullDotDuration, damage, impact->subObjId, impact->attackerId);
}

void Retold::ApplyShipVulnerabilityStacks(Ship* ship, MunitionImpactData* impact, const ExtraMunitionData& munitionData)
{
    if (munitionData.hullVulnerability == 0.f)
    {
        return;
    }

    float plasmaModifier = munitionData.hullVulnerability;
    auto& em = ship->cship()->equipManager;
    CEquipTraverser trav{ static_cast<int>(EquipmentClass::Armor) };
    const CEArmor* armor = nullptr;
    while ((armor = static_cast<CEArmor*>(em.Traverse(trav))))
    {
        plasmaModifier /= armor->ArmorArch()->hitPointsScale;
    }

    const auto shipModifiers = extraShipData.find(ship->ceqobj()->archetype->archId);
    auto& fuses = shipModifiers == extraShipData.end() ? hullVulnerabilityFuses : shipModifiers->second.hullVulnerabilityFuses;

    uint fuseId = 0;
    for (auto [threshold, fuse] : fuses)
    {
        if (plasmaModifier >= threshold)
        {
            fuseId = fuse;
            ship->light_fuse(0, fuse, impact->subObjId, 0.f, 0.f);
            break;
        }
    }

    auto& vulnerabilities = shipHullVulnerabilities[ship->get_id()];
    vulnerabilities.emplace_back(hullVulnerabilityDuration, plasmaModifier, fuseId, impact->subObjId);
}

void Retold::ApplyShieldReductionStacks(Ship* ship, MunitionImpactData* impact, const ExtraMunitionData& munitionData)
{
    if (munitionData.shieldRechargeReduction <= 0.f)
    {
        return;
    }

    auto& em = ship->cship()->equipManager;
    CEquipTraverser traverser{ static_cast<int>(EquipmentClass::ShieldGenerator) };
    CEShieldGenerator* gen;
    float shieldStrength = 1.f;
    while ((gen = static_cast<CEShieldGenerator*>(em.Traverse(traverser))))
    {
        if (auto shield = extraShieldData.find(gen->archetype->archId); shield != extraShieldData.end())
        {
            shieldStrength += shield->second.shieldStrength;
        }
    }

    auto& reductionInfo = shipShieldRechargeDebuffs[ship->get_id()];
    reductionInfo.emplace_back(shieldRechargeReductionDuration, munitionData.shieldRechargeReduction / shieldStrength);
}
