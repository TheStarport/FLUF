#include "PCH.hpp"

#include "Internal/Hooks/ClientReceive.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

bool __fastcall ClientReceive::FireWeapon(IClientImpl* clientImpl, void*, uint client, XFireWeaponInfo& info)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XFireWeaponInfo&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::FireWeapon)))(clientImpl, client, info);

    Fluf::instance->CallModuleEvent(&FlufModule::OnFireWeapon, client, info);
    return result;
}

bool __fastcall ClientReceive::ActivateEquip(IClientImpl* clientImpl, void*, uint client, XActivateEquip& info)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XActivateEquip&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::ActivateEquip)))(clientImpl, client, info);

    Fluf::instance->CallModuleEvent(&FlufModule::OnActivateEquip, client, info);
    return result;
}

bool __fastcall ClientReceive::ActivateCruise(IClientImpl* clientImpl, void*, uint client, XActivateCruise& info)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XActivateCruise&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::ActivateCruise)))(clientImpl, client, info);

    Fluf::instance->CallModuleEvent(&FlufModule::OnActivateCruise, client, info);
    return result;
}

bool __fastcall ClientReceive::ActivateThruster(IClientImpl* clientImpl, void*, uint client, XActivateThrusters& info)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XActivateThrusters&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::ActivateThruster)))(clientImpl, client, info);

    Fluf::instance->CallModuleEvent(&FlufModule::OnActivateThruster, client, info);
    return result;
}

bool __fastcall ClientReceive::SetTarget(IClientImpl* clientImpl, void*, uint client, XSetTarget& st)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XSetTarget&);
    const auto result = static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SetTarget)))(clientImpl, client, st);

    Fluf::instance->CallModuleEvent(&FlufModule::OnSetTarget, client, st);
    return result;
}

bool __fastcall ClientReceive::EnterTradeLane(IClientImpl* clientImpl, void*, uint client, XGoTradelane& tl)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XGoTradelane&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::EnterTradeLane)))(clientImpl, client, tl);

    Fluf::instance->CallModuleEvent(&FlufModule::BeforeEnterTradelane, client, tl);
    return result;
}

bool __fastcall ClientReceive::StopTradeLane(IClientImpl* clientImpl, void*, uint client, uint shipID, uint archTradelane1, uint archTradelane2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint, uint);
    const auto result = static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::LeaveTradeLane)))(
        clientImpl, client, shipID, archTradelane1, archTradelane2);
    Fluf::instance->CallModuleEvent(&FlufModule::OnStopTradeLane, client, shipID, archTradelane1, archTradelane2);
    return result;
}

bool __fastcall ClientReceive::JettisonCargo(IClientImpl* clientImpl, void*, uint client, XJettisonCargo& jc)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XJettisonCargo&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::JettisonCargo)))(clientImpl, client, jc);
    Fluf::instance->CallModuleEvent(&FlufModule::OnJettisonCargo, client, jc);
    return result;
}

bool __fastcall ClientReceive::Login(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN* unk)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*);
    const auto result = static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::Login)))(clientImpl, client, unk);

    Fluf::instance->CallModuleEvent(&FlufModule::OnLogin, client, unk);
    return result;
}

bool __fastcall ClientReceive::CharacterInformationReceived(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN* unk)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::CharacterInfo)))(clientImpl, client, unk);

    Fluf::instance->CallModuleEvent(&FlufModule::OnCharacterInformationReceived, client, unk);
    return result;
}

bool __fastcall ClientReceive::CharacterSelect(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN* unk)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::CharacterSelect)))(clientImpl, client, unk);

    Fluf::instance->CallModuleEvent(&FlufModule::OnCharacterSelect, client, unk);
    return result;
}

bool __fastcall ClientReceive::AddItem(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN* unk1, FLPACKET_UNKNOWN* unk2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*, FLPACKET_UNKNOWN*);
    const auto result = static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::AddItemToCharacter)))(
        clientImpl, client, unk1, unk2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnAddItem, client, unk1, unk2);
    return result;
}

bool __fastcall ClientReceive::StartRoom(IClientImpl* clientImpl, void*, uint client, uint a1, uint a2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SetStartRoom)))(clientImpl, client, a1, a2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnStartRoom, client, a1, a2);
    return result;
}

bool __fastcall ClientReceive::DestroyCharacter(IClientImpl* clientImpl, void*, uint client, uint a1, uint a2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::DestroyCharacter)))(clientImpl, client, a1, a2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnDestroyCharacter, client, a1, a2);
    return result;
}

bool __fastcall ClientReceive::UpdateCharacter(IClientImpl* clientImpl, void*, uint client, uint a1, uint a2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::UpdateCharacter)))(clientImpl, client, a1, a2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnUpdateCharacter, client, a1, a2);
    return result;
}

bool __fastcall ClientReceive::SetReputation(IClientImpl* clientImpl, void*, uint client, FLPACKET_SETREPUTATION& rep)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_SETREPUTATION&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SetReputation)))(clientImpl, client, rep);

    Fluf::instance->CallModuleEvent(&FlufModule::OnSetReputation, client, rep);
    return result;
}

bool __fastcall ClientReceive::Land(IClientImpl* clientImpl, void*, uint client, FLPACKET_LAND& land)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_LAND&);
    const auto result = static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::Land)))(clientImpl, client, land);

    Fluf::instance->CallModuleEvent(&FlufModule::OnLand, client, land);
    return result;
}

