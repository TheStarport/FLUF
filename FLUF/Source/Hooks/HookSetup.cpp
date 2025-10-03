#include <ClientServerCommunicator.hpp>
#include <PCH.hpp>

#include <Fluf.hpp>
#include <KeyManager.hpp>
#include <Internal/FlufConfiguration.hpp>
#include <Internal/Hooks/ClientReceive.hpp>
#include <Utils/MemUtils.hpp>
#include <spdlog/sinks/rotating_file_sink.h>

using OnPhysicsUpdate = void (*)(uint system, float delta);
FunctionDetour physicsUpdateDetour{ reinterpret_cast<OnPhysicsUpdate>(GetProcAddress(GetModuleHandleA("common.dll"), "?Update@PhySys@@YAXIM@Z")) };

void Fluf::HookIClient(char* client, const bool unhook, const bool local)
{
    if (!client)
    {
        return;
    }

    char* vtable = client;
    memcpy(&vtable, vtable, 4);
    for (auto& [newFunc, oldFunc, offset] : clientPatches)
    {
        if (!newFunc || oldFunc == newFunc || (local && (offset / 4) == static_cast<long>(IClientVTable::Login)))
        {
            continue;
        }

        char* address = vtable + offset;
        if (unhook && oldFunc)
        {
            MemUtils::WriteProcMem(reinterpret_cast<DWORD>(address), &oldFunc, 4);
            oldFunc = nullptr;
        }
        else
        {
            char* oldFuncAddr;
            MemUtils::ReadProcMem(reinterpret_cast<DWORD>(address), &oldFuncAddr, 4);

            // Protection against double hooking happens if someone leaves a server and then reconnects without going back to the main menu
            if (oldFuncAddr == newFunc)
            {
                continue;
            }

            oldFunc = oldFuncAddr;
            MemUtils::WriteProcMem(reinterpret_cast<DWORD>(address), &newFunc, 4);
        }
    }
}

void Fluf::HookIServer(char* server, bool unhook)
{
    if (!server)
    {
        return;
    }

    char* vtable = server;
    memcpy(&vtable, vtable, 4);
    for (auto& [newFunc, oldFunc, offset] : serverPatches)
    {
        if (!newFunc || oldFunc == newFunc)
        {
            continue;
        }

        char* address = vtable + offset;

        if (unhook && oldFunc)
        {
            MemUtils::WriteProcMem(reinterpret_cast<DWORD>(address), &oldFunc, 4);
            oldFunc = nullptr;
        }
        else
        {
            char* oldFuncAddr;
            MemUtils::ReadProcMem(reinterpret_cast<DWORD>(address), &oldFuncAddr, 4);
            if (oldFuncAddr == newFunc)
            {
                continue;
            }

            oldFunc = oldFuncAddr;
            MemUtils::WriteProcMem(reinterpret_cast<DWORD>(address), &newFunc, 4);
        }
    }
}

bool __fastcall Fluf::DelayedRPCLocalDetour(void* _this, void* edx, void* dunno1, void* dunno2)
{
    auto handle = GetModuleHandleA("rpclocal");
    using GetChatServer = IChatServer*(__stdcall*)();

    delayedRPCLocalDetour->UnDetour();
    bool retVal = delayedRPCLocalDetour->GetOriginalFunc()(_this, edx, dunno1, dunno2);

    instance->HookIClient(reinterpret_cast<char*>(GetProcAddress(handle, "Client")), false, true);
    instance->HookIServer(reinterpret_cast<char*>(GetProcAddress(handle, "Server")), false);
    instance->clientServerCommunicator->clientChatServer = reinterpret_cast<GetChatServer>(GetProcAddress(handle, "GetChatServerInterface"))();

    return retVal;
}

void Fluf::OnPhysicsUpdateDetour(uint system, float delta)
{
    instance->CallModuleEvent(&FlufModule::BeforePhysicsUpdate, system, delta);

    physicsUpdateDetour.UnDetour();
    physicsUpdateDetour.GetOriginalFunc()(system, delta);
    physicsUpdateDetour.Detour(OnPhysicsUpdateDetour);

    instance->CallModuleEvent(&FlufModule::OnPhysicsUpdate, system, delta);
}

