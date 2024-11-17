#include "PCH.hpp"

#include "FLCore/FLCoreRemoteClient.h"
#include "Typedefs.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "Internal/FlufConfiguration.hpp"
#include "Internal/Hooks/ClientReceive.hpp"
#include "Internal/Hooks/ClientSend.hpp"

#include <iostream>

using ScriptLoadPtr = void* (*)(const char* fileName);
using FrameUpdatePtr = void (*)(double delta);
using ContextSwitchPtr = IClientImpl* (*)(const char* dllName);
std::unique_ptr<FunctionDetour<ScriptLoadPtr>> thornLoadDetour;
std::unique_ptr<FunctionDetour<FrameUpdatePtr>> frameUpdateDetour;
std::unique_ptr<FunctionDetour<ContextSwitchPtr>> contextSwitchDetour;

std::shared_ptr<Fluf> fluf;
HMODULE thisDll;

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

constexpr auto majorVersion = ModuleMajorVersion::One;
constexpr auto minorVersion = ModuleMinorVersion::Zero;

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    if (reason == DLL_PROCESS_ATTACH)
    {
        thisDll = mod;
        fluf = std::make_shared<Fluf>();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        fluf.reset();
    }

    return TRUE;
}

void Fluf::OnGameLoad() const
{
    Log(LogLevel::Info, "Data loaded, Freelancer ready.");
    for (const auto& module : loadedModules)
    {
        Log(LogLevel::Trace, std::format("OnGameLoad - {}", module->GetModuleName()));
        module->OnGameLoad();
    }
}

// ReSharper disable twice CppPassValueParameterByConstReference
bool Fluf::ModuleSorter(std::shared_ptr<FlufModule> a, std::shared_ptr<FlufModule> b) // NOLINT(*-unnecessary-value-param)
{
    // Sort all modules so they are in alphabetical order, but with the FLUF* core modules being loaded first
    if (a->GetModuleName().starts_with("FLUF") && !b->GetModuleName().starts_with("FLUF"))
    {
        return true;
    }

    if (b->GetModuleName().starts_with("FLUF") && !a->GetModuleName().starts_with("FLUF"))
    {
        return false;
    }

    return a->GetModuleName() < b->GetModuleName();
}

void Fluf::OnUpdateHook(const double delta)
{
    constexpr float SixtyFramesPerSecond = 1.0f / 60.0f;
    static double timeCounter = 0.0f;

    timeCounter += delta;
    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (timeCounter > SixtyFramesPerSecond)
    {
        for (auto& module : fluf->loadedModules)
        {
            module->OnFixedUpdate(SixtyFramesPerSecond);
        }

        timeCounter -= SixtyFramesPerSecond;
    }

    for (auto& module : fluf->loadedModules)
    {
        module->OnUpdate(delta);
    }

    frameUpdateDetour->UnDetour();
    frameUpdateDetour->GetOriginalFunc()(delta);
    frameUpdateDetour->Detour(OnUpdateHook);
}

void* Fluf::OnScriptLoadHook(const char* file)
{
    static bool loaded = false;
    if (!loaded)
    {
        loaded = true;
        fluf->OnGameLoad();
    }

    thornLoadDetour->UnDetour();
    void* ret = thornLoadDetour->GetOriginalFunc()(file);
    thornLoadDetour->Detour(OnScriptLoadHook);
    return ret;
}

