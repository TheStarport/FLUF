#include "PCH.hpp"

#include "Internal/Hooks/ClientSend.hpp"

#include "ClientServerCommunicator.hpp"
#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "VTables.hpp"

void __thiscall ClientSend::FireWeapon(IServerImpl* serverImpl, uint client, XFireWeaponInfo& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XFireWeaponInfo&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeFireWeapon, client, info))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::FireWeapon)].oldFunc)(serverImpl, client, info);
    }
}

void __thiscall ClientSend::ActivateEquip(IServerImpl* serverImpl, uint client, XActivateEquip& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XActivateEquip&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeActivateEquip, client, info))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::ActivateEquip)].oldFunc)(serverImpl, client, info);
    }
}

void __thiscall ClientSend::ActivateCruise(IServerImpl* serverImpl, uint client, XActivateCruise& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XActivateCruise&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeActivateCruise, client, info))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::ActivateCruise)].oldFunc)(serverImpl, client, info);
    }
}

void __thiscall ClientSend::ActivateThruster(IServerImpl* serverImpl, uint client, XActivateThrusters& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XActivateThrusters&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeActivateThrusters, client, info))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::ActivateThruster)].oldFunc)(serverImpl, client, info);
    }
}

void __thiscall ClientSend::SetTarget(IServerImpl* serverImpl, uint client, XSetTarget& st)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XSetTarget&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, st))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::SetTarget)].oldFunc)(serverImpl, client, st);
    }
}

void __thiscall ClientSend::TractorObjects(IServerImpl* serverImpl, uint client, XTractorObjects& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XTractorObjects&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeTractorObjects, client, info))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::TractorObjects)].oldFunc)(serverImpl, client, info);
    }
}

void __thiscall ClientSend::EnterTradeLane(IServerImpl* serverImpl, uint client, XGoTradelane& tl)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XGoTradelane&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeEnterTradelane, client, tl))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::EnterTradeLane)].oldFunc)(serverImpl, client, tl);
    }
}

void __thiscall ClientSend::LeaveTradeLane(IServerImpl* serverImpl, uint client, uint shipId, uint archTradelane1, uint archTradelane2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeLeaveTradelane, client, shipId, archTradelane1, archTradelane2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::LeaveTradeLane)].oldFunc)(
            serverImpl, client, shipId, archTradelane1, archTradelane2);
    }
}

void __thiscall ClientSend::JettisonCargo(IServerImpl* serverImpl, uint client, XJettisonCargo& jc)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XJettisonCargo&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeJettisonCargo, client, jc))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::JettisonCargo)].oldFunc)(serverImpl, client, jc);
    }
}

void __thiscall ClientSend::Connect(IServerImpl* serverImpl, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeConnect, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::Connect)].oldFunc)(serverImpl, client);
    }
}

void __thiscall ClientSend::Disconnect(IServerImpl* serverImpl, uint client, EFLConnection connection)
{
    using FuncType = void(__thiscall*)(IServerImpl*, uint, EFLConnection);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeDisconnect, client, connection))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::Disconnect)].oldFunc)(serverImpl, client, connection);
    }
}

void __thiscall ClientSend::Login(IServerImpl* serverImpl, SLoginInfo& li, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SLoginInfo&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeLogin, li, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::Login)].oldFunc)(serverImpl, li, client);
    }
}

void __thiscall ClientSend::CharacterInfoRequest(IServerImpl* serverImpl, uint client, bool unk)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, bool);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeCharacterInfoRequest, client, unk))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::CharacterInfoRequest)].oldFunc)(serverImpl, client, unk);
    }
}

void __thiscall ClientSend::CharacterSelect(IServerImpl* serverImpl, CHARACTER_ID& cid, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, CHARACTER_ID & cid, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeCharacterSelect, cid, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::CharacterSelect)].oldFunc)(serverImpl, cid, client);
    }
}

void __thiscall ClientSend::RequestAddItem(IServerImpl* serverImpl, uint goodID, char* hardPoint, int count, float status, bool mounted, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, char*, int, float, bool, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeReqAddItem, goodID, hardPoint, count, status, mounted, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestAddItem)].oldFunc)(
            serverImpl, goodID, hardPoint, count, status, mounted, client);
    }
}

void __thiscall ClientSend::RequestRemoveItem(IServerImpl* serverImpl, ushort slotId, int count, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, ushort, int, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeReqRemoveItem, slotId, count, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestRemoveItem)].oldFunc)(
            serverImpl, slotId, count, client);
    }
}

void __thiscall ClientSend::RequestModifyItem(IServerImpl* serverImpl, ushort slotId, char* hardPoint, int count, float status, bool mounted,
                                              uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, ushort, char*, int, float, bool, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeReqModifyItem, slotId, hardPoint, count, status, mounted, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestModifyItem)].oldFunc)(
            serverImpl, slotId, hardPoint, count, status, mounted, client);
    }
}

void __thiscall ClientSend::RequestSetCash(IServerImpl* serverImpl, int newCash, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, int, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeReqSetCash, newCash, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestSetCash)].oldFunc)(serverImpl, newCash, client);
    }
}

