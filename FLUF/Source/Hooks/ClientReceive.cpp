#include "PCH.hpp"

#include "Internal/Hooks/ClientReceive.hpp"

#include "ClientServerCommunicator.hpp"
#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "Internal/Hooks/InfocardOverrides.hpp"

bool __thiscall ClientReceive::FireWeapon(IClientImpl* clientImpl, uint client, XFireWeaponInfo& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XFireWeaponInfo&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::FireWeapon)].oldFunc)(clientImpl, client, info);

    Fluf::instance->CallModuleEvent(&FlufModule::OnFireWeapon, client, info);
    return result;
}

bool __thiscall ClientReceive::ActivateEquip(IClientImpl* clientImpl, uint client, XActivateEquip& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XActivateEquip&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::ActivateEquip)].oldFunc)(clientImpl, client, info);

    Fluf::instance->CallModuleEvent(&FlufModule::OnActivateEquip, client, info);
    return result;
}

bool __thiscall ClientReceive::ActivateCruise(IClientImpl* clientImpl, uint client, XActivateCruise& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XActivateCruise&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::ActivateCruise)].oldFunc)(clientImpl, client, info);

    Fluf::instance->CallModuleEvent(&FlufModule::OnActivateCruise, client, info);
    return result;
}

bool __thiscall ClientReceive::ActivateThruster(IClientImpl* clientImpl, uint client, XActivateThrusters& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XActivateThrusters&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::ActivateThruster)].oldFunc)(clientImpl, client, info);

    Fluf::instance->CallModuleEvent(&FlufModule::OnActivateThruster, client, info);
    return result;
}

bool __thiscall ClientReceive::SetTarget(IClientImpl* clientImpl, uint client, XSetTarget& st)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XSetTarget&);
    const auto result = reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::SetTarget)].oldFunc)(clientImpl, client, st);

    Fluf::instance->CallModuleEvent(&FlufModule::OnSetTarget, client, st);
    return result;
}

bool __thiscall ClientReceive::EnterTradeLane(IClientImpl* clientImpl, uint client, XGoTradelane& tl)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XGoTradelane&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::EnterTradeLane)].oldFunc)(clientImpl, client, tl);

    Fluf::instance->CallModuleEvent(&FlufModule::BeforeEnterTradelane, client, tl);
    return result;
}

bool __thiscall ClientReceive::StopTradeLane(IClientImpl* clientImpl, uint client, uint shipID, uint archTradelane1, uint archTradelane2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint, uint);
    const auto result = reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::LeaveTradeLane)].oldFunc)(
        clientImpl, client, shipID, archTradelane1, archTradelane2);
    Fluf::instance->CallModuleEvent(&FlufModule::OnStopTradeLane, client, shipID, archTradelane1, archTradelane2);
    return result;
}

bool __thiscall ClientReceive::JettisonCargo(IClientImpl* clientImpl, uint client, XJettisonCargo& jc)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, XJettisonCargo&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::JettisonCargo)].oldFunc)(clientImpl, client, jc);
    Fluf::instance->CallModuleEvent(&FlufModule::OnJettisonCargo, client, jc);
    return result;
}

bool __thiscall ClientReceive::Login(IClientImpl* clientImpl, uint client, FLPACKET_UNKNOWN* unk)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    InfocardOverrides::ClearOverrides();

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*);
    const auto result = reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::Login)].oldFunc)(clientImpl, client, unk);

    if (!SinglePlayer())
    {
        constexpr std::string_view header = "fluf";

        struct FlufMetadata
        {
                DWORD infocardIndex;
        };

        const FlufMetadata metadata{
            .infocardIndex = Fluf::startingResourceIndex,
        };

        Fluf::GetClientServerCommunicator()->SendPayloadFromClient(header, metadata);
    }

    Fluf::instance->CallModuleEvent(&FlufModule::OnLogin, client, SinglePlayer(), unk);
    return result;
}