struct VTableHack
{
        template <typename ClientVTable, typename ServerVTable>
        static void HookClientServer(ClientVTable* cvTable, ServerVTable* svTable)
        {
            const void* ptr;
#define HOOK(vtable, func, entry) \
    ptr = &func;                  \
    (vtable)->Hook(static_cast<DWORD>(entry), &ptr);

            HOOK(cvTable, ClientReceive::FireWeapon, IClientVTable::FireWeapon);
            HOOK(cvTable, ClientReceive::ActivateEquip, IClientVTable::ActivateEquip);
            HOOK(cvTable, ClientReceive::ActivateCruise, IClientVTable::ActivateCruise);
            HOOK(cvTable, ClientReceive::ActivateThruster, IClientVTable::ActivateThruster);
            HOOK(cvTable, ClientReceive::SetTarget, IClientVTable::SetTarget);
            HOOK(cvTable, ClientReceive::EnterTradeLane, IClientVTable::EnterTradeLane);
            HOOK(cvTable, ClientReceive::StopTradeLane, IClientVTable::LeaveTradeLane);
            HOOK(cvTable, ClientReceive::JettisonCargo, IClientVTable::JettisonCargo);
            HOOK(cvTable, ClientReceive::Login, IClientVTable::Login);
            HOOK(cvTable, ClientReceive::CharacterInformationReceived, IClientVTable::CharacterInfo);
            HOOK(cvTable, ClientReceive::CharacterSelect, IClientVTable::CharacterSelect);
            HOOK(cvTable, ClientReceive::AddItem, IClientVTable::AddItemToCharacter);
            HOOK(cvTable, ClientReceive::StartRoom, IClientVTable::SetStartRoom);
            HOOK(cvTable, ClientReceive::DestroyCharacter, IClientVTable::DestroyCharacter);
            HOOK(cvTable, ClientReceive::UpdateCharacter, IClientVTable::UpdateCharacter);
            HOOK(cvTable, ClientReceive::SetReputation, IClientVTable::SetReputation);
            HOOK(cvTable, ClientReceive::Land, IClientVTable::Land);
            HOOK(cvTable, ClientReceive::Launch, IClientVTable::Launch);
            HOOK(cvTable, ClientReceive::SystemSwitchOut, IClientVTable::SystemSwitchOut);
            HOOK(cvTable, ClientReceive::JumpInComplete, IClientVTable::JumpInComplete);
            HOOK(cvTable, ClientReceive::ShipCreate, IClientVTable::CreateShip);
            HOOK(cvTable, ClientReceive::DamageObject, IClientVTable::DamageObject);
            HOOK(cvTable, ClientReceive::ItemTractored, IClientVTable::ItemTractored);
            HOOK(cvTable, ClientReceive::CargoObjectUpdated, IClientVTable::ObjectCargoUpdate);
            HOOK(cvTable, ClientReceive::FuseBurnStarted, IClientVTable::BeginFuse);
            HOOK(cvTable, ClientReceive::WeaponGroupSet, IClientVTable::SetWeaponGroup);
            HOOK(cvTable, ClientReceive::VisitStateSet, IClientVTable::SetVisitState);
            HOOK(cvTable, ClientReceive::BestPathResponse, IClientVTable::RequestBestPath);
            HOOK(cvTable, ClientReceive::PlayerInformation, IClientVTable::RequestPlayerStats);
            HOOK(cvTable, ClientReceive::GroupPositionResponse, IClientVTable::RequestGroupPositions);
            HOOK(cvTable, ClientReceive::PlayerIsLeavingServer, IClientVTable::PlayerIsLeavingServer);
            HOOK(cvTable, ClientReceive::FormationUpdate, IClientVTable::FormationUpdate);

            HOOK(svTable, ClientSend::FireWeapon, IServerVTable::FireWeapon);
            HOOK(svTable, ClientSend::ActivateEquip, IServerVTable::ActivateEquip);
            HOOK(svTable, ClientSend::ActivateCruise, IServerVTable::ActivateCruise);
            HOOK(svTable, ClientSend::ActivateThruster, IServerVTable::ActivateThruster);
            HOOK(svTable, ClientSend::SetTarget, IServerVTable::SetTarget);
            HOOK(svTable, ClientSend::TractorObjects, IServerVTable::TractorObjects);
            HOOK(svTable, ClientSend::EnterTradeLane, IServerVTable::EnterTradeLane);
            HOOK(svTable, ClientSend::LeaveTradeLane, IServerVTable::LeaveTradeLane);
            HOOK(svTable, ClientSend::JettisonCargo, IServerVTable::JettisonCargo);
            HOOK(svTable, ClientSend::Disconnect, IServerVTable::Disconnect);
            HOOK(svTable, ClientSend::Connect, IServerVTable::Connect);
            HOOK(svTable, ClientSend::Login, IServerVTable::Login);
            HOOK(svTable, ClientSend::CharacterInfoRequest, IServerVTable::CharacterInfoRequest);
            HOOK(svTable, ClientSend::CharacterSelect, IServerVTable::CharacterSelect);
            HOOK(svTable, ClientSend::CreateNewCharacter, IServerVTable::CreateNewCharacter);
            HOOK(svTable, ClientSend::DestroyCharacter, IServerVTable::DestroyCharacter);
            HOOK(svTable, ClientSend::RequestAddItem, IServerVTable::RequestAddItem);
            HOOK(svTable, ClientSend::RequestRemoveItem, IServerVTable::RequestRemoveItem);
            HOOK(svTable, ClientSend::RequestModifyItem, IServerVTable::RequestModifyItem);
            HOOK(svTable, ClientSend::RequestSetCash, IServerVTable::RequestSetCash);
            HOOK(svTable, ClientSend::RequestChangeCash, IServerVTable::RequestChangeCash);
            HOOK(svTable, ClientSend::BaseEnter, IServerVTable::BaseEnter);
            HOOK(svTable, ClientSend::BaseExit, IServerVTable::BaseExit);
            HOOK(svTable, ClientSend::LocationEnter, IServerVTable::LocationEnter);
            HOOK(svTable, ClientSend::LocationExit, IServerVTable::LocationExit);
            HOOK(svTable, ClientSend::ObjectSelect, IServerVTable::ObjectSelect);
            HOOK(svTable, ClientSend::GoodVaporized, IServerVTable::GoodVaporized);
            HOOK(svTable, ClientSend::TradeResponse, IServerVTable::TradeResponse);
            HOOK(svTable, ClientSend::GoodBuy, IServerVTable::GoodBuy);
            HOOK(svTable, ClientSend::GoodSell, IServerVTable::GoodSell);
            HOOK(svTable, ClientSend::SystemSwitchOutComplete, IServerVTable::SystemSwitchOutComplete);
            HOOK(svTable, ClientSend::PlayerLaunch, IServerVTable::PlayerLaunch);
            HOOK(svTable, ClientSend::LaunchComplete, IServerVTable::LaunchComplete);
            HOOK(svTable, ClientSend::JumpInComplete, IServerVTable::JumpInComplete);
            HOOK(svTable, ClientSend::Hail, IServerVTable::Hail);
            HOOK(svTable, ClientSend::ObjectUpdate, IServerVTable::ObjectUpdate);
            HOOK(svTable, ClientSend::MunitionCollision, IServerVTable::MunitionCollision);
            HOOK(svTable, ClientSend::ObjectCollision, IServerVTable::ObjectCollision);
            HOOK(svTable, ClientSend::RequestEvent, IServerVTable::RequestEvent);
            HOOK(svTable, ClientSend::RequestCancel, IServerVTable::RequestCancel);
            HOOK(svTable, ClientSend::AbortMission, IServerVTable::AbortMission);
            HOOK(svTable, ClientSend::SetWeaponGroup, IServerVTable::SetWeaponGroup);
            HOOK(svTable, ClientSend::SetVisitedState, IServerVTable::SetVisitedState);
            HOOK(svTable, ClientSend::RequestBestPath, IServerVTable::RequestBestPath);
            HOOK(svTable, ClientSend::RequestPlayerStats, IServerVTable::RequestPlayerStats);
            HOOK(svTable, ClientSend::InitiateTrade, IServerVTable::InitiateTrade);
            HOOK(svTable, ClientSend::TerminateTrade, IServerVTable::TerminateTrade);
            HOOK(svTable, ClientSend::AcceptTrade, IServerVTable::AcceptTrade);
            HOOK(svTable, ClientSend::SetTradeMoney, IServerVTable::SetTradeMoney);
            HOOK(svTable, ClientSend::AddTradeEquip, IServerVTable::AddTradeEquip);
            HOOK(svTable, ClientSend::DelTradeEquip, IServerVTable::DelTradeEquip);
            HOOK(svTable, ClientSend::RequestTrade, IServerVTable::RequestTrade);
            HOOK(svTable, ClientSend::StopTradeRequest, IServerVTable::StopTradeRequest);
            HOOK(svTable, ClientSend::RequestDifficultyScale, IServerVTable::RequestDifficultyScale);
            HOOK(svTable, ClientSend::Dock, IServerVTable::Dock);
            // HOOK(svTable, ClientSend::SubmitChat, IServerVTable::SubmitChat);

#undef HOOK
        }
};

