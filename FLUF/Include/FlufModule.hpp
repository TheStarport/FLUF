#pragma once

#include "FLCore/Common/CArchGroup.hpp"
#include "FLCore/Common/EquipDesc.hpp"
#include "FLCore/st6.h"
#include "ImportFluf.hpp"
#include "Exceptions.hpp"

#include <FLCore/Common/Packets.hpp>
#include <FLCore/FLCoreRemoteClient.h>
#include <FLCore/Server.hpp>

struct FlufPayload;
class ClientServerCommunicator;
class ClientSend;
class ClientReceive;
class Fluf;
class IEngineHook;

enum class ModuleMajorVersion
{
    One = 1,
};

enum class ModuleMinorVersion
{
    Zero = 0
};

// ReSharper disable once CppClassCanBeFinal
class FLUF_API FlufModule
{
        friend Fluf;
        friend ClientReceive;
        friend ClientSend;
        friend ClientServerCommunicator;
        friend IEngineHook;

        ModuleMajorVersion majorVersion = ModuleMajorVersion::One;
        ModuleMinorVersion minorVersion = ModuleMinorVersion::Zero;

    protected:
        enum class ModuleProcessCode
        {
            ContinueUnhandled = 1,
            Continue = 2,
            Handled = 3
        };

        FlufModule() = default;
        virtual ~FlufModule() = default;

        virtual void OnLog(enum class LogLevel level, std::string_view message) {}

        virtual void OnUpdate(const double delta) {}
        virtual void OnFixedUpdate(const double delta) {};
        virtual void OnGameLoad() {}
        virtual void OnServerStart(const SStartupInfo&) {}
        virtual void OnDllLoaded(std::string_view dllName, HMODULE dllPtr) {}
        virtual void OnDllUnloaded(std::string_view dllName, HMODULE dllPtr) {}

        // Before Hooks

        virtual bool BeforeFireWeapon(uint client, XFireWeaponInfo& fwi) { return true; }
        virtual bool BeforeActivateEquip(uint client, XActivateEquip& aq) { return true; }
        virtual bool BeforeActivateCruise(uint client, XActivateCruise& ac) { return true; }
        virtual bool BeforeActivateThrusters(uint client, XActivateThrusters& at) { return true; }
        virtual bool BeforeSetTarget(uint client, XSetTarget& st) { return true; }
        virtual bool BeforeTractorObjects(uint client, XTractorObjects& to) { return true; }
        virtual bool BeforeEnterTradelane(uint client, XGoTradelane& gt) { return true; }
        virtual bool BeforeLeaveTradelane(uint client, uint shipId, uint tradelaneRing1, uint tradelaneRing2) { return true; }
        virtual bool BeforeJettisonCargo(uint client, XJettisonCargo& jc) { return true; }
        virtual bool BeforeDisconnect(uint client, EFLConnection conn) { return true; }
        virtual bool BeforeConnect(uint client) { return true; }
        virtual bool BeforeLogin(SLoginInfo& li, uint client) { return true; }
        virtual bool BeforeCharacterInfoRequest(uint client, bool) { return true; }
        virtual bool BeforeCharacterSelect(struct CHARACTER_ID& cid, uint client) { return true; }
        virtual bool BeforeCreateNewCharacter(SCreateCharacterInfo&, uint client) { return true; }
        virtual bool BeforeDestroyCharacter(CHARACTER_ID const&, uint client) { return true; }
        virtual bool BeforeReqEquipment(EquipDescList& edl, uint client) { return true; }
        virtual bool BeforeReqCargo(EquipDescList&, uint) { return true; }
        virtual bool BeforeReqAddItem(uint goodId, const char* hardpoint, int count, float status, bool mounted, uint client) { return true; }
        virtual bool BeforeReqRemoveItem(ushort slotId, int count, uint client) { return true; }
        virtual bool BeforeReqModifyItem(ushort slotId, const char* hardpoint, int count, float status, bool mounted, uint client) { return true; }
        virtual bool BeforeReqSetCash(int cash, uint client) { return true; }
        virtual bool BeforeReqChangeCash(int cashAdd, uint client) { return true; }
        virtual bool BeforeBaseEnter(uint baseId, uint client) { return true; }
        virtual bool BeforeBaseExit(uint baseId, uint client) { return true; }
        virtual bool BeforeLocationEnter(uint locationId, uint client) { return true; }
        virtual bool BeforeLocationExit(uint locationId, uint client) { return true; }
        virtual bool BeforeBaseInfoRequest(unsigned int, unsigned int, bool) { return true; }
        virtual bool BeforeLocationInfoRequest(unsigned int, unsigned int, bool) { return true; }
        virtual bool BeforeObjectSelect(unsigned int, unsigned int) { return true; }
        virtual bool BeforeGoodVaporized(const SGFGoodVaporizedInfo& gvi, uint client) { return true; }
        virtual bool BeforeMissionResponse(unsigned int, unsigned long, bool, uint client) { return true; }
        virtual bool BeforeTradeResponse(const unsigned char*, int, uint client) { return true; }
        virtual bool BeforeGoodBuy(const SGFGoodBuyInfo&, uint client) { return true; }
        virtual bool BeforeGoodSell(const SGFGoodSellInfo&, uint client) { return true; }
        virtual bool BeforeSystemSwitchOutComplete(uint shipId, uint client) { return true; }
        virtual bool BeforePlayerLaunch(uint shipId, uint client) { return true; }
        virtual bool BeforeLaunchComplete(uint baseId, uint shipId) { return true; }
        virtual bool BeforeJumpInComplete(uint systemId, uint shipId) { return true; }
        virtual bool BeforeHail(uint unk1, uint unk2, uint unk3) { return true; }
        virtual bool BeforeObjectUpdate(const SSPObjUpdateInfo& ui, uint client) { return true; }
        virtual bool BeforeMunitionCollision(const SSPMunitionCollisionInfo& mci, uint client) { return true; }
        virtual bool BeforeObjectCollision(const SSPObjCollisionInfo& oci, uint client) { return true; }
        virtual bool BeforeRequestEvent(int eventType, uint shipId, uint dockTarget, uint unk1, ulong unk2, uint client) { return true; }
        virtual bool BeforeRequestCancel(int eventType, uint shipId, uint unk1, ulong unk2, uint client) { return true; }
        virtual bool BeforeAbortMission(uint client, uint unk2) { return true; }
        virtual bool BeforeSetWeaponGroup(uint client, uchar*, int) { return true; }
        virtual bool BeforeSetVisitedState(uint client, uchar*, int) { return true; }
        virtual bool BeforeRequestBestPath(uint client, uchar*, int) { return true; }
        virtual bool BeforeRequestPlayerStats(uint client, uchar*, int) { return true; }
        virtual bool BeforeRequestGroupPositions(uint client, uchar*, int) { return true; }
        virtual bool BeforeInitiateTrade(uint client1, uint client2) { return true; }
        virtual bool BeforeTerminateTrade(uint client, int accepted) { return true; }
        virtual bool BeforeAcceptTrade(uint client, bool unk) { return true; }
        virtual bool BeforeSetTradeMoney(uint client, ulong money) { return true; }
        virtual bool BeforeAddTradeEquip(uint client, EquipDesc& desc) { return true; }
        virtual bool BeforeRemoveTradeEquip(uint client, EquipDesc& desc) { return true; }
        virtual bool BeforeRequestTrade(uint unk1, uint unk2) { return true; }
        virtual bool BeforeStopTradeRequest(uint client) { return true; }
        virtual bool BeforeRequestDifficultyScale(float scale, uint unk2) { return true; }
        virtual bool BeforeDock(uint& unk1, uint& unk2) { return true; }
        virtual bool BeforeSubmitChat(uint from, ulong size, const void* rdlReader, uint to, int) { return true; }
        virtual void BeforePhysicsUpdate(uint system, float delta) {}