bool __fastcall ClientReceive::Launch(IClientImpl* clientImpl, void*, uint client, FLPACKET_LAUNCH& launch)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_LAUNCH&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::Launch)))(clientImpl, client, launch);

    Fluf::instance->CallModuleEvent(&FlufModule::OnLaunch, client, launch);
    return result;
}

bool __fastcall ClientReceive::SystemSwitchOut(IClientImpl* clientImpl, void*, uint client, FLPACKET_SYSTEM_SWITCH_OUT& out)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_SYSTEM_SWITCH_OUT&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SystemSwitchOut)))(clientImpl, client, out);

    Fluf::instance->CallModuleEvent(&FlufModule::OnSystemSwitchOut, client, out);
    return result;
}

bool __fastcall ClientReceive::JumpInComplete(IClientImpl* clientImpl, void*, uint client, FLPACKET_SYSTEM_SWITCH_IN& in)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_SYSTEM_SWITCH_IN&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::JumpInComplete)))(clientImpl, client, in);

    Fluf::instance->CallModuleEvent(&FlufModule::OnJumpInComplete, client, in);
    return result;
}

bool __fastcall ClientReceive::ShipCreate(IClientImpl* clientImpl, void*, uint client, bool response, uint shipId)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, bool, uint);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::CreateShip)))(clientImpl, client, response, shipId);
    Fluf::instance->CallModuleEvent(&FlufModule::OnShipCreate, client, response, shipId);
    return result;
}

bool __fastcall ClientReceive::DamageObject(IClientImpl* clientImpl, void*, uint client, uint objId, DamageList& dmgList)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, DamageList&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::DamageObject)))(clientImpl, client, objId, dmgList);

    Fluf::instance->CallModuleEvent(&FlufModule::OnDamageObject, client, objId, dmgList);
    return result;
}

bool __fastcall ClientReceive::ItemTractored(IClientImpl* clientImpl, void*, uint client, uint itemId)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::ItemTractored)))(clientImpl, client, itemId);

    Fluf::instance->CallModuleEvent(&FlufModule::OnItemTractored, client, itemId);
    return result;
}

bool __fastcall ClientReceive::CargoObjectUpdated(IClientImpl* clientImpl, void*, SObjectCargoUpdate& cargoUpdate, uint dunno1, uint dunno2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, SObjectCargoUpdate&, uint, uint);
    const auto result = static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::ObjectCargoUpdate)))(
        clientImpl, cargoUpdate, dunno1, dunno2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnCargoObjectUpdated, cargoUpdate, dunno1, dunno2);
    return result;
}

bool __fastcall ClientReceive::FuseBurnStarted(IClientImpl* clientImpl, void*, uint client, FLPACKET_BURNFUSE& burnFuse)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_BURNFUSE&);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::BeginFuse)))(clientImpl, client, burnFuse);

    Fluf::instance->CallModuleEvent(&FlufModule::OnFuseBurnStarted, client, burnFuse);
    return result;
}

bool __fastcall ClientReceive::WeaponGroupSet(IClientImpl* clientImpl, void*, uint client, uint unk1, int unk2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SetWeaponGroup)))(clientImpl, client, unk1, unk2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnWeaponGroupSet, client, unk1, unk2);
    return result;
}

bool __fastcall ClientReceive::VisitStateSet(IClientImpl* clientImpl, void*, uint client, uint objHash, int state)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SetVisitState)))(clientImpl, client, objHash, state);

    Fluf::instance->CallModuleEvent(&FlufModule::OnVisitStateSet, client, objHash, state);
    return result;
}

bool __fastcall ClientReceive::BestPathResponse(IClientImpl* clientImpl, void*, uint client, uint objHash, int unk)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    const auto result =
        static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::RequestBestPath)))(clientImpl, client, objHash, unk);

    Fluf::instance->CallModuleEvent(&FlufModule::OnBestPathResponse, client, objHash, unk);
    return result;
}

bool __fastcall ClientReceive::PlayerInformation(IClientImpl* clientImpl, void*, uint client, uint unk1, int unk2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    const auto result = static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::RequestPlayerStats)))(
        clientImpl, client, unk1, unk2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnPlayerInformation, client, unk1, unk2);
    return result;
}

bool __fastcall ClientReceive::GroupPositionResponse(IClientImpl* clientImpl, void*, uint client, uint unk1, int unk2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    const auto result = static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::RequestGroupPositions)))(
        clientImpl, client, unk1, unk2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnGroupPositionResponse, client, unk1, unk2);
    return result;
}

bool __fastcall ClientReceive::PlayerIsLeavingServer(IClientImpl* clientImpl, void*, uint client, uint leavingClient)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint);
    const auto result = static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::PlayerIsLeavingServer)))(
        clientImpl, client, leavingClient);

    Fluf::instance->CallModuleEvent(&FlufModule::OnPlayerLeavingServer, client, leavingClient);
    return result;
}

bool __fastcall ClientReceive::FormationUpdate(IClientImpl* clientImpl, void*, uint client, uint shipId, Vector& formationOffset)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, Vector&);
    const auto result = static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::FormationUpdate)))(
        clientImpl, client, shipId, formationOffset);

    Fluf::instance->CallModuleEvent(&FlufModule::OnFormationUpdate, client, shipId, formationOffset);
    return result;
}