void Fluf::SetupHooks()
{
    //physicsUpdateDetour.Detour(OnPhysicsUpdateDetour);

    if (!runningOnClient)
    {
        return;
    }

    SetupLogging();

#define CLIENT_PATCH(x)      { reinterpret_cast<char*>(&ClientReceive::x), nullptr, static_cast<long>(IClientVTable::x) * 4 }
#define NULL_CLIENT_PATCH(x) { nullptr, nullptr, static_cast<long>(IClientVTable::x) * 4 }
#define SERVER_PATCH(x)      { reinterpret_cast<char*>(&ClientSend::x), nullptr, static_cast<long>(IServerVTable::x) * 4 }
#define NULL_SERVER_PATCH(x) { nullptr, nullptr, static_cast<long>(IServerVTable::x) * 4 }
    clientPatches = {
        {
         CLIENT_PATCH(FireWeapon),
         CLIENT_PATCH(ActivateEquip),
         CLIENT_PATCH(ActivateCruise),
         CLIENT_PATCH(ActivateThruster),
         CLIENT_PATCH(SetTarget),
         NULL_CLIENT_PATCH(unknown_6),
         CLIENT_PATCH(EnterTradeLane),
         NULL_CLIENT_PATCH(LeaveTradeLane),
         CLIENT_PATCH(JettisonCargo),
         NULL_CLIENT_PATCH(SendPacket),
         NULL_CLIENT_PATCH(Startup),
         NULL_CLIENT_PATCH(Nullsub),
         CLIENT_PATCH(Login),
         NULL_CLIENT_PATCH(CharacterInfo),
         CLIENT_PATCH(CharacterSelect),
         NULL_CLIENT_PATCH(Shutdown),
         NULL_CLIENT_PATCH(DispatchMsgs),
         NULL_CLIENT_PATCH(CdpProxyDisconnect),
         NULL_CLIENT_PATCH(CdpProxyGetSendQSize),
         NULL_CLIENT_PATCH(CdpProxyGetSendQBytes),
         NULL_CLIENT_PATCH(CdpProxyGetLinkSaturation),
         NULL_CLIENT_PATCH(SetShipArch),
         NULL_CLIENT_PATCH(SetHullStatus),
         NULL_CLIENT_PATCH(SetCollisionGroups),
         NULL_CLIENT_PATCH(SetEquipment),
         NULL_CLIENT_PATCH(unknown_26),
         NULL_CLIENT_PATCH(AddItemToCharacter),
         NULL_CLIENT_PATCH(unknown_28),
         NULL_CLIENT_PATCH(SetStartRoom),
         CLIENT_PATCH(DestroyCharacter),
         CLIENT_PATCH(UpdateCharacter),
         NULL_CLIENT_PATCH(CompleteCharacterList),
         NULL_CLIENT_PATCH(ScriptBehaviour),
         NULL_CLIENT_PATCH(DestroyScriptBehaviour),
         NULL_CLIENT_PATCH(CompleteScriptBehaviourList),
         NULL_CLIENT_PATCH(unknown_36),
         NULL_CLIENT_PATCH(unknown_37),
         NULL_CLIENT_PATCH(CompleteAmbientScriptList),
         NULL_CLIENT_PATCH(DestroyMissionComputer),
         NULL_CLIENT_PATCH(UpdateMissionComputer),
         NULL_CLIENT_PATCH(CompleteMissionComputerList),
         NULL_CLIENT_PATCH(MissionVendorAcceptance),
         NULL_CLIENT_PATCH(MissionVendorEmptyReason),
         NULL_CLIENT_PATCH(unknown_44),
         NULL_CLIENT_PATCH(UpdateNewsBroadcast),
         NULL_CLIENT_PATCH(CompleteNewsBroadcastList),
         NULL_CLIENT_PATCH(CreateSolar),
         NULL_CLIENT_PATCH(CreateShip),
         NULL_CLIENT_PATCH(CreateLoot),
         NULL_CLIENT_PATCH(CreateMine),
         NULL_CLIENT_PATCH(CreateGuided),
         NULL_CLIENT_PATCH(CreateCounter),
         NULL_CLIENT_PATCH(unknown_53),
         NULL_CLIENT_PATCH(unknown_54),
         NULL_CLIENT_PATCH(UpdateObject),
         NULL_CLIENT_PATCH(DestroyObject),
         NULL_CLIENT_PATCH(ActivateObject),
         CLIENT_PATCH(SystemSwitchOut),
         CLIENT_PATCH(JumpInComplete),
         CLIENT_PATCH(Land),
         CLIENT_PATCH(Launch),
         NULL_CLIENT_PATCH(RequestShip),
         NULL_CLIENT_PATCH(unknown_63),
         CLIENT_PATCH(DamageObject),
         CLIENT_PATCH(ItemTractored),
         NULL_CLIENT_PATCH(UseItem),
         CLIENT_PATCH(SetReputation),
         NULL_CLIENT_PATCH(unknown_68),
         NULL_CLIENT_PATCH(SendComm),
         NULL_CLIENT_PATCH(unknown_70),
         NULL_CLIENT_PATCH(SetMissionMessage),
         NULL_CLIENT_PATCH(unknown_72),
         NULL_CLIENT_PATCH(SetMissionObjectives),
         NULL_CLIENT_PATCH(unknown_74),
         NULL_CLIENT_PATCH(unknown_75),
         NULL_CLIENT_PATCH(MarkObject),
         NULL_CLIENT_PATCH(unknown_77),
         NULL_CLIENT_PATCH(SetCash),
         NULL_CLIENT_PATCH(unknown_79),
         NULL_CLIENT_PATCH(unknown_80),
         NULL_CLIENT_PATCH(unknown_81),
         NULL_CLIENT_PATCH(unknown_82),
         NULL_CLIENT_PATCH(unknown_83),
         NULL_CLIENT_PATCH(RequestReturned),
         NULL_CLIENT_PATCH(unknown_85),
         NULL_CLIENT_PATCH(unknown_86),
         NULL_CLIENT_PATCH(ObjectCargoUpdate),
         NULL_CLIENT_PATCH(BeginFuse),
         NULL_CLIENT_PATCH(unknown_89),
         NULL_CLIENT_PATCH(unknown_90),
         NULL_CLIENT_PATCH(unknown_91),
         NULL_CLIENT_PATCH(SetWeaponGroup),
         NULL_CLIENT_PATCH(SetVisitState),
         NULL_CLIENT_PATCH(RequestBestPath),
         NULL_CLIENT_PATCH(RequestPlayerStats),
         NULL_CLIENT_PATCH(unknown_96),
         NULL_CLIENT_PATCH(RequestGroupPositions),
         NULL_CLIENT_PATCH(SetMissionLog),
         NULL_CLIENT_PATCH(SetInterfaceState),
         NULL_CLIENT_PATCH(unknown_100),
         NULL_CLIENT_PATCH(unknown_101),
         NULL_CLIENT_PATCH(PlayerInitiateTrade),
         NULL_CLIENT_PATCH(PlayerTradeTarget),
         NULL_CLIENT_PATCH(PlayerAcceptTrade),
         NULL_CLIENT_PATCH(PlayerSetTradeMoney),
         NULL_CLIENT_PATCH(PlayerAddTradeEquip),
         NULL_CLIENT_PATCH(PlayerRemoveTradeEquip),
         NULL_CLIENT_PATCH(PlayerRequestTrade),
         NULL_CLIENT_PATCH(PlayerStopTradeRequest),
         NULL_CLIENT_PATCH(ScanNotify),
         NULL_CLIENT_PATCH(PlayerListUpdate),
         CLIENT_PATCH(PlayerIsLeavingServer),
         NULL_CLIENT_PATCH(PlayerListUpdate2),
         NULL_CLIENT_PATCH(MiscObjectUpdate6),
         NULL_CLIENT_PATCH(MiscObjectUpdate7),
         NULL_CLIENT_PATCH(MiscObjectUpdate1),
         NULL_CLIENT_PATCH(MiscObjectUpdate2),
         NULL_CLIENT_PATCH(MiscObjectUpdate3),
         NULL_CLIENT_PATCH(MiscObjectUpdate4),
         NULL_CLIENT_PATCH(MiscObjectUpdate5),
         NULL_CLIENT_PATCH(unknown_121),
         CLIENT_PATCH(FormationUpdate),
         NULL_CLIENT_PATCH(unknown_123),
         NULL_CLIENT_PATCH(unknown_124),
         NULL_CLIENT_PATCH(unknown_125),
         }
    };

    serverPatches = {
        {
         SERVER_PATCH(FireWeapon),
         SERVER_PATCH(ActivateEquip),
         SERVER_PATCH(ActivateCruise),
         SERVER_PATCH(ActivateThruster),
         SERVER_PATCH(SetTarget),
         SERVER_PATCH(TractorObjects),
         SERVER_PATCH(EnterTradeLane),
         SERVER_PATCH(LeaveTradeLane),
         SERVER_PATCH(JettisonCargo),
         SERVER_PATCH(Startup),
         NULL_SERVER_PATCH(Shutdown),
         SERVER_PATCH(Update),
         NULL_SERVER_PATCH(ElapseTime),
         NULL_SERVER_PATCH(Nullopt1),
         NULL_SERVER_PATCH(SwapConnections),
         NULL_SERVER_PATCH(Nullopt2),
         SERVER_PATCH(Disconnect),
         SERVER_PATCH(Connect),
         SERVER_PATCH(Login),
         SERVER_PATCH(CharacterInfoRequest),
         SERVER_PATCH(CharacterSelect),
         NULL_SERVER_PATCH(Nullopt3),
         SERVER_PATCH(CreateNewCharacter),
         SERVER_PATCH(DestroyCharacter),
         NULL_SERVER_PATCH(CharacterSkipAutosave),
         NULL_SERVER_PATCH(ReqShipArch),
         NULL_SERVER_PATCH(ReqHullStatus),
         NULL_SERVER_PATCH(ReqCollisionGroups),
         NULL_SERVER_PATCH(ReqEquipment),
         NULL_SERVER_PATCH(ReqCargo),
         SERVER_PATCH(RequestAddItem),
         SERVER_PATCH(RequestRemoveItem),
         SERVER_PATCH(RequestModifyItem),
         SERVER_PATCH(RequestSetCash),
         SERVER_PATCH(RequestChangeCash),
         SERVER_PATCH(BaseEnter),
         SERVER_PATCH(BaseExit),
         SERVER_PATCH(LocationEnter),
         SERVER_PATCH(LocationExit),
         NULL_SERVER_PATCH(BaseInfoRequest),
         NULL_SERVER_PATCH(LocationInfoRequest),
         SERVER_PATCH(ObjectSelect),
         SERVER_PATCH(GoodVaporized),
         NULL_SERVER_PATCH(MissionResponse),
         SERVER_PATCH(TradeResponse),
         SERVER_PATCH(GoodBuy),
         SERVER_PATCH(GoodSell),
         SERVER_PATCH(SystemSwitchOutComplete),
         SERVER_PATCH(PlayerLaunch),
         SERVER_PATCH(LaunchComplete),
         SERVER_PATCH(JumpInComplete),
         SERVER_PATCH(Hail),
         SERVER_PATCH(ObjectUpdate),
         SERVER_PATCH(MunitionCollision),
         NULL_SERVER_PATCH(SPBadLandsObjCollision),
         SERVER_PATCH(ObjectCollision),
         NULL_SERVER_PATCH(SPRequestUseItem),
         NULL_SERVER_PATCH(SPRequestInvincibility),
         NULL_SERVER_PATCH(SaveGame),
         NULL_SERVER_PATCH(MissionSaveB),
         SERVER_PATCH(RequestEvent),
         SERVER_PATCH(RequestCancel),
         NULL_SERVER_PATCH(MineAsteroid),
         NULL_SERVER_PATCH(CommComplete),
         NULL_SERVER_PATCH(RequestCreateShip),
         NULL_SERVER_PATCH(SPScanCargo),
         NULL_SERVER_PATCH(SetManeuver),
         NULL_SERVER_PATCH(InterfaceItemUsed),
         SERVER_PATCH(AbortMission),
         NULL_SERVER_PATCH(RTCDone),
         SERVER_PATCH(SetWeaponGroup),
         SERVER_PATCH(SetVisitedState),
         SERVER_PATCH(RequestBestPath),
         SERVER_PATCH(RequestPlayerStats),
         NULL_SERVER_PATCH(PopUpDialog),
         NULL_SERVER_PATCH(RequestGroupPositions),
         NULL_SERVER_PATCH(SetMissionLog),
         NULL_SERVER_PATCH(SetInterfaceState),
         NULL_SERVER_PATCH(RequestRankLevel),
         SERVER_PATCH(InitiateTrade),
         SERVER_PATCH(TerminateTrade),
         SERVER_PATCH(AcceptTrade),
         SERVER_PATCH(SetTradeMoney),
         SERVER_PATCH(AddTradeEquip),
         SERVER_PATCH(DelTradeEquip),
         SERVER_PATCH(RequestTrade),
         SERVER_PATCH(StopTradeRequest),
         SERVER_PATCH(RequestDifficultyScale),
         NULL_SERVER_PATCH(GetServerID),
         NULL_SERVER_PATCH(GetServerSig),
         NULL_SERVER_PATCH(DumpPacketStats),
         SERVER_PATCH(Dock),
         // NULL_SERVER_PATCH(PushToServer),
        }
    };

#undef CLIENT_PATCH
#undef NULL_CLIENT_PATCH
#undef SERVER_PATCH
#undef NULL_SERVER_PATCH

    keyManager = std::make_unique<KeyManager>();
    ClientPatches();

    loadResourceDllDetour = std::make_unique<FunctionDetour<LoadResourceDll>>(reinterpret_cast<LoadResourceDll>(0x5B0C30));
    loadResourceDllDetour->Detour(ResourceDllLoadDetour);
}