IClientImpl* Fluf::OnContextSwitchDetour(const char* dllName)
{
    contextSwitchDetour->UnDetour();
    fluf->serverClient = contextSwitchDetour->GetOriginalFunc()(dllName);
    contextSwitchDetour->Detour(OnContextSwitchDetour);

    const bool local = SinglePlayer();
    Log(LogLevel::Debug, std::format("Context switching to {} ({})", local ? "SP" : "MP", dllName));

    // Swap to rpclocal.dll or RemoteServer.dll, depending on the context
    fluf->clientServer = reinterpret_cast<IServerImpl*>(GetProcAddress(GetModuleHandleA(dllName), "??_7IClient@@6B@"));

    if (local)
    {
        fluf->localClientVTable =
            std::make_unique<VTableHook<static_cast<DWORD>(IClientVTable::LocalStart), static_cast<DWORD>(IClientVTable::LocalEnd)>>(dllName);
        fluf->localServerVTable =
            std::make_unique<VTableHook<static_cast<DWORD>(IServerVTable::LocalStart), static_cast<DWORD>(IServerVTable::LocalEnd)>>(dllName);
        VTableHack::HookClientServer(fluf->localClientVTable.get(), fluf->localServerVTable.get());
    }
    else
    {
        fluf->remoteClientVTable =
            std::make_unique<VTableHook<static_cast<DWORD>(IClientVTable::RemoteStart), static_cast<DWORD>(IClientVTable::RemoteEnd)>>(dllName);
        fluf->remoteServerVTable =
            std::make_unique<VTableHook<static_cast<DWORD>(IServerVTable::RemoteStart), static_cast<DWORD>(IServerVTable::RemoteEnd)>>(dllName);
        VTableHack::HookClientServer(fluf->remoteClientVTable.get(), fluf->remoteServerVTable.get());
    }

    return fluf->serverClient;
}

