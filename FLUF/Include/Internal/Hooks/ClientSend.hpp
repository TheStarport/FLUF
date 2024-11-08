#pragma once

#include <FLCore/FLCoreServer.h>

struct VTableHack;
class ClientSend
{
        friend VTableHack;

        // Commented out hooks are ones that exist on the vtable, but are not hooked as their value as not yet been determined.

        static void __fastcall FireWeapon(IServerImpl*, void* edx, uint client, XFireWeaponInfo& info);
        static void __fastcall ActivateEquip(IServerImpl*, void* edx, uint client, XActivateEquip& info);
        static void __fastcall ActivateCruise(IServerImpl*, void* edx, uint client, XActivateCruise& info);
        static void __fastcall ActivateThruster(IServerImpl*, void* edx, uint client, XActivateThrusters& at);
        static void __fastcall SetTarget(IServerImpl*, void* edx, uint client, XSetTarget& st);
        static void __fastcall TractorObjects(IServerImpl*, void* edx, uint client, XTractorObjects& info);
        static void __fastcall EnterTradeLane(IServerImpl*, void* edx, uint client, XGoTradelane& gt);
        static void __fastcall LeaveTradeLane(IServerImpl*, void* edx, uint client, uint shipId, uint tradelaneRing1, uint tradelaneRing2);
        static void __fastcall JettisonCargo(IServerImpl*, void* edx, uint client, XJettisonCargo& jc);
        static void __fastcall Disconnect(IServerImpl*, void* edx, uint client, EFLConnection connection);
        static void __fastcall Connect(IServerImpl*, void* edx, uint client);
        static void __fastcall Login(IServerImpl*, void* edx, SLoginInfo& li, uint client);
        static void __fastcall CharacterInfoRequest(IServerImpl*, void* edx, uint client, bool);
        static void __fastcall CharacterSelect(IServerImpl*, void* edx, CHARACTER_ID& cid, uint client);
        static void __fastcall CreateNewCharacter(IServerImpl*, void* edx, SCreateCharacterInfo&, uint client);
        static void __fastcall DestroyCharacter(IServerImpl*, void* edx, CHARACTER_ID const&, uint client);
        // static void __fastcall ReqShipArch(IServerImpl*, void* edx, uint archID, uint client);
        // static void __fastcall ReqHullStatus(IServerImpl*, void* edx, float status, uint client);
        // static void __fastcall ReqCollisionGroups(IServerImpl*, void* edx, st6::list<CollisionGroupDesc>& collisionGroups, uint client);
        // static void __fastcall ReqEquipment(IServerImpl*, void* edx, EquipDescList& edl, uint client);
        // static void __fastcall ReqCargo(IServerImpl*, void* edx, EquipDescList&, uint unk2);
        static void __fastcall RequestAddItem(IServerImpl*, void* edx, uint goodID, char* hardPoint, int count, float status, bool mounted, uint client);
        static void __fastcall RequestRemoveItem(IServerImpl*, void* edx, ushort slotId, int count, uint client);
        static void __fastcall RequestModifyItem(IServerImpl*, void* edx, ushort slotId, char* hardPoint, int count, float status, bool mounted, uint client);
        static void __fastcall RequestSetCash(IServerImpl*, void* edx, int newCash, uint client);
        static void __fastcall RequestChangeCash(IServerImpl*, void* edx, int cashChange, uint client);
        static void __fastcall BaseEnter(IServerImpl*, void* edx, uint baseId, uint client);
        static void __fastcall BaseExit(IServerImpl*, void* edx, uint baseId, uint client);
        static void __fastcall LocationEnter(IServerImpl*, void* edx, uint locationId, uint client);
        static void __fastcall LocationExit(IServerImpl*, void* edx, uint locationId, uint client);
        // static void __fastcall BaseInfoRequest(IServerImpl*, void* edx, unsigned int unk1, unsigned int unk1, bool);
        // static void __fastcall LocationInfoRequest(IServerImpl*, void* edx, unsigned int unk1, unsigned int unk1, bool);
        static void __fastcall ObjectSelect(IServerImpl*, void* edx, unsigned int unk1, unsigned int unk2);
        static void __fastcall GoodVaporized(IServerImpl*, void* edx, SGFGoodVaporizedInfo& gvi, uint client);
        // static void __fastcall MissionResponse(IServerImpl*, void* edx, unsigned int unk1, unsigned long, bool, uint client);
        static void __fastcall TradeResponse(IServerImpl*, void* edx, unsigned char*, int, uint client);
        static void __fastcall GoodBuy(IServerImpl*, void* edx, SGFGoodBuyInfo&, uint client);
        static void __fastcall GoodSell(IServerImpl*, void* edx, SGFGoodSellInfo&, uint client);
        static void __fastcall SystemSwitchOutComplete(IServerImpl*, void* edx, uint shipId, uint client);
        static void __fastcall PlayerLaunch(IServerImpl*, void* edx, uint shipID, uint client);
        static void __fastcall LaunchComplete(IServerImpl*, void* edx, uint baseID, uint client);
        static void __fastcall JumpInComplete(IServerImpl*, void* edx, uint systemId, uint shipId);
        static void __fastcall Hail(IServerImpl*, void* edx, unsigned int unk1, unsigned int unk2, unsigned int unk3);
        static void __fastcall ObjectUpdate(IServerImpl*, void* edx, SSPObjUpdateInfo& ui, uint client);
        static void __fastcall MunitionCollision(IServerImpl*, void* edx, SSPMunitionCollisionInfo& mci, uint client);
        static void __fastcall ObjectCollision(IServerImpl*, void* edx, SSPObjCollisionInfo& info, uint client);
        static void __fastcall RequestEvent(IServerImpl*, void* edx, int eventType, uint shipId, uint dockTarget, uint, ulong, uint client);
        static void __fastcall RequestCancel(IServerImpl*, void* edx, int eventType, uint shipId, uint, ulong, uint client);
        // static void __fastcall RequestCreateShip(IServerImpl*, void* edx, uint client);
        // static void __fastcall SPScanCargo(IServerImpl*, void* edx, uint&, uint&, uint unk2);
        // static void __fastcall SetManeuver(IServerImpl*, void* edx, uint client, XSetManeuver& sm);
        // static void __fastcall InterfaceItemUsed(IServerImpl*, void* edx, uint, uint unk2);
        static void __fastcall AbortMission(IServerImpl*, void* edx, uint client, uint unk2);
        static void __fastcall SetWeaponGroup(IServerImpl*, void* edx, uint client, uchar* unk1, int unk2);
        static void __fastcall SetVisitedState(IServerImpl*, void* edx, uint client, uchar* unk1, int unk2);
        static void __fastcall RequestBestPath(IServerImpl*, void* edx, uint client, uchar* unk1, int unk2);
        static void __fastcall RequestPlayerStats(IServerImpl*, void* edx, uint client, uchar* unk1, int unk2);
        // static void __fastcall RequestGroupPositions(IServerImpl*, void* edx, uint client, uchar* unk1, int unk2);
        // static void __fastcall SetMissionLog(IServerImpl*, void* edx, uint, uchar*, int unk2);
        // static void __fastcall SetInterfaceState(IServerImpl*, void* edx, uint client, uchar* unk1, int unk2);
        // static void __fastcall RequestRankLevel(IServerImpl*, void* edx, uint client, uchar* unk1, int unk2);
        static void __fastcall InitiateTrade(IServerImpl*, void* edx, uint client1, uint client2);
        static void __fastcall TerminateTrade(IServerImpl*, void* edx, uint client, int accepted);
        static void __fastcall AcceptTrade(IServerImpl*, void* edx, uint client, bool unk);
        static void __fastcall SetTradeMoney(IServerImpl*, void* edx, uint client, ulong money);
        static void __fastcall AddTradeEquip(IServerImpl*, void* edx, uint client, EquipDesc& ed);
        static void __fastcall DelTradeEquip(IServerImpl*, void* edx, uint client, EquipDesc& ed);
        static void __fastcall RequestTrade(IServerImpl*, void* edx, uint, uint unk2);
        static void __fastcall StopTradeRequest(IServerImpl*, void* edx, uint client);
        static void __fastcall RequestDifficultyScale(IServerImpl*, void* edx, float scale, uint unk2);
        static void __fastcall Dock(IServerImpl*, void* edx, uint&, uint&);
        static void __fastcall SubmitChat(IServerImpl*, void* edx, uint cidFrom, ulong size, void* rdlReader, uint cidTo, int unk2);
};
