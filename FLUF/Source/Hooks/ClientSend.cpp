#include "PCH.hpp"

#include "Internal/Hooks/ClientSend.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "VTables.hpp"

void __fastcall ClientSend::FireWeapon(IServerImpl* serverImpl, void*, uint client, XFireWeaponInfo& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XFireWeaponInfo&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeFireWeapon, client, info))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::FireWeapon)))(serverImpl, client, info);
    }
}

void __fastcall ClientSend::ActivateEquip(IServerImpl* serverImpl, void*, uint client, XActivateEquip& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XActivateEquip&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeActivateEquip, client, info))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::ActivateEquip)))(serverImpl, client, info);
    }
}

void __fastcall ClientSend::ActivateCruise(IServerImpl* serverImpl, void*, uint client, XActivateCruise& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XActivateCruise&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeActivateCruise, client, info))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::ActivateCruise)))(serverImpl, client, info);
    }
}

void __fastcall ClientSend::ActivateThruster(IServerImpl* serverImpl, void*, uint client, XActivateThrusters& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XActivateThrusters&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeActivateThrusters, client, info))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::ActivateThruster)))(serverImpl, client, info);
    }
}

void __fastcall ClientSend::SetTarget(IServerImpl* serverImpl, void*, uint client, XSetTarget& st)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XSetTarget&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTarget, client, st))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::SetTarget)))(serverImpl, client, st);
    }
}

void __fastcall ClientSend::TractorObjects(IServerImpl* serverImpl, void*, uint client, XTractorObjects& info)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XTractorObjects&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeTractorObjects, client, info))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::TractorObjects)))(serverImpl, client, info);
    }
}

void __fastcall ClientSend::EnterTradeLane(IServerImpl* serverImpl, void*, uint client, XGoTradelane& tl)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XGoTradelane&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeEnterTradelane, client, tl))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::EnterTradeLane)))(serverImpl, client, tl);
    }
}

void __fastcall ClientSend::LeaveTradeLane(IServerImpl* serverImpl, void*, uint client, uint shipId, uint archTradelane1, uint archTradelane2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeLeaveTradelane, client, shipId, archTradelane1, archTradelane2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::LeaveTradeLane)))(
            serverImpl, client, shipId, archTradelane1, archTradelane2);
    }
}

void __fastcall ClientSend::JettisonCargo(IServerImpl* serverImpl, void*, uint client, XJettisonCargo& jc)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, XJettisonCargo&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeJettisonCargo, client, jc))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::JettisonCargo)))(serverImpl, client, jc);
    }
}

void __fastcall ClientSend::Connect(IServerImpl* serverImpl, void*, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeConnect, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::Connect)))(serverImpl, client);
    }
}

void __fastcall ClientSend::Disconnect(IServerImpl* serverImpl, void*, uint client, EFLConnection connection)
{
    using FuncType = void(__thiscall*)(IServerImpl*, uint, EFLConnection);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeDisconnect, client, connection))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::Disconnect)))(serverImpl, client, connection);
    }
}

void __fastcall ClientSend::Login(IServerImpl* serverImpl, void*, SLoginInfo& li, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SLoginInfo&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeLogin, li, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::Login)))(serverImpl, li, client);
    }
}

void __fastcall ClientSend::CharacterInfoRequest(IServerImpl* serverImpl, void*, uint client, bool unk)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, bool);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeCharacterInfoRequest, client, unk))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::CharacterInfoRequest)))(
            serverImpl, client, unk);
    }
}

void __fastcall ClientSend::CharacterSelect(IServerImpl* serverImpl, void*, CHARACTER_ID& cid, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, CHARACTER_ID & cid, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeCharacterSelect, cid, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::CharacterSelect)))(serverImpl, cid, client);
    }
}

void __fastcall ClientSend::RequestAddItem(IServerImpl* serverImpl, void*, uint goodID, char* hardPoint, int count, float status, bool mounted, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, char*, int, float, bool, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeReqAddItem, goodID, hardPoint, count, status, mounted, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestAddItem)))(
            serverImpl, goodID, hardPoint, count, status, mounted, client);
    }
}

void __fastcall ClientSend::RequestRemoveItem(IServerImpl* serverImpl, void*, ushort slotId, int count, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, ushort, int, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeReqRemoveItem, slotId, count, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestRemoveItem)))(
            serverImpl, slotId, count, client);
    }
}

void __fastcall ClientSend::RequestModifyItem(IServerImpl* serverImpl, void*, ushort slotId, char* hardPoint, int count, float status, bool mounted,
                                              uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, ushort, char*, int, float, bool, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeReqModifyItem, slotId, hardPoint, count, status, mounted, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestModifyItem)))(
            serverImpl, slotId, hardPoint, count, status, mounted, client);
    }
}

void __fastcall ClientSend::RequestSetCash(IServerImpl* serverImpl, void*, int newCash, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, int, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeReqSetCash, newCash, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestSetCash)))(serverImpl, newCash, client);
    }
}

void __fastcall ClientSend::RequestChangeCash(IServerImpl* serverImpl, void*, int cashChange, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, int, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeReqChangeCash, cashChange, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestChangeCash)))(
            serverImpl, cashChange, client);
    }
}