std::string SetLogMetadata(void* address, LogLevel level)
{
    if (HMODULE dll; RtlPcToFileHeader(address, reinterpret_cast<void**>(&dll)))
    {
        std::array<char, MAX_PATH> path;
        if (GetModuleFileNameA(dll, path.data(), path.size()))
        {
            const std::string fullPath = path.data();
            std::string levelStr;
            switch (level)
            {
                case LogLevel::Trace: levelStr = "TRACE"; break;
                case LogLevel::Debug: levelStr = "DBG"; break;
                case LogLevel::Info: levelStr = "INFO"; break;
                case LogLevel::Warn: levelStr = "WARN"; break;
                case LogLevel::Error: levelStr = "ERR"; break;
            }

            return std::format("({} {}) {}: ", "TIME", fullPath.substr(fullPath.find_last_of("\\") + 1), levelStr);
        }
    }

    return "";
}

void Fluf::Log(LogLevel level, std::string_view message)
{
    if (level < instance->config->logLevel)
    {
        return;
    }

    const std::string paddedMessage = SetLogMetadata(_ReturnAddress(), level) + std::string(message);

    if (instance->config->logSinks.contains(LogSink::Console))
    {
        enum class ConsoleColour
        {
            Blue = 1,
            Green = 2,
            Red = 4,
            Bold = 8,
            Cyan = Blue | Green,
            Purple = Red | Blue,
            Yellow = Red | Green,
            White = Red | Green | Blue,
            StrongWhite = White | Bold,
            StrongCyan = Cyan | Bold,
            StrongRed = Red | Bold,
            StrongYellow = Yellow | Bold,
            StrongGreen = Green | Bold,
        };

        const auto outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        switch (level)
        {
            case LogLevel::Trace: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongCyan)); break;
            case LogLevel::Debug: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongGreen)); break;
            case LogLevel::Info: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongWhite)); break;
            case LogLevel::Warn: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongYellow)); break;
            case LogLevel::Error: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongRed)); break;
        }

        std::cout << paddedMessage << std::endl;
    }
}

