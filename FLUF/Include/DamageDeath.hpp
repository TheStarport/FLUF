#include "PCH.hpp"
#include "IObjectVtables.hpp"

#include <xbyak/xbyak.h>

class Fluf;

class IEngineHook
{
        friend Fluf;

    protected:
        using SendCommType = int (*)(uint sender, uint receiver, uint voiceId, const Costume* costume, uint infocardId, uint* lines, int lineCount,
                                     uint infocardId2, float radioSilenceTimerAfter, bool global);

        enum ZoneDamageType
        {
            ZONEDMG_HULL = 1 << 0,
            ZONEDMG_SHIELD = 1 << 1,
            ZONEDMG_ENERGY = 1 << 2,
            ZONEDMG_CRUISE = 1 << 3,
        };

        struct ZoneSpecialData
        {
                uint dmgType = ZONEDMG_HULL;
                float percentageDamage;
                float flatDamage;

                float distanceScaling;
                float logScale;
                float shieldMult;
                float energyMult;
        };

        inline static std::unordered_map<Id, ZoneSpecialData> zoneSpecialData;

        struct LoadRepData
        {
                uint repId;
                float attitude;
        };

        struct RepDataList
        {
                uint dunno;
                LoadRepData* begin;
                LoadRepData* end;
        };

        struct SendCommData
        {
                struct Callsign
                {
                        static constexpr uint AlphaDesignation = 197808;
                        static constexpr uint FreelancerCommHash = 0xb967660b;
                        static constexpr uint FreelancerAffiliation = 0x1049;

                        uint lastFactionAff = 0;
                        uint factionLine = FreelancerCommHash;
                        uint formationLine;
                        uint number1;
                        uint number2;
                };

                struct FactionData
                {
                        uint msgId;
                        std::vector<uint> formationHashes;
                };

                std::unordered_map<uint, Callsign> callsigns;
                std::unordered_map<uint, FactionData> factions;
                std::unordered_map<uint, std::pair<uint, uint>> numberHashes; // number said in the middle and in the end
        };

#define VTablePtr(x) static_cast<DWORD>(x)

        inline static VTableHook<VTablePtr(CShipVTable::Start), VTablePtr(CShipVTable::End)> cShipVTable{ "common" };
        inline static VTableHook<VTablePtr(CLootVTable::Start), VTablePtr(CLootVTable::End)> cLootVTable{ "common" };
        inline static VTableHook<VTablePtr(CSolarVTable::Start), VTablePtr(CSolarVTable::End)> cSolarVTable{ "common" };
        inline static VTableHook<VTablePtr(CGuidedVTable::Start), VTablePtr(CGuidedVTable::End)> cGuidedVTable{ "common" };
        inline static VTableHook<VTablePtr(CELauncherVTable::Start), VTablePtr(CELauncherVTable::End)> ceLauncherVTable{ "common" };

        inline static VTableHook<VTablePtr(IShipInspectVTable::Start), VTablePtr(IShipInspectVTable::End)> iShipVTable{ "server" };
        inline static VTableHook<VTablePtr(IShipAffectVTable::Start), VTablePtr(IShipAffectVTable::End)> iShipAffectVTable{ "server" };
        inline static VTableHook<VTablePtr(ISolarInspectVTable::Start), VTablePtr(ISolarInspectVTable::End)> iSolarVTable{ "server" };
        inline static VTableHook<VTablePtr(ILootInspectVTable::Start), VTablePtr(ILootInspectVTable::End)> iLootVTable{ "server" };
        inline static VTableHook<VTablePtr(IGuidedInspectVTable::Start), VTablePtr(IGuidedInspectVTable::End)> iGuidedVTable{ "server" };
        inline static VTableHook<VTablePtr(IMineInspectVTable::Start), VTablePtr(IMineInspectVTable::End)> iMineVTable{ "server" };

#undef VTablePtr

        static void __fastcall ShipDestroy(Ship* ship, DamageList* dmgList, DestroyType destroyType, Id killerId);
        static void __fastcall LootDestroy(Loot* loot, void* edx, DestroyType destroyType, Id killerId);
        static void __fastcall SolarDestroy(Solar* solar, void* edx, DestroyType destroyType, Id killerId);
        static void __fastcall MineDestroy(Mine* mine, void* edx, DestroyType destroyType, Id killerId);
        static void __fastcall GuidedDestroy(Guided* mine, void* edx, DestroyType destroyType, Id killerId);

        static void __fastcall ShipHullDamage(Ship* ship, void* edx, float damage, DamageList* dmgList);
        static void __fastcall SolarHullDamage(Solar* ship, void* edx, float damage, DamageList* dmgList);

        static void __fastcall SolarColGrpDestroy(Solar* solar, void* edx, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList,
                                                  bool killParent);

        static void __fastcall ShipEquipDmg(Ship* ship, void* edx, CAttachedEquip* equip, float damage, DamageList* dmgList);
        static void __fastcall ShipEquipDestroy(Ship* ship, void* edx, CEquip* equip, DamageEntry::SubObjFate fate, DamageList* dmgList);
        static void __fastcall SolarEquipDmg(Solar* solar, void* edx, CAttachedEquip* equip, float damage, DamageList* dmgList);

        static void __fastcall ShipColGrpDmg(Ship*, void* edx, CArchGroup* colGrp, float incDmg, DamageList* dmg);
        static void __fastcall ShipColGrpDestroy(Ship* ship, void* edx, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList,
                                                 bool killLinkedElements);
        static void __fastcall SolarColGrpDmg(Solar* solar, void* edx, CArchGroup* colGrp, float incDmg, DamageList* dmg);

        static void __fastcall ShipUseItem(Ship* ship, void* edx, ushort sId, uint count, uint clientId);

        static void __fastcall ShipDropAllCargo(Ship* ship, void* edx, const char* hardPoint, DamageList* dmgList);

        static void __fastcall ShipRadiationDamage(Ship* ship, void* edx, float deltaTime, DamageList* dmgList);

        static void __fastcall GuidedExplosionHit(Guided* guided, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList);
        static void __fastcall SolarExplosionHit(Solar* guided, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList);
        static void __fastcall ShipMunitionHit(Ship* ship, void* edx, MunitionImpactData* impact, DamageList* dmgList);
        static void __fastcall SolarMunitionHit(Solar* ship, void* edx, MunitionImpactData* impact, DamageList* dmgList);
        static void __fastcall ShipExplosionHit(Ship* ship, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList);

        static void __fastcall ShipShieldDmg(Ship* iobj, void* edx, CEShield* shield, float incDmg, DamageList* dmg);
        static void __fastcall ShipEnergyDmg(Ship* ship, void* edx, float incDmg, DamageList* dmgList);

        static void __fastcall CShipInit(CShip* ship, void* edx, CShip::CreateParms* creationParams);
        static void __thiscall CSolarInit(CSolar* solar, CSolar::CreateParms* creationParams);
        static void __fastcall CLootInit(CLoot* loot, void* edx, CLoot::CreateParms* createParams);
        static void __fastcall CGuidedInit(CGuided* guided, void* edx, CGuided::CreateParms* creationParams);

        static void __fastcall ShipFuse(Ship* ship, void* edx, uint fuseCause, uint& fuseId, ushort sId, float radius, float lifetime);
};
