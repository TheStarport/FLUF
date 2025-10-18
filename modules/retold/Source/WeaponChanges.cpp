#include <PCH.hpp>

#include <Fluf.hpp>

#include "Retold.hpp"

FireResult __thiscall Retold::GunCanFireDetour(CEGun* gun, Vector& target)
{
    RetoldHooks::gunCanFireDetour.UnDetour();
    const auto canFire = RetoldHooks::gunCanFireDetour.GetOriginalFunc()(gun, target);
    RetoldHooks::gunCanFireDetour.Detour(GunCanFireDetour);

    const auto gunInfo = instance->extraWeaponData.find(gun->archetype->archId);
    if (canFire != FireResult::Success || gunInfo == instance->extraWeaponData.end())
    {
        return canFire;
    }

    auto& em = gun->owner->equipManager;

    if (const CEShield* shield = static_cast<CEShield*>(em.FindFirst(static_cast<uint>(EquipmentClass::Shield)));
        !shield || shield->currShieldHitPoints < gunInfo->second.shieldPowerUsage)
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
    if (const auto shield = static_cast<CEShield*>(em.FindFirst(static_cast<uint>(EquipmentClass::Shield))))
    {
        shield->currShieldHitPoints = std::clamp(shield->currShieldHitPoints - gunInfo->second.shieldPowerUsage, 0.f, shield->maxShieldHitPoints);
    }
}

void Retold::BeforeShipDestroy(Ship* ship, DamageList* dmgList, DestroyType destroyType, Id killerId)
{
    const auto shipId = ship->get_id();
    const auto shipDot = shipDots.find(shipId);
    if (shipDot == shipDots.end())
    {
        return;
    }

    for (auto& stack : shipDot->second)
    {
        // TODO: Remove FX?
    }

    shipDots.erase(shipDot);
}

void Retold::BeforeShipMunitionHitAfter(Ship* ship, MunitionImpactData* impact, DamageList* dmgList)
{
    auto& agm = ship->cship()->archGroupManager;

    float currentShields = 0.f;
    float maxShields = 0.f;
    bool shieldActive = true;
    pub::SpaceObj::GetShieldHealth(ship->get_id(), currentShields, maxShields, shieldActive);

    if (shieldActive)
    {
        return;
    }

    const auto munitionData = extraMunitionData.find(impact->munitionArch->archId);
    if (munitionData == extraMunitionData.end() || munitionData->second.hullDot == 0.f)
    {
        return;
    }

    float totalDamage = 0.f;
    auto& dotInfo = shipDots[ship->get_id()];
    for (const auto& data : dotInfo)
    {
        totalDamage += data.damageToApply;
    }

    const auto shipData = extraShipData.find(ship->ceqobj()->archetype->archId);
    const auto curHullDotMax = (shipData != extraShipData.end() && shipData->second.hullDotMax != 0.f) ? shipData->second.hullDotMax : hullDotMax;
    if (totalDamage >= curHullDotMax && totalDamage != 0.f)
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
    float damage = std::clamp(munitionData->second.hullDot, 0.f, maxStackDamage);
    dotInfo.emplace_back(hullDotDuration, damage, impact->subObjId);
}

void Retold::ApplyShipDotStacks()
{
    constexpr float delta = 1.f / 60.f;
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
            auto part = agm.FindByID(stack->targetHardpoint);
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
                if (const auto& back = list.damageEntries.back(); back.health == 0.f)
                {
                    stacks.clear();
                }
            }
            else
            {
                obj->damage_col_grp(part, damage, &list);
                if (const auto& back = list.damageEntries.back(); back.health == 0.f)
                {
                    std::erase_if(stacks, [&](auto& stack) { return stack.targetHardpoint == back.subObj; });
                }
            }
        }

        obj->apply_damage_list(&list);
    }
}