void __fastcall ClientSend::BaseEnter(IServerImpl* serverImpl, void*, uint baseId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeBaseEnter, baseId, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::BaseEnter)))(serverImpl, baseId, client);
    }
}

void __fastcall ClientSend::BaseExit(IServerImpl* serverImpl, void*, uint baseId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeBaseExit, baseId, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::BaseExit)))(serverImpl, baseId, client);
    }
}

void __fastcall ClientSend::LocationEnter(IServerImpl* serverImpl, void*, uint locationId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeLocationEnter, locationId, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::LocationEnter)))(
            serverImpl, locationId, client);
    }
}

void __fastcall ClientSend::LocationExit(IServerImpl* serverImpl, void*, uint locationId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeLocationExit, locationId, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::LocationExit)))(serverImpl, locationId, client);
    }
}

void __fastcall ClientSend::DestroyCharacter(IServerImpl* serverImpl, void*, CHARACTER_ID const& cid, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, CHARACTER_ID const&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeDestroyCharacter, cid, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::DestroyCharacter)))(serverImpl, cid, client);
    }
}

void __fastcall ClientSend::CreateNewCharacter(IServerImpl* serverImpl, void*, SCreateCharacterInfo& info, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SCreateCharacterInfo&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeCreateNewCharacter, info, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::CreateNewCharacter)))(serverImpl, info, client);
    }
}

void __fastcall ClientSend::ObjectSelect(IServerImpl* serverImpl, void*, unsigned int spaceId, unsigned int client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeObjectSelect, spaceId, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::ObjectSelect)))(serverImpl, spaceId, client);
    }
}

void __fastcall ClientSend::GoodVaporized(IServerImpl* serverImpl, void*, SGFGoodVaporizedInfo& info, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SGFGoodVaporizedInfo&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeGoodVaporized, info, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::GoodVaporized)))(serverImpl, info, client);
    }
}

void __fastcall ClientSend::GoodBuy(IServerImpl* serverImpl, void*, SGFGoodBuyInfo& info, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SGFGoodBuyInfo&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeGoodBuy, info, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::GoodBuy)))(serverImpl, info, client);
    }
}

void __fastcall ClientSend::GoodSell(IServerImpl* serverImpl, void*, SGFGoodSellInfo& info, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SGFGoodSellInfo&, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeGoodSell, info, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::GoodSell)))(serverImpl, info, client);
    }
}

void __fastcall ClientSend::TradeResponse(IServerImpl* serverImpl, void*, unsigned char* unk1, int unk2, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, unsigned char*, int, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeTradeResponse, unk1, unk2, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::TradeResponse)))(
            serverImpl, unk1, unk2, client);
    }
}

void __fastcall ClientSend::PlayerLaunch(IServerImpl* serverImpl, void*, uint baseId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforePlayerLaunch, baseId, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::PlayerLaunch)))(serverImpl, baseId, client);
    }
}

void __fastcall ClientSend::LaunchComplete(IServerImpl* serverImpl, void*, uint baseId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeLaunchComplete, baseId, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::LaunchComplete)))(serverImpl, baseId, client);
    }
}

void __fastcall ClientSend::SystemSwitchOutComplete(IServerImpl* serverImpl, void*, uint shipId, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSystemSwitchOutComplete, shipId, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::SystemSwitchOutComplete)))(
            serverImpl, shipId, client);
    }
}

void __fastcall ClientSend::JumpInComplete(IServerImpl* serverImpl, void*, uint systemId, uint shipId)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeJumpInComplete, systemId, shipId))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::JumpInComplete)))(serverImpl, systemId, shipId);
    }
}

void __fastcall ClientSend::Hail(IServerImpl* serverImpl, void* edx, unsigned int unk1, unsigned int unk2, unsigned int unk3)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeHail, unk1, unk2, unk3))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::Hail)))(serverImpl, unk1, unk2, unk3);
    }
}

void __fastcall ClientSend::ObjectUpdate(IServerImpl* serverImpl, void* edx, SSPObjUpdateInfo& ui, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SSPObjUpdateInfo & ui, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeObjectUpdate, ui, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::ObjectUpdate)))(serverImpl, ui, client);
    }
}

void __fastcall ClientSend::MunitionCollision(IServerImpl* serverImpl, void* edx, SSPMunitionCollisionInfo& mci, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SSPMunitionCollisionInfo&, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeMunitionCollision, mci, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::MunitionCollision)))(serverImpl, mci, client);
    }
}

void __fastcall ClientSend::ObjectCollision(IServerImpl* serverImpl, void* edx, SSPObjCollisionInfo& info, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, SSPObjCollisionInfo&, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeObjectCollision, info, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::ObjectCollision)))(serverImpl, info, client);
    }
}

void __fastcall ClientSend::RequestEvent(IServerImpl* serverImpl, void* edx, int eventType, uint shipId, uint dockTarget, uint unk1, ulong unk2, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, int eventType, uint shipID, uint dockTarget, uint, ulong, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestEvent, eventType, shipId, dockTarget, unk1, unk2, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestEvent)))(
            serverImpl, eventType, shipId, dockTarget, unk1, unk2, client);
    }
}