void __thiscall ClientSend::RequestChangeCash(IServerImpl* serverImpl, int cashChange, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, int, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeReqChangeCash, cashChange, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestChangeCash)].oldFunc)(serverImpl, cashChange, client);
    }
}

void __thiscall ClientSend::BaseEnter(IServerImpl* serverImpl, uint baseId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeBaseEnter, baseId, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::BaseEnter)].oldFunc)(serverImpl, baseId, client);
    }
}

void __thiscall ClientSend::BaseExit(IServerImpl* serverImpl, uint baseId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeBaseExit, baseId, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::BaseExit)].oldFunc)(serverImpl, baseId, client);
    }
}

void __thiscall ClientSend::LocationEnter(IServerImpl* serverImpl, uint locationId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeLocationEnter, locationId, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::LocationEnter)].oldFunc)(serverImpl, locationId, client);
    }
}

void __thiscall ClientSend::LocationExit(IServerImpl* serverImpl, uint locationId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeLocationExit, locationId, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::LocationExit)].oldFunc)(serverImpl, locationId, client);
    }
}

void __thiscall ClientSend::DestroyCharacter(IServerImpl* serverImpl, CHARACTER_ID const& cid, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, CHARACTER_ID const&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeDestroyCharacter, cid, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::DestroyCharacter)].oldFunc)(serverImpl, cid, client);
    }
}

void __thiscall ClientSend::CreateNewCharacter(IServerImpl* serverImpl, SCreateCharacterInfo& info, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SCreateCharacterInfo&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeCreateNewCharacter, info, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::CreateNewCharacter)].oldFunc)(serverImpl, info, client);
    }
}

void __thiscall ClientSend::ObjectSelect(IServerImpl* serverImpl, unsigned int spaceId, unsigned int client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeObjectSelect, spaceId, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::ObjectSelect)].oldFunc)(serverImpl, spaceId, client);
    }
}

void __thiscall ClientSend::GoodVaporized(IServerImpl* serverImpl, SGFGoodVaporizedInfo& info, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SGFGoodVaporizedInfo&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeGoodVaporized, info, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::GoodVaporized)].oldFunc)(serverImpl, info, client);
    }
}

void __thiscall ClientSend::GoodBuy(IServerImpl* serverImpl, SGFGoodBuyInfo& info, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SGFGoodBuyInfo&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeGoodBuy, info, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::GoodBuy)].oldFunc)(serverImpl, info, client);
    }
}

void __thiscall ClientSend::GoodSell(IServerImpl* serverImpl, SGFGoodSellInfo& info, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SGFGoodSellInfo&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeGoodSell, info, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::GoodSell)].oldFunc)(serverImpl, info, client);
    }
}

void __thiscall ClientSend::TradeResponse(IServerImpl* serverImpl, unsigned char* unk1, int unk2, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, unsigned char*, int, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeTradeResponse, unk1, unk2, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::TradeResponse)].oldFunc)(serverImpl, unk1, unk2, client);
    }
}

void __thiscall ClientSend::PlayerLaunch(IServerImpl* serverImpl, uint baseId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforePlayerLaunch, baseId, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::PlayerLaunch)].oldFunc)(serverImpl, baseId, client);
    }
}

void __thiscall ClientSend::LaunchComplete(IServerImpl* serverImpl, uint baseId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeLaunchComplete, baseId, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::LaunchComplete)].oldFunc)(serverImpl, baseId, client);
    }
}

void __thiscall ClientSend::SystemSwitchOutComplete(IServerImpl* serverImpl, uint shipId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSystemSwitchOutComplete, shipId, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::SystemSwitchOutComplete)].oldFunc)(serverImpl, shipId, client);
    }
}

void __thiscall ClientSend::JumpInComplete(IServerImpl* serverImpl, uint systemId, uint shipId)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeJumpInComplete, systemId, shipId))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::JumpInComplete)].oldFunc)(serverImpl, systemId, shipId);
    }
}

void __thiscall ClientSend::Hail(IServerImpl* serverImpl, unsigned int unk1, unsigned int unk2, unsigned int unk3)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeHail, unk1, unk2, unk3))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::Hail)].oldFunc)(serverImpl, unk1, unk2, unk3);
    }
}

void __thiscall ClientSend::ObjectUpdate(IServerImpl* serverImpl, SSPObjUpdateInfo& ui, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SSPObjUpdateInfo & ui, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeObjectUpdate, ui, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::ObjectUpdate)].oldFunc)(serverImpl, ui, client);
    }
}

void __thiscall ClientSend::MunitionCollision(IServerImpl* serverImpl, SSPMunitionCollisionInfo& mci, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SSPMunitionCollisionInfo&, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeMunitionCollision, mci, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::MunitionCollision)].oldFunc)(serverImpl, mci, client);
    }
}

void __thiscall ClientSend::ObjectCollision(IServerImpl* serverImpl, SSPObjCollisionInfo& info, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SSPObjCollisionInfo&, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeObjectCollision, info, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::ObjectCollision)].oldFunc)(serverImpl, info, client);
    }
}

