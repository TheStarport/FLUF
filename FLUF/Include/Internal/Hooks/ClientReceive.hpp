#pragma once

#include "FLCore/FLCoreRemoteClient.h"

#include <FLCore/Common.hpp>

class Fluf;
class ClientReceive
{
        friend Fluf;
        static bool __thiscall FireWeapon(IClientImpl* clientImpl, uint client, XFireWeaponInfo& info);
        static bool __thiscall ActivateEquip(IClientImpl* clientImpl, uint client, XActivateEquip& info);
        static bool __thiscall ActivateCruise(IClientImpl* clientImpl, uint client, XActivateCruise& info);
        static bool __thiscall ActivateThruster(IClientImpl* clientImpl, uint client, XActivateThrusters& info);
        static bool __thiscall SetTarget(IClientImpl* clientImpl, uint client, XSetTarget& st);
        static bool __thiscall EnterTradeLane(IClientImpl* clientImpl, uint client, XGoTradelane& tl);
        static bool __thiscall StopTradeLane(IClientImpl* clientImpl, uint client, uint shipID, uint archTradelane1, uint archTradelane2);
        static bool __thiscall JettisonCargo(IClientImpl* clientImpl, uint client, XJettisonCargo& jc);
        static bool __thiscall Login(IClientImpl* clientImpl, uint client, FLPACKET_UNKNOWN* unk);
        static bool __thiscall CharacterInformationReceived(IClientImpl* clientImpl, uint client, FLPACKET_UNKNOWN* unk);
        static bool __thiscall CharacterSelect(IClientImpl* clientImpl, uint client, FLPACKET_UNKNOWN* unk);
        static bool __thiscall AddItem(IClientImpl* clientImpl, uint client, FLPACKET_UNKNOWN*, FLPACKET_UNKNOWN* unk2);
        static bool __thiscall StartRoom(IClientImpl* clientImpl, uint client, uint, uint a2);
        static bool __thiscall DestroyCharacter(IClientImpl* clientImpl, uint client, uint, uint a2);
        static bool __thiscall UpdateCharacter(IClientImpl* clientImpl, uint client, uint, uint a2);
        static bool __thiscall SetReputation(IClientImpl* clientImpl, uint client, FLPACKET_SETREPUTATION& rep);
        static bool __thiscall Land(IClientImpl* clientImpl, uint client, FLPACKET_LAND& land);
        static bool __thiscall Launch(IClientImpl* clientImpl, uint client, FLPACKET_LAUNCH& launch);
        static bool __thiscall SystemSwitchOut(IClientImpl* clientImpl, uint client, FLPACKET_SYSTEM_SWITCH_OUT& out);
        static bool __thiscall JumpInComplete(IClientImpl* clientImpl, uint client, FLPACKET_SYSTEM_SWITCH_IN& in);
        static bool __thiscall ShipCreate(IClientImpl* clientImpl, uint client, bool response, uint shipId);
        static bool __thiscall DamageObject(IClientImpl* clientImpl, uint client, uint objId, DamageList& dmgList);
        static bool __thiscall ItemTractored(IClientImpl* clientImpl, uint client, uint itemId);
        static bool __thiscall CargoObjectUpdated(IClientImpl* clientImpl, SObjectCargoUpdate& cargoUpdate, uint dunno1, uint dunno2);
        static bool __thiscall FuseBurnStarted(IClientImpl* clientImpl, uint client, FLPACKET_BURNFUSE& burnFuse);
        static bool __thiscall WeaponGroupSet(IClientImpl* clientImpl, uint client, uint unk1, int);
        static bool __thiscall VisitStateSet(IClientImpl* clientImpl, uint client, uint objHash, int state);
        static bool __thiscall BestPathResponse(IClientImpl* clientImpl, uint objHash, uint client, int unk);
        static bool __thiscall PlayerInformation(IClientImpl* clientImpl, uint client, uint unk1, int);
        static bool __thiscall GroupPositionResponse(IClientImpl* clientImpl, uint client, uint unk1, int);
        static void __thiscall PlayerIsLeavingServer(IClientImpl* clientImpl, uint client, uint leavingClient);
        static bool __thiscall FormationUpdate(IClientImpl* clientImpl, uint client, uint shipId, Vector& formationOffset);
};