        //Damage/init hooks
        virtual void BeforeShipDestroy(Ship* ship, DamageList* dmgList, DestroyType destroyType, Id killerId) {}
        virtual void BeforeLootDestroy(Loot* loot, DestroyType destroyType, Id killerId){}
        virtual void BeforeSolarDestroy(Solar* solar, DestroyType destroyType, Id killerId){}
        virtual void BeforeMineDestroy(Mine* mine, DestroyType destroyType, Id killerId){}
        virtual void BeforeGuidedDestroy(Guided* mine, DestroyType destroyType, Id killerId){}
        virtual void BeforeShipHullDamage(Ship* ship, float damage, DamageList* dmgList){}
        virtual void BeforeSolarHullDamage(Solar* ship, float damage, DamageList* dmgList){}
        virtual void BeforeSolarColGrpDestroy(Solar* solar, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList, bool killParent){}
        virtual void BeforeShipEquipDmg(Ship* ship, CAttachedEquip* equip, float damage, DamageList* dmgList){}
        virtual void BeforeShipEquipDestroy(Ship* ship, CEquip* equip, DamageEntry::SubObjFate fate, DamageList* dmgList){}
        virtual void BeforeShipColGrpDmg(Ship*, CArchGroup* colGrp, float incDmg, DamageList* dmg){}
        virtual void BeforeShipColGrpDestroy(Ship* ship, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList, bool killLinkedElements){}
        virtual void BeforeShipDropAllCargo(Ship* ship, const char* hardPoint, DamageList* dmgList){}
        virtual void BeforeShipRadiationDamage(Ship* ship, float deltaTime, DamageList* dmgList){}
        virtual void BeforeGuidedExplosionHit(Guided* guided, ExplosionDamageEvent* explosion, DamageList* dmgList){}
        virtual void BeforeSolarExplosionHit(Solar* guided, ExplosionDamageEvent* explosion, DamageList* dmgList){}
        virtual void BeforeShipMunitionHit(Ship* ship, MunitionImpactData* impact, DamageList* dmgList){}
        virtual void BeforeShipMunitionHitAfter(Ship* ship, MunitionImpactData* impact, DamageList* dmgList){}
        virtual void BeforeShipExplosionHit(Ship* ship, ExplosionDamageEvent* explosion, DamageList* dmgList){}
        virtual void BeforeShipShieldDmg(Ship* iobj, CEShield* shield, float incDmg, DamageList* dmg){}
        virtual void BeforeShipEnergyDmg(Ship* ship, float incDmg, DamageList* dmgList){}
        virtual void BeforeShipFuse(Ship* ship, uint fuseCause, uint& fuseId, ushort sId, float radius, float lifetime){}

