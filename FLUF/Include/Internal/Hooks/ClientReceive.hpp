#pragma once

#include "FLCore/FLCoreRemoteClient.h"

#include <FLCore/Common.hpp>

struct VTableHack;
class ClientReceive
{
        friend VTableHack;
        static bool __fastcall FireWeapon(IClientImpl* clientImpl, void* edx, uint client, XFireWeaponInfo& info);
        static bool __fastcall ActivateEquip(IClientImpl* clientImpl, void* edx, uint client, XActivateEquip& info);
        static bool __fastcall ActivateCruise(IClientImpl* clientImpl, void* edx, uint client, XActivateCruise& info);
        static bool __fastcall ActivateThruster(IClientImpl* clientImpl, void* edx, uint client, XActivateThrusters& info);
        static bool __fastcall SetTarget(IClientImpl* clientImpl, void* edx, uint client, XSetTarget& st);
        static bool __fastcall EnterTradeLane(IClientImpl* clientImpl, void* edx, uint client, XGoTradelane& tl);
        static bool __fastcall StopTradeLane(IClientImpl* clientImpl, void* edx, uint client, uint shipID, uint archTradelane1, uint archTradelane2);
        static bool __fastcall JettisonCargo(IClientImpl* clientImpl, void* edx, uint client, XJettisonCargo& jc);
        static bool __fastcall Login(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN* unk);
        static bool __fastcall CharacterInformationReceived(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN* unk);
        static bool __fastcall CharacterSelect(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN* unk);
        static bool __fastcall AddItem(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN*, FLPACKET_UNKNOWN* unk2);
        static bool __fastcall StartRoom(IClientImpl* clientImpl, void*, uint client, uint, uint a2);
        static bool __fastcall DestroyCharacter(IClientImpl* clientImpl, void*, uint client, uint, uint a2);
        static bool __fastcall UpdateCharacter(IClientImpl* clientImpl, void*, uint client, uint, uint a2);
        static bool __fastcall SetReputation(IClientImpl* clientImpl, void* edx, uint client, FLPACKET_SETREPUTATION& rep);
        static bool __fastcall Land(IClientImpl* clientImpl, void* edx, uint client, FLPACKET_LAND& land);
        static bool __fastcall Launch(IClientImpl* clientImpl, void* edx, uint client, FLPACKET_LAUNCH& launch);
        static bool __fastcall SystemSwitchOut(IClientImpl* clientImpl, void*, uint client, FLPACKET_SYSTEM_SWITCH_OUT& out);
        static bool __fastcall JumpInComplete(IClientImpl* clientImpl, void*, uint client, FLPACKET_SYSTEM_SWITCH_IN& in);
        static bool __fastcall ShipCreate(IClientImpl* clientImpl, void* edx, uint client, bool response, uint shipId);
        static bool __fastcall DamageObject(IClientImpl* clientImpl, void* edx, uint client, uint objId, DamageList& dmgList);
        static bool __fastcall ItemTractored(IClientImpl* clientImpl, void*, uint client, uint itemId);
        static bool __fastcall CargoObjectUpdated(IClientImpl* clientImpl, void*, SObjectCargoUpdate& cargoUpdate, uint dunno1, uint dunno2);
        static bool __fastcall FuseBurnStarted(IClientImpl* clientImpl, void* edx, uint client, FLPACKET_BURNFUSE& burnFuse);
        static bool __fastcall WeaponGroupSet(IClientImpl* clientImpl, void*, uint client, uint unk1, int);
        static bool __fastcall VisitStateSet(IClientImpl* clientImpl, void* edx, uint client, uint objHash, int state);
        static bool __fastcall BestPathResponse(IClientImpl* clientImpl, void*, uint objHash, uint client, int unk);
        static bool __fastcall PlayerInformation(IClientImpl* clientImpl, void*, uint client, uint unk1, int);
        static bool __fastcall GroupPositionResponse(IClientImpl* clientImpl, void*, uint client, uint unk1, int);
        static bool __fastcall PlayerIsLeavingServer(IClientImpl* clientImpl, void* edx, uint client, uint leavingClient);
        static bool __fastcall FormationUpdate(IClientImpl* clientImpl, void* edx, uint client, uint shipId, Vector& formationOffset);
};
