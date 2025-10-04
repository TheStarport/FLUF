#pragma once
#include "PCH.hpp"

#include <FLCore/Server.hpp>
#include <FLCore/Common/Damage.hpp>

class IEngineHook;

class FLUF_API DamageDeathInterface
{
    friend IEngineHook;
    protected:
        static void OnShipDestroy(Ship* ship, DamageList* dmgList, DestroyType destroyType, Id killerId);
        static void OnLootDestroy(Loot* loot, DestroyType destroyType, Id killerId);
        static void OnSolarDestroy(Solar* solar, DestroyType destroyType, Id killerId);
        static void OnMineDestroy(Mine* mine, DestroyType destroyType, Id killerId);
        static void OnGuidedDestroy(Guided* mine, DestroyType destroyType, Id killerId);
                    
        static void OnShipHullDamage(Ship* ship, float damage, DamageList* dmgList);
        static void OnSolarHullDamage(Solar* ship, float damage, DamageList* dmgList);
                    
        static void OnSolarColGrpDestroy(Solar* solar, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList,
                                       bool killParent);
                    
        static void OnShipEquipDmg(Ship* ship, CAttachedEquip* equip, float damage, DamageList* dmgList);
        static void OnShipEquipDestroy(Ship* ship, CEquip* equip, DamageEntry::SubObjFate fate, DamageList* dmgList);
                    
        static void OnShipColGrpDmg(Ship*, CArchGroup* colGrp, float incDmg, DamageList* dmg);
        static void OnShipColGrpDestroy(Ship* ship, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList,
                                      bool killLinkedElements);
                    
        static void OnShipDropAllCargo(Ship* ship, const char* hardPoint, DamageList* dmgList);
                    
        static void OnShipRadiationDamage(Ship* ship, float deltaTime, DamageList* dmgList);
                    
        static void OnGuidedExplosionHit(Guided* guided, ExplosionDamageEvent* explosion, DamageList* dmgList);
        static void OnSolarExplosionHit(Solar* guided, ExplosionDamageEvent* explosion, DamageList* dmgList);
        static void OnShipMunitionHit(Ship* ship, MunitionImpactData* impact, DamageList* dmgList);
        static void OnShipMunitionHitAfter(Ship* ship, MunitionImpactData* impact, DamageList* dmgList);
        static void OnShipExplosionHit(Ship* ship, ExplosionDamageEvent* explosion, DamageList* dmgList);
                    
        static void OnShipShieldDmg(Ship* iobj, CEShield* shield, float incDmg, DamageList* dmg);
        static void OnShipEnergyDmg(Ship* ship, float incDmg, DamageList* dmgList);
                    
        static void OnCShipInit(CShip* ship, CShip::CreateParms* creationParams);
        static void OnCSolarInit(CSolar* solar, CSolar::CreateParms* creationParams);
        static void OnCLootInit(CLoot* loot, CLoot::CreateParms* createParams);
        static void OnCGuidedInit(CGuided* guided, CGuided::CreateParms* creationParams);
                    
        static void OnShipFuse(Ship* ship, uint fuseCause, uint& fuseId, ushort sId, float radius, float lifetime);
};