std::weak_ptr<FlufModule> Fluf::GetModule(const std::string_view identifier)
{
    for (auto& module : instance->loadedModules)
    {
        if (module->GetModuleName() == identifier)
        {
            return module;
        }
    }

    return {};
}

__declspec(naked) CShip* Fluf::GetCShip()
{
    __asm
    {
       mov	eax, 0x54baf0
       call	eax
       test	eax, eax
       jz	noship
       add	eax, 12
       mov	eax, [eax + 4]
       noship:
       ret
    }
}

Fluf::Fluf()
{
    instance = this;
    config = std::make_shared<FlufConfiguration>();
    config->Load();

    // Console sink enabled, allocate console and allow us to use std::cout
    if (config->logSinks.contains(LogSink::Console))
    {
        AllocConsole();
        SetConsoleTitleA("FLUF - Freelancer Unified Framework");

        const auto console = GetConsoleWindow();
        RECT r;
        GetWindowRect(console, &r);

        MoveWindow(console, r.left, r.top, 1366, 768, TRUE);

        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        SetStdHandle(STD_OUTPUT_HANDLE, stdout);
    }

    Log(LogLevel::Trace, "Hello world");
    Log(LogLevel::Debug, "Hello world");
    Log(LogLevel::Info, "Hello world");
    Log(LogLevel::Warn, "Hello world");
    Log(LogLevel::Error, "Hello world");

    // Load all dlls as needed
    for (const auto& modulePath : config->modules)
    {
        // Ensure it is not already loaded
        auto lib = GetModuleHandleA(modulePath.c_str());
        if (lib)
        {
            Log(LogLevel::Warn, std::format("Module already loaded: {}", modulePath));
            continue;
        }

        lib = LoadLibraryA(modulePath.c_str());
        if (!lib)
        {
            Log(LogLevel::Error, std::format("Failed to load module: {}", modulePath));
            continue;
        }

        const auto factory = reinterpret_cast<std::shared_ptr<FlufModule> (*)()>(GetProcAddress(lib, "ModuleFactory"));
        if (!factory)
        {
            Log(LogLevel::Error, std::format("Loaded module did not have ModuleFactory export: {}", modulePath));
            continue;
        }

        const auto module = factory();
        if (module->majorVersion != majorVersion)
        {
            Log(LogLevel::Error, std::format("Major version did not match for module: {}", modulePath));
            continue;
        }

        if (module->minorVersion != minorVersion)
        {
            Log(LogLevel::Warn, std::format("Minor version did not match for module: {}", modulePath));
        }

        loadedModules.emplace_hint(loadedModules.end(), module);
        Log(LogLevel::Info, std::format("Loaded Module: {}", module->GetModuleName()));
    }

    const HMODULE common = GetModuleHandleA("common");
    thornLoadDetour = std::make_unique<FunctionDetour<ScriptLoadPtr>>(
        reinterpret_cast<ScriptLoadPtr>(GetProcAddress(common, "?ThornScriptLoad@@YAPAUIScriptEngine@@PBD@Z"))); // NOLINT

    thornLoadDetour->Detour(OnScriptLoadHook);

    const auto fl = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    frameUpdateDetour = std::make_unique<FunctionDetour<FrameUpdatePtr>>(reinterpret_cast<FrameUpdatePtr>(fl + 0x1B2890));
    frameUpdateDetour->Detour(OnUpdateHook);

    contextSwitchDetour = std::make_unique<FunctionDetour<ContextSwitchPtr>>(reinterpret_cast<ContextSwitchPtr>(fl + 0x1B6F40));
    contextSwitchDetour->Detour(OnContextSwitchDetour);
}

Fluf::~Fluf() = default;
