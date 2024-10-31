#include "PCH.hpp"

#include "Internal/Hooks/ClientSend.hpp"

/*bool __fastcall ClientSend::FireWeapon(IClientImpl* clientImpl, void*, uint client, XFireWeaponInfo& info)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XFireWeaponInfo&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeFireWeapon, client, info)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::FireWeapon)))(clientImpl, client, info)
               : false;
}

bool __fastcall ClientSend::ActivateEquip(IClientImpl* clientImpl, void*, uint client, XActivateEquip& info)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XActivateEquip&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeActivateEquip, client, info)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::ActivateEquip)))(clientImpl, client, info)
               : false;
}

bool __fastcall ClientSend::ActivateCruise(IClientImpl* clientImpl, void*, uint client, XActivateCruise& info)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XActivateCruise&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeActivateCruise, client, info)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::ActivateCruise)))(clientImpl, client, info)
               : false;
}

bool __fastcall ClientSend::ActivateThruster(IClientImpl* clientImpl, void*, uint client, XActivateThrusters& info)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XActivateThrusters&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeActivateThrusters, client, info)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::ActivateThruster)))(
                     clientImpl, client, info)
               : false;
}

bool __fastcall ClientSend::SetTarget(IClientImpl* clientImpl, void*, uint client, XSetTarget& st)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XSetTarget&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, st)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SetTarget)))(clientImpl, client, st)
               : false;
}

bool __fastcall ClientSend::EnterTradeLane(IClientImpl* clientImpl, void*, uint client, XGoTradelane& tl)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XGoTradelane&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeEnterTradelane, client, tl)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::EnterTradeLane)))(clientImpl, client, tl)
               : false;
}

bool __fastcall ClientSend::StopTradeLane(IClientImpl* clientImpl, void*, uint client, uint shipID, uint archTradelane1, uint archTradelane2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint, uint);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeLeaveTradelane, client, shipID, archTradelane1, archTradelane2)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::LeaveTradeLane)))(
                     clientImpl, client, shipID, archTradelane1, archTradelane2)
               : false;
}

bool __fastcall ClientSend::JettisonCargo(IClientImpl* clientImpl, void*, uint client, XJettisonCargo& jc)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XJettisonCargo&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeJettisonCargo, client, jc)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::JettisonCargo)))(clientImpl, client, jc)
               : false;
}

bool __fastcall ClientSend::Login(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN* unk)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeLogin, client, unk)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::Login)))(clientImpl, client, unk)
               : false;
}

bool __fastcall ClientSend::CharacterInformationReceived(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN* unk)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*);
    return Fluf::instance->CallModuleEvent(&FlufModule::OnCharacterInformationReceived, client, unk)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::CharacterInfo)))(clientImpl, client, unk)
               : false;
}

bool __fastcall ClientSend::CharacterSelect(IClientImpl* clientImpl, void*, uint client, FLPACKET_UNKNOWN* unk)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, unk)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::CharacterSelect)))(clientImpl, client, unk)
               : false;
}

bool __fastcall ClientSend::AddItem(IClientImpl* clientImpl, uint client, void*, FLPACKET_UNKNOWN*, FLPACKET_UNKNOWN* unk)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, unk)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::AddItemToCharacter)))(
                     clientImpl, client, unk)
               : false;
}

bool __fastcall ClientSend::StartRoom(IClientImpl* clientImpl, void*, uint client, uint a1, uint a2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, a1, a2)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SetStartRoom)))(clientImpl, client, a1, a2)
               : false;
}

bool __fastcall ClientSend::DestroyCharacter(IClientImpl* clientImpl, void*, uint client, uint a1, uint a2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, a1, a2)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::DestroyCharacter)))(
                     clientImpl, client, a1, a2)
               : false;
}

bool __fastcall ClientSend::UpdateCharacter(IClientImpl* clientImpl, void*, uint client, uint a1, uint a2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, a1, a2)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::UpdateCharacter)))(
                     clientImpl, client, a1, a2)
               : false;
}

bool __fastcall ClientSend::SetReputation(IClientImpl* clientImpl, void*, uint client, FLPACKET_SETREPUTATION& rep)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_SETREPUTATION&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, rep)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SetReputation)))(clientImpl, client, rep)
               : false;
}

bool __fastcall ClientSend::Land(IClientImpl* clientImpl, void*, uint client, FLPACKET_LAND& land)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_LAND&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, land)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::Land)))(clientImpl, client, land)
               : false;
}

bool __fastcall ClientSend::Launch(IClientImpl* clientImpl, void*, uint client, FLPACKET_LAUNCH& launch)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_LAUNCH&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, launch)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::Launch)))(clientImpl, client, launch)
               : false;
}

bool __fastcall ClientSend::SystemSwitchOut(IClientImpl* clientImpl, void*, uint client, FLPACKET_SYSTEM_SWITCH_OUT& out)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_SYSTEM_SWITCH_OUT&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, out)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SystemSwitchOut)))(clientImpl, client, out)
               : false;
}

bool __fastcall ClientSend::JumpInComplete(IClientImpl* clientImpl, void*, uint client, FLPACKET_SYSTEM_SWITCH_IN& in)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_SYSTEM_SWITCH_IN&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, in)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::JumpInComplete)))(clientImpl, client, in)
               : false;
}

bool __fastcall ClientSend::ShipCreate(IClientImpl* clientImpl, void*, uint client, bool response, uint shipId)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, bool, uint);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, response, shipId)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::CreateShip)))(
                     clientImpl, client, response, shipId)
               : false;
}

bool __fastcall ClientSend::DamageObject(IClientImpl* clientImpl, void*, uint client, uint objId, DamageList& dmgList)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, DamageList&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, objId, dmgList)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::DamageObject)))(
                     clientImpl, client, objId, dmgList)
               : false;
}

bool __fastcall ClientSend::ItemTractored(IClientImpl* clientImpl, void*, uint client, uint itemId)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, itemId)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::ItemTractored)))(clientImpl, client, itemId)
               : false;
}

bool __fastcall ClientSend::CargoObjectUpdated(IClientImpl* clientImpl, void*, SObjectCargoUpdate& cargoUpdate, uint dunno1, uint dunno2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, SObjectCargoUpdate&, uint, uint);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, cargoUpdate, dunno1, dunno2)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::ObjectCargoUpdate)))(
                     clientImpl, cargoUpdate, dunno1, dunno2)
               : false;
}

bool __fastcall ClientSend::FuseBurnStarted(IClientImpl* clientImpl, void*, uint client, FLPACKET_BURNFUSE& burnFuse)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_BURNFUSE&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, burnFuse)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::BeginFuse)))(clientImpl, client, burnFuse)
               : false;
}

bool __fastcall ClientSend::WeaponGroupSet(IClientImpl* clientImpl, void*, uint client, uint unk1, int unk2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, unk1, unk2)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SetWeaponGroup)))(
                     clientImpl, client, unk1, unk2)
               : false;
}

bool __fastcall ClientSend::VisitStateSet(IClientImpl* clientImpl, void*, uint client, uint objHash, int state)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, objHash, state)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::SetVisitState)))(
                     clientImpl, client, objHash, state)
               : false;
}

bool __fastcall ClientSend::BestPathResponse(IClientImpl* clientImpl, void*, uint client, uint objHash, int unk)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, objHash, unk)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::RequestBestPath)))(
                     clientImpl, client, objHash, unk)
               : false;
}

bool __fastcall ClientSend::PlayerInformation(IClientImpl* clientImpl, void*, uint client, uint unk1, int unk2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, unk1, unk2)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::RequestPlayerStats)))(
                     clientImpl, client, unk1, unk2)
               : false;
}

bool __fastcall ClientSend::GroupPositionResponse(IClientImpl* clientImpl, void*, uint client, uint unk1, int unk2)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, unk1, unk2)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::RequestGroupPositions)))(
                     clientImpl, client, unk1, unk2)
               : false;
}

bool __fastcall ClientSend::PlayerIsLeavingServer(IClientImpl* clientImpl, void*, uint client, uint leavingClient)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, leavingClient)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::PlayerIsLeavingServer)))(
                     clientImpl, client, leavingClient)
               : false;
}

bool __fastcall ClientSend::FormationUpdate(IClientImpl* clientImpl, void*, uint client, uint shipId, Vector& formationOffset)
{
    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, Vector&);
    return Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, shipId, formationOffset)
               ? static_cast<FuncType>(Fluf::instance->clientVTable->GetOriginal(static_cast<ushort>(IClientVTable::FormationUpdate)))(
                     clientImpl, client, shipId, formationOffset)
               : false;
}
*/