void __thiscall ClientSend::RequestEvent(IServerImpl* serverImpl, int eventType, uint shipId, uint dockTarget, uint unk1, ulong unk2, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, int eventType, uint shipID, uint dockTarget, uint, ulong, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestEvent, eventType, shipId, dockTarget, unk1, unk2, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestEvent)].oldFunc)(
            serverImpl, eventType, shipId, dockTarget, unk1, unk2, client);
    }
}

void __thiscall ClientSend::RequestCancel(IServerImpl* serverImpl, int eventType, uint shipId, uint unk1, ulong unk2, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, int, uint, uint, ulong, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestCancel, eventType, shipId, unk1, unk2, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestCancel)].oldFunc)(
            serverImpl, eventType, shipId, unk1, unk2, client);
    }
}

void __thiscall ClientSend::AbortMission(IServerImpl* serverImpl, uint client, uint unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uint unk2);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeAbortMission, client, unk2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::AbortMission)].oldFunc)(serverImpl, client, unk2);
    }
}

void __thiscall ClientSend::SetWeaponGroup(IServerImpl* serverImpl, uint client, uchar* unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uchar * unk1, uint unk2);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetWeaponGroup, client, unk1, unk2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::SetWeaponGroup)].oldFunc)(serverImpl, client, unk1, unk2);
    }
}

void __thiscall ClientSend::SetVisitedState(IServerImpl* serverImpl, uint client, uchar* unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uchar * unk1, uint unk2);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetVisitedState, client, unk1, unk2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::SetVisitedState)].oldFunc)(serverImpl, client, unk1, unk2);
    }
}

void __thiscall ClientSend::RequestBestPath(IServerImpl* serverImpl, uint client, uchar* unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uchar * unk1, uint unk2);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestBestPath, client, unk1, unk2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestBestPath)].oldFunc)(serverImpl, client, unk1, unk2);
    }
}

void __thiscall ClientSend::RequestPlayerStats(IServerImpl* serverImpl, uint client, uchar* unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uchar * unk1, uint unk2);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestPlayerStats, client, unk1, unk2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestPlayerStats)].oldFunc)(serverImpl, client, unk1, unk2);
    }
}

void __thiscall ClientSend::InitiateTrade(IServerImpl* serverImpl, uint client1, uint client2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeInitiateTrade, client1, client2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::InitiateTrade)].oldFunc)(serverImpl, client1, client2);
    }
}

void __thiscall ClientSend::TerminateTrade(IServerImpl* serverImpl, uint client, int accepted)
{
    using FuncType = void(__thiscall*)(IServerImpl*, uint client, int);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeTerminateTrade, client, accepted))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::TerminateTrade)].oldFunc)(serverImpl, client, accepted);
    }
}

void __thiscall ClientSend::AcceptTrade(IServerImpl* serverImpl, uint client, bool unk)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uint unk);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeAcceptTrade, client, unk))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::AcceptTrade)].oldFunc)(serverImpl, client, unk);
    }
}

void __thiscall ClientSend::SetTradeMoney(IServerImpl* serverImpl, uint client, ulong money)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uint unk);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTradeMoney, client, money))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::SetTradeMoney)].oldFunc)(serverImpl, client, money);
    }
}

void __thiscall ClientSend::AddTradeEquip(IServerImpl* serverImpl, uint client, EquipDesc& ed)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, EquipDesc & ed);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeAddTradeEquip, client, ed))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::AddTradeEquip)].oldFunc)(serverImpl, client, ed);
    }
}

void __thiscall ClientSend::DelTradeEquip(IServerImpl* serverImpl, uint client, EquipDesc& ed)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, EquipDesc&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRemoveTradeEquip, client, ed))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::DelTradeEquip)].oldFunc)(serverImpl, client, ed);
    }
}

void __thiscall ClientSend::RequestTrade(IServerImpl* serverImpl, uint unk1, uint unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestTrade, unk1, unk2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestTrade)].oldFunc)(serverImpl, unk1, unk2);
    }
}

void __thiscall ClientSend::StopTradeRequest(IServerImpl* serverImpl, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeStopTradeRequest, client))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::StopTradeRequest)].oldFunc)(serverImpl, client);
    }
}

void __thiscall ClientSend::RequestDifficultyScale(IServerImpl* serverImpl, float scale, uint unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, float, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestDifficultyScale, scale, unk2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::RequestDifficultyScale)].oldFunc)(serverImpl, scale, unk2);
    }
}

void __thiscall ClientSend::Dock(IServerImpl* serverImpl, uint& unk1, uint& unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint&, uint&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeDock, unk1, unk2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::Dock)].oldFunc)(serverImpl, unk1, unk2);
    }
}

/*
void __thiscall ClientSend::SubmitChat(IServerImpl* serverImpl, uint from, ulong size rdlReader, uint to, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, ulong, uint, int);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSubmitChat, from, size, rdlReader, to, unk2))
    {
        reinterpret_cast<FuncType>(Fluf::instance->serverPatches[static_cast<int>(IServerVTable::SubmitChat)].oldFunc)(
            serverImpl, from, size, rdlReader, to, unk2);
    }
}
*/