bool __thiscall ClientReceive::CharacterInformationReceived(IClientImpl* clientImpl, uint client, FLPACKET_UNKNOWN* unk)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::CharacterInfo)].oldFunc)(clientImpl, client, unk);

    Fluf::instance->CallModuleEvent(&FlufModule::OnCharacterInformationReceived, client, unk);
    return result;
}

bool __thiscall ClientReceive::CharacterSelect(IClientImpl* clientImpl, uint client, FLPACKET_UNKNOWN* unk)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::CharacterSelect)].oldFunc)(clientImpl, client, unk);

    Fluf::instance->CallModuleEvent(&FlufModule::OnCharacterSelect, client, unk);
    return result;
}

bool __thiscall ClientReceive::AddItem(IClientImpl* clientImpl, uint client, FLPACKET_UNKNOWN* unk1, FLPACKET_UNKNOWN* unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_UNKNOWN*, FLPACKET_UNKNOWN*);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::AddItemToCharacter)].oldFunc)(clientImpl, client, unk1, unk2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnAddItem, client, unk1, unk2);
    return result;
}

bool __thiscall ClientReceive::StartRoom(IClientImpl* clientImpl, uint client, uint a1, uint a2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::SetStartRoom)].oldFunc)(clientImpl, client, a1, a2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnStartRoom, client, a1, a2);
    return result;
}

bool __thiscall ClientReceive::DestroyCharacter(IClientImpl* clientImpl, uint client, uint a1, uint a2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::DestroyCharacter)].oldFunc)(clientImpl, client, a1, a2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnDestroyCharacter, client, a1, a2);
    return result;
}

bool __thiscall ClientReceive::UpdateCharacter(IClientImpl* clientImpl, uint client, uint a1, uint a2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, uint);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::UpdateCharacter)].oldFunc)(clientImpl, client, a1, a2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnUpdateCharacter, client, a1, a2);
    return result;
}

bool __thiscall ClientReceive::SetReputation(IClientImpl* clientImpl, uint client, FLPACKET_SETREPUTATION& rep)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_SETREPUTATION&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::SetReputation)].oldFunc)(clientImpl, client, rep);

    Fluf::instance->CallModuleEvent(&FlufModule::OnSetReputation, client, rep);
    return result;
}

bool __thiscall ClientReceive::Land(IClientImpl* clientImpl, uint client, FLPACKET_LAND& land)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_LAND&);
    const auto result = reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::Land)].oldFunc)(clientImpl, client, land);

    Fluf::instance->CallModuleEvent(&FlufModule::OnLand, client, land);
    return result;
}

bool __thiscall ClientReceive::Launch(IClientImpl* clientImpl, uint client, FLPACKET_LAUNCH& launch)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_LAUNCH&);
    const auto result = reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::Launch)].oldFunc)(clientImpl, client, launch);

    Fluf::instance->CallModuleEvent(&FlufModule::OnLaunch, client, launch);
    return result;
}

bool __thiscall ClientReceive::SystemSwitchOut(IClientImpl* clientImpl, uint client, FLPACKET_SYSTEM_SWITCH_OUT& out)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_SYSTEM_SWITCH_OUT&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::SystemSwitchOut)].oldFunc)(clientImpl, client, out);

    Fluf::instance->CallModuleEvent(&FlufModule::OnSystemSwitchOut, client, out);
    return result;
}

bool __thiscall ClientReceive::JumpInComplete(IClientImpl* clientImpl, uint client, FLPACKET_SYSTEM_SWITCH_IN& in)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_SYSTEM_SWITCH_IN&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::JumpInComplete)].oldFunc)(clientImpl, client, in);

    Fluf::instance->CallModuleEvent(&FlufModule::OnJumpInComplete, client, in);
    return result;
}

bool __thiscall ClientReceive::ShipCreate(IClientImpl* clientImpl, uint client, bool response, uint shipId)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, bool, uint);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::CreateShip)].oldFunc)(clientImpl, client, response, shipId);
    Fluf::instance->CallModuleEvent(&FlufModule::OnShipCreate, client, response, shipId);
    return result;
}