void __fastcall ClientSend::RequestCancel(IServerImpl* serverImpl, void* edx, int eventType, uint shipId, uint unk1, ulong unk2, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, int, uint, uint, ulong, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestCancel, eventType, shipId, unk1, unk2, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestCancel)))(
            serverImpl, eventType, shipId, unk1, unk2, client);
    }
}

void __fastcall ClientSend::AbortMission(IServerImpl* serverImpl, void* edx, uint client, uint unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uint unk2);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeAbortMission, client, unk2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::AbortMission)))(serverImpl, client, unk2);
    }
}

void __fastcall ClientSend::SetWeaponGroup(IServerImpl* serverImpl, void* edx, uint client, uchar* unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uchar * unk1, uint unk2);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetWeaponGroup, client, unk1, unk2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::SetWeaponGroup)))(
            serverImpl, client, unk1, unk2);
    }
}

void __fastcall ClientSend::SetVisitedState(IServerImpl* serverImpl, void* edx, uint client, uchar* unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uchar * unk1, uint unk2);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetVisitedState, client, unk1, unk2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::SetVisitedState)))(
            serverImpl, client, unk1, unk2);
    }
}

void __fastcall ClientSend::RequestBestPath(IServerImpl* serverImpl, void* edx, uint client, uchar* unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uchar * unk1, uint unk2);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestBestPath, client, unk1, unk2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestBestPath)))(
            serverImpl, client, unk1, unk2);
    }
}

void __fastcall ClientSend::RequestPlayerStats(IServerImpl* serverImpl, void* edx, uint client, uchar* unk1, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uchar * unk1, uint unk2);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestPlayerStats, client, unk1, unk2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestPlayerStats)))(
            serverImpl, client, unk1, unk2);
    }
}

void __fastcall ClientSend::InitiateTrade(IServerImpl* serverImpl, void* edx, uint client1, uint client2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeInitiateTrade, client1, client2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::InitiateTrade)))(serverImpl, client1, client2);
    }
}

void __fastcall ClientSend::TerminateTrade(IServerImpl* serverImpl, void* edx, uint client, int accepted)
{
    using FuncType = void(__thiscall*)(IServerImpl*, uint client, int);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeTerminateTrade, client, accepted))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::TerminateTrade)))(serverImpl, client, accepted);
    }
}

void __fastcall ClientSend::AcceptTrade(IServerImpl* serverImpl, void* edx, uint client, bool unk)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uint unk);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeAcceptTrade, client, unk))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::AcceptTrade)))(serverImpl, client, unk);
    }
}

void __fastcall ClientSend::SetTradeMoney(IServerImpl* serverImpl, void* edx, uint client, ulong money)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client, uint unk);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSetTradeMoney, client, money))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::SetTradeMoney)))(serverImpl, client, money);
    }
}

void __fastcall ClientSend::AddTradeEquip(IServerImpl* serverImpl, void* edx, uint client, EquipDesc& ed)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, EquipDesc & ed);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeAddTradeEquip, client, ed))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::AddTradeEquip)))(serverImpl, client, ed);
    }
}

void __fastcall ClientSend::DelTradeEquip(IServerImpl* serverImpl, void* edx, uint client, EquipDesc& ed)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, EquipDesc&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRemoveTradeEquip, client, ed))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::DelTradeEquip)))(serverImpl, client, ed);
    }
}

void __fastcall ClientSend::RequestTrade(IServerImpl* serverImpl, void* edx, uint unk1, uint unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestTrade, unk1, unk2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestTrade)))(serverImpl, unk1, unk2);
    }
}

void __fastcall ClientSend::StopTradeRequest(IServerImpl* serverImpl, void* edx, uint client)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint client);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeStopTradeRequest, client))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::StopTradeRequest)))(serverImpl, client);
    }
}

void __fastcall ClientSend::RequestDifficultyScale(IServerImpl* serverImpl, void* edx, float scale, uint unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, float, uint);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeRequestDifficultyScale, scale, unk2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::RequestDifficultyScale)))(
            serverImpl, scale, unk2);
    }
}

void __fastcall ClientSend::Dock(IServerImpl* serverImpl, void* edx, uint& unk1, uint& unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint&, uint&);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeDock, unk1, unk2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::Dock)))(serverImpl, unk1, unk2);
    }
}
/*
void __fastcall ClientSend::SubmitChat(IServerImpl* serverImpl, void* edx, uint from, ulong size, void* rdlReader, uint to, int unk2)
{
    Fluf::Log(LogLevel::Trace, __FUNCTION__);

    using FuncType = void(__thiscall*)(IServerImpl*, uint, ulong, void*, uint, int);
    if (Fluf::instance->CallModuleEvent(&FlufModule::BeforeSubmitChat, from, size, rdlReader, to, unk2))
    {
        static_cast<FuncType>(Fluf::instance->localServerVTable->GetOriginal(static_cast<ushort>(IServerVTable::SubmitChat)))(
            serverImpl, from, size, rdlReader, to, unk2);
    }
}
*/
