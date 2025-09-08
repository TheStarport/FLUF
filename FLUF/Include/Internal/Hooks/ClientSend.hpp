#pragma once

#include <FLCore/FLCoreServer.h>

struct Fluf;
class ClientSend
{
        friend Fluf;

        // Commented out hooks are ones that exist on the vtable, but are not hooked as their value as not yet been determined.

        static void __thiscall FireWeapon(IServerImpl*, uint client, XFireWeaponInfo& info);
        static void __thiscall ActivateEquip(IServerImpl*, uint client, XActivateEquip& info);
        static void __thiscall ActivateCruise(IServerImpl*, uint client, XActivateCruise& info);
        static void __thiscall ActivateThruster(IServerImpl*, uint client, XActivateThrusters& at);
        static void __thiscall SetTarget(IServerImpl*, uint client, XSetTarget& st);
        static void __thiscall TractorObjects(IServerImpl*, uint client, XTractorObjects& info);
        static void __thiscall EnterTradeLane(IServerImpl*, uint client, XGoTradelane& gt);
        static void __thiscall LeaveTradeLane(IServerImpl*, uint client, uint shipId, uint tradelaneRing1, uint tradelaneRing2);
        static void __thiscall JettisonCargo(IServerImpl*, uint client, XJettisonCargo& jc);
        static void __thiscall Disconnect(IServerImpl*, uint client, EFLConnection connection);
        static void __thiscall Connect(IServerImpl*, uint client);
        static void __thiscall Startup(IServerImpl*, const SStartupInfo&);
        static int  __thiscall Update(IServerImpl*);
        static void __thiscall Login(IServerImpl*, SLoginInfo& li, uint client);
        static void __thiscall CharacterInfoRequest(IServerImpl*, uint client, bool);
        static void __thiscall CharacterSelect(IServerImpl*, CHARACTER_ID& cid, uint client);
        static void __thiscall CreateNewCharacter(IServerImpl*, SCreateCharacterInfo&, uint client);
        static void __thiscall DestroyCharacter(IServerImpl*, CHARACTER_ID const&, uint client);
        // static void __thiscall ReqShipArch(IServerImpl*,  uint archID, uint client);
        // static void __thiscall ReqHullStatus(IServerImpl*,  float status, uint client);
        // static void __thiscall ReqCollisionGroups(IServerImpl*,  st6::list<CollisionGroupDesc>& collisionGroups, uint client);
        // static void __thiscall ReqEquipment(IServerImpl*,  EquipDescList& edl, uint client);
        // static void __thiscall ReqCargo(IServerImpl*,  EquipDescList&, uint unk2);
        static void __thiscall RequestAddItem(IServerImpl*, uint goodID, char* hardPoint, int count, float status, bool mounted, uint client);
        static void __thiscall RequestRemoveItem(IServerImpl*, ushort slotId, int count, uint client);
        static void __thiscall RequestModifyItem(IServerImpl*, ushort slotId, char* hardPoint, int count, float status, bool mounted, uint client);
        static void __thiscall RequestSetCash(IServerImpl*, int newCash, uint client);
        static void __thiscall RequestChangeCash(IServerImpl*, int cashChange, uint client);
        static void __thiscall BaseEnter(IServerImpl*, uint baseId, uint client);
        static void __thiscall BaseExit(IServerImpl*, uint baseId, uint client);
        static void __thiscall LocationEnter(IServerImpl*, uint locationId, uint client);
        static void __thiscall LocationExit(IServerImpl*, uint locationId, uint client);
        // static void __thiscall BaseInfoRequest(IServerImpl*,  unsigned int unk1, unsigned int unk1, bool);
        // static void __thiscall LocationInfoRequest(IServerImpl*,  unsigned int unk1, unsigned int unk1, bool);
        static void __thiscall ObjectSelect(IServerImpl*, unsigned int unk1, unsigned int unk2);
        static void __thiscall GoodVaporized(IServerImpl*, SGFGoodVaporizedInfo& gvi, uint client);
        // static void __thiscall MissionResponse(IServerImpl*,  unsigned int unk1, unsigned long, bool, uint client);
        static void __thiscall TradeResponse(IServerImpl*, unsigned char*, int, uint client);
        static void __thiscall GoodBuy(IServerImpl*, SGFGoodBuyInfo&, uint client);
        static void __thiscall GoodSell(IServerImpl*, SGFGoodSellInfo&, uint client);
        static void __thiscall SystemSwitchOutComplete(IServerImpl*, uint shipId, uint client);
        static void __thiscall PlayerLaunch(IServerImpl*, uint shipID, uint client);
        static void __thiscall LaunchComplete(IServerImpl*, uint baseID, uint client);
        static void __thiscall JumpInComplete(IServerImpl*, uint systemId, uint shipId);
        static void __thiscall Hail(IServerImpl*, unsigned int unk1, unsigned int unk2, unsigned int unk3);
        static void __thiscall ObjectUpdate(IServerImpl*, SSPObjUpdateInfo& ui, uint client);
        static void __thiscall MunitionCollision(IServerImpl*, SSPMunitionCollisionInfo& mci, uint client);
        static void __thiscall ObjectCollision(IServerImpl*, SSPObjCollisionInfo& info, uint client);
        static void __thiscall RequestEvent(IServerImpl*, int eventType, uint shipId, uint dockTarget, uint, ulong, uint client);
        static void __thiscall RequestCancel(IServerImpl*, int eventType, uint shipId, uint, ulong, uint client);
        // static void __thiscall RequestCreateShip(IServerImpl*,  uint client);
        // static void __thiscall SPScanCargo(IServerImpl*,  uint&, uint&, uint unk2);
        // static void __thiscall SetManeuver(IServerImpl*,  uint client, XSetManeuver& sm);
        // static void __thiscall InterfaceItemUsed(IServerImpl*,  uint, uint unk2);
        static void __thiscall AbortMission(IServerImpl*, uint client, uint unk2);
        static void __thiscall SetWeaponGroup(IServerImpl*, uint client, uchar* unk1, int unk2);
        static void __thiscall SetVisitedState(IServerImpl*, uint client, uchar* unk1, int unk2);
        static void __thiscall RequestBestPath(IServerImpl*, uint client, uchar* unk1, int unk2);
        static void __thiscall RequestPlayerStats(IServerImpl*, uint client, uchar* unk1, int unk2);
        // static void __thiscall RequestGroupPositions(IServerImpl*,  uint client, uchar* unk1, int unk2);
        // static void __thiscall SetMissionLog(IServerImpl*,  uint, uchar*, int unk2);
        // static void __thiscall SetInterfaceState(IServerImpl*,  uint client, uchar* unk1, int unk2);
        // static void __thiscall RequestRankLevel(IServerImpl*,  uint client, uchar* unk1, int unk2);
        static void __thiscall InitiateTrade(IServerImpl*, uint client1, uint client2);
        static void __thiscall TerminateTrade(IServerImpl*, uint client, int accepted);
        static void __thiscall AcceptTrade(IServerImpl*, uint client, bool unk);
        static void __thiscall SetTradeMoney(IServerImpl*, uint client, ulong money);
        static void __thiscall AddTradeEquip(IServerImpl*, uint client, EquipDesc& ed);
        static void __thiscall DelTradeEquip(IServerImpl*, uint client, EquipDesc& ed);
        static void __thiscall RequestTrade(IServerImpl*, uint, uint unk2);
        static void __thiscall StopTradeRequest(IServerImpl*, uint client);
        static void __thiscall RequestDifficultyScale(IServerImpl*, float scale, uint unk2);
        static void __thiscall Dock(IServerImpl*, uint&, uint&);
};