bool __thiscall ClientReceive::DamageObject(IClientImpl* clientImpl, uint client, uint objId, DamageList& dmgList)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, DamageList&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::DamageObject)].oldFunc)(clientImpl, client, objId, dmgList);

    Fluf::instance->CallModuleEvent(&FlufModule::OnDamageObject, client, objId, dmgList);
    return result;
}

bool __thiscall ClientReceive::ItemTractored(IClientImpl* clientImpl, uint client, uint itemId)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::ItemTractored)].oldFunc)(clientImpl, client, itemId);

    Fluf::instance->CallModuleEvent(&FlufModule::OnItemTractored, client, itemId);
    return result;
}

bool __thiscall ClientReceive::CargoObjectUpdated(IClientImpl* clientImpl, SObjectCargoUpdate& cargoUpdate, uint dunno1, uint dunno2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, SObjectCargoUpdate&, uint, uint);
    const auto result = reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::ObjectCargoUpdate)].oldFunc)(
        clientImpl, cargoUpdate, dunno1, dunno2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnCargoObjectUpdated, cargoUpdate, dunno1, dunno2);
    return result;
}

bool __thiscall ClientReceive::FuseBurnStarted(IClientImpl* clientImpl, uint client, FLPACKET_BURNFUSE& burnFuse)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, FLPACKET_BURNFUSE&);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::BeginFuse)].oldFunc)(clientImpl, client, burnFuse);

    Fluf::instance->CallModuleEvent(&FlufModule::OnFuseBurnStarted, client, burnFuse);
    return result;
}

bool __thiscall ClientReceive::WeaponGroupSet(IClientImpl* clientImpl, uint client, uint unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::SetWeaponGroup)].oldFunc)(clientImpl, client, unk1, unk2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnWeaponGroupSet, client, unk1, unk2);
    return result;
}

bool __thiscall ClientReceive::VisitStateSet(IClientImpl* clientImpl, uint client, uint objHash, int state)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::SetVisitState)].oldFunc)(clientImpl, client, objHash, state);

    Fluf::instance->CallModuleEvent(&FlufModule::OnVisitStateSet, client, objHash, state);
    return result;
}

bool __thiscall ClientReceive::BestPathResponse(IClientImpl* clientImpl, uint client, uint objHash, int unk)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::RequestBestPath)].oldFunc)(clientImpl, client, objHash, unk);

    Fluf::instance->CallModuleEvent(&FlufModule::OnBestPathResponse, client, objHash, unk);
    return result;
}

bool __thiscall ClientReceive::PlayerInformation(IClientImpl* clientImpl, uint client, uint unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    const auto result =
        reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::RequestPlayerStats)].oldFunc)(clientImpl, client, unk1, unk2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnPlayerInformation, client, unk1, unk2);
    return result;
}

bool __thiscall ClientReceive::GroupPositionResponse(IClientImpl* clientImpl, uint client, uint unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, int);
    const auto result = reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::RequestGroupPositions)].oldFunc)(
        clientImpl, client, unk1, unk2);

    Fluf::instance->CallModuleEvent(&FlufModule::OnGroupPositionResponse, client, unk1, unk2);
    return result;
}

void __thiscall ClientReceive::PlayerIsLeavingServer(IClientImpl* clientImpl, uint client, uint leavingClient)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IClientImpl*, uint, uint);
    reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::PlayerIsLeavingServer)].oldFunc)(
        clientImpl, client, leavingClient);

    Fluf::instance->CallModuleEvent(&FlufModule::OnPlayerLeavingServer, client, leavingClient);
}

bool __thiscall ClientReceive::FormationUpdate(IClientImpl* clientImpl, uint client, uint shipId, Vector& formationOffset)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = bool(__thiscall*)(IClientImpl*, uint, uint, Vector&);
    const auto result = reinterpret_cast<FuncType>(Fluf::instance->clientPatches[static_cast<int>(IClientVTable::FormationUpdate)].oldFunc)(
        clientImpl, client, shipId, formationOffset);

    Fluf::instance->CallModuleEvent(&FlufModule::OnFormationUpdate, client, shipId, formationOffset);
    return result;
}
