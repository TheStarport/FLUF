#include <PCH.hpp>

#include <Fluf.hpp>

#include "Retold.hpp"

// Reimplementation of common.dll version

FireResult CanGunFire(const CEGun* gun, const Vector& target)
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

    const auto muzzleConeAngle = MUZZLE_CONE_ANGLE * 0.017453292f;
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
        !shield || (shield->IsFunctioning() && shield->currShieldHitPoints < gunInfo->second.shieldPowerUsage))
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

void Retold::ProcessShipDotStacks(float delta)
{
    for (auto& [id, stacks] : shipDots)
    {
        const auto obj = static_cast<EqObj*>(Fluf::GetObjInspect(id));
        if (!obj)
        {
            continue;
        }

        std::unordered_map<uint, std::pair<float, CArchGroup*>> groupDamage;
        auto& agm = obj->ceqobj()->archGroupManager;
        DamageList list;
        for (auto stack = stacks.begin(); stack != stacks.end();)
        {
            const auto part = agm.FindByID(stack->targetHardpoint);
            if ((!part && stack->targetHardpoint != 1) || stack->timeLeft <= 0.f)
            {
                stack = stacks.erase(stack);
                continue;
            }

            auto& data = groupDamage[stack->targetHardpoint];
            if (stack->targetHardpoint != 1)
            {
                data.second = part;
            }

            data.first += stack->damageToApply * delta;

            stack->timeLeft -= delta;
            ++stack;
        }

        for (auto& [damage, part] : groupDamage | std::views::values)
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
    dotInfo.emplace_back(hullDotDuration, damage, impact->subObjId);
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
