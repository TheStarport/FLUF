#include <PCH.hpp>

#include "Retold.hpp"

#include <Fluf.hpp>

void Retold::ProcessAutoTurrets(float delta)
{
    static float timeUntilPool = 1.f;
    timeUntilPool -= delta;

    if (!Fluf::IsRunningOnClient() || !autoTurretsEnabled)
    {
        return;
    }

    auto iObj = Fluf::GetPlayerIObj();
    if (!iObj)
    {
        return;
    }

    auto* equipList = reinterpret_cast<st6::list<CliLauncher*>*>(reinterpret_cast<DWORD>(iObj) + (4 * 45));

    // TODO: Handle when obj or auto turret is destroyed / released
    if (timeUntilPool <= 0.f)
    {
        Fluf::Info("Searching");
        timeUntilPool = 1.f;
        autoTurrets.clear();
        autoTurretTargets.clear();

        float maxRange = 0.f;
        for (const auto& equip : *equipList)
        {
            // TODO: verify whether the equipList contains only launchers or all possible items
            if (equip->launcher->archetype->get_class_type() != Archetype::ClassType::Gun || !reinterpret_cast<CEGun*>(equip->launcher)->GunArch()->autoTurret)
            {
                continue;
            }

            autoTurrets.emplace_back(equip);

            if (const float range = equip->launcher->GetEffectiveRange(); range > maxRange)
            {
                maxRange = range;
            }
        }

        auto playerPos = iObj->get_position();
        IObjRW* playerTarget{};
        iObj->get_target(playerTarget);
        auto obj = dynamic_cast<CEqObj*>(CObject::FindFirst(CObject::Class::CSHIP_OBJECT));
        while (obj)
        {
            const auto ship = obj;
            obj = dynamic_cast<CEqObj*>(CObject::FindNext());
            if (iObj->cobj != obj && std::fabsf(length(playerPos - ship->position)) > maxRange * 2.f)
            {
                continue;
            }

            const auto npcIObj = dynamic_cast<EqObj*>(Fluf::GetObjInspect(ship->id));
            if (!npcIObj)
            {
                continue;
            }

            float attitude = 0.0f;
            npcIObj->get_attitude_towards(attitude, reinterpret_cast<const IObjInspect*>(iObj));

            if (attitude > -0.6f)
            {
                continue;
            }

            autoTurretTargets.emplace_back(npcIObj);
        }

        // Sort objects, closest-to-farthest, and prioritise the player's target if possible
        autoTurretTargets.sort(
            [&playerPos, playerTarget](const EqObj* a, const EqObj* b)
            {
                return reinterpret_cast<const IObjRW*>(a) == playerTarget ||
                       std::fabsf(length(playerPos - a->get_position())) < std::fabsf(length(playerPos - b->get_position()));
            });
    }

    // Nothing in range, nothing to shoot
    if (autoTurretTargets.empty())
    {
        return;
    }

    auto& gunStats = const_cast<ShipGunStats&>(iObj->cship()->get_gun_stats());
    const auto originalGunStats = gunStats;
    for (const auto equip : autoTurrets)
    {
        if (equip->launcher->IsFunctioning())
        {
            // We only want auto turrets to fire when they are offline (not in active weapon group)
            continue;
        }

        gunStats.activeGunCount = 1;
        gunStats.maxGunRange = equip->launcher->GetEffectiveRange();
        gunStats.avgGunSpeed = equip->launcher->LauncherArch()->muzzleVelocity;

        // Cycle through the possible targets and see if they are in range, and if so, try attack
        for (const EqObj* target : autoTurretTargets)
        {
            Vector targetPos{};
            if (!iObj->cship()->get_tgt_lead_fire_pos(reinterpret_cast<const IObjInspect*>(target), targetPos))
            {
                continue;
            }

            const auto gun = reinterpret_cast<CEGun*>(equip->launcher);
            if (!gun->CanPointAt(targetPos, defaultMuzzleCone))
            {
                // Incapable of looking at that target, go next
                continue;
            }

            gun->SetTargetPoint(targetPos);
            if (gun->CanSeeTargetPoint(targetPos, defaultMuzzleCone))
            {
                if (const auto status = equip->launcher->CanFire(targetPos); status == FireResult::Success && equip->fire(targetPos))
                {
                    auto pos = equip->launcher->GetBarrelPosWS(0);
                    const auto mult = 1.0f / static_cast<float>(equip->launcher->GetProjectilesPerFire());
                    pos *= mult;
                    CliLauncher::PlayFireSound(equip, pos, nullptr);
                }
            }

            break;
        }
    }

    gunStats = originalGunStats;
}