        virtual void OnCShipInit(CShip* ship){}
        virtual void OnCSolarInit(CSolar* solar){}
        virtual void OnCLootInit(CLoot* loot){}
        virtual void OnCGuidedInit(CGuided* guided) {}

        // After Hooks

        virtual void OnFireWeapon(uint client, XFireWeaponInfo& info) {}
        virtual void OnActivateEquip(uint client, XActivateEquip& aq) {}
        virtual void OnActivateCruise(uint client, XActivateCruise& aq) {}
        virtual void OnActivateThruster(uint client, XActivateThrusters& aq) {}
        virtual void OnSetTarget(uint client, XSetTarget& st) {}
        virtual void OnEnterTradeLane(uint client, XGoTradelane& tl) {}
        virtual void OnStopTradeLane(uint client, uint shipId, uint archTradelane1, uint archTradelane2) {}
        virtual void OnJettisonCargo(uint client, XJettisonCargo& jc) {}
        virtual void OnLogin(uint client, bool singlePlayer, FLPACKET_UNKNOWN*) {}
        virtual void OnCharacterInformationReceived(uint client, FLPACKET_UNKNOWN*) {}
        virtual void OnCharacterSelect(uint client, FLPACKET_UNKNOWN*) {}
        virtual void OnAddItem(uint client, FLPACKET_UNKNOWN*, FLPACKET_UNKNOWN*) {}
        virtual void OnStartRoom(uint client, uint, uint) {}
        virtual void OnDestroyCharacter(uint client, uint, uint) {}
        virtual void OnUpdateCharacter(uint client, uint, uint) {}
        virtual void OnSetReputation(uint client, struct FLPACKET_SETREPUTATION& rep) {}
        virtual void OnLand(uint client, struct FLPACKET_LAND& land) {}
        virtual void OnLaunch(uint client, struct FLPACKET_LAUNCH& launch) {}
        virtual void OnSystemSwitchOut(uint client, struct FLPACKET_SYSTEM_SWITCH_OUT&) {}
        virtual void OnJumpInComplete(uint client, struct FLPACKET_SYSTEM_SWITCH_IN&) {}
        virtual void OnShipCreate(uint client, bool response, uint shipId) {}
        virtual void OnDamageObject(uint client, uint objId, DamageList& dmgList) {}
        virtual void OnItemTractored(uint client, uint) {}
        virtual void OnCargoObjectUpdated(struct SObjectCargoUpdate& cargoUpdate, uint dunno1, uint dunno2) {}
        virtual void OnFuseBurnStarted(uint client, struct FLPACKET_BURNFUSE& burnFuse) {}
        virtual void OnWeaponGroupSet(uint client, uint, int) {}
        virtual void OnVisitStateSet(uint client, uint objHash, int state) {}
        virtual void OnBestPathResponse(uint client, uint objHash, int) {}
        virtual void OnPlayerInformation(uint client, uint, int) {}
        virtual void OnGroupPositionResponse(uint client, uint, int) {}
        virtual void OnPlayerLeavingServer(uint onlineClient, uint leavingClient) {}
        virtual void OnFormationUpdate(uint client, uint shipId, Vector& formationOffset) {}
        virtual ModuleProcessCode OnPayloadReceived(uint sourceClientId, const FlufPayload& payload) { return ModuleProcessCode::ContinueUnhandled; }
        virtual void OnPhysicsUpdate(uint system, float delta) {}

    public:
        virtual std::string_view GetModuleName() = 0;
};

#define SETUP_MODULE(type)                                                      \
    __declspec(dllexport) std::shared_ptr<type> ModuleFactory()                 \
    {                                                                           \
        __pragma(comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)) {} \
        return std::move(std::make_shared<type>());                             \
    }

#define AssertRunningOnClient                                                                                                                     \
    if (!Fluf::IsRunningOnClient())                                                                                                               \
    {                                                                                                                                             \
        MessageBoxA(nullptr, std::format("{} should only be running on the client!", GetModuleName()).c_str(), "Wrong Execution Context", MB_OK); \
        throw ModuleLoadException(std::format("{} does not run in server context.", moduleName));                                                 \
    }

#define AssertRunningOnServer                                                                                                                     \
    if (Fluf::IsRunningOnClient())                                                                                                                \
    {                                                                                                                                             \
        MessageBoxA(nullptr, std::format("{} should only be running on the server!", GetModuleName()).c_str(), "Wrong Execution Context", MB_OK); \
        throw ModuleLoadException(std::format("{} does not run in server context.", moduleName));                                                 \
    }
