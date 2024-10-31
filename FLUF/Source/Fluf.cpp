#include "PCH.hpp"

#include "FLCore/FLCoreRemoteClient.h"
#include "Typedefs.hpp"
#include "Utils/Detour.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "Internal/FlufConfiguration.hpp"
#include "Internal/Hooks/ClientReceive.hpp"

using ScriptLoadPtr = void* (*)(const char* fileName);
using FrameUpdatePtr = void (*)(double delta);
using ContextSwitchPtr = IClientImpl* (*)(const char* dllName);
std::unique_ptr<FunctionDetour<ScriptLoadPtr>> thornLoadDetour;
std::unique_ptr<FunctionDetour<FrameUpdatePtr>> frameUpdateDetour;
std::unique_ptr<FunctionDetour<ContextSwitchPtr>> contextSwitchDetour;

std::shared_ptr<Fluf> fluf;

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
    for (const auto& module : loadedModules)
    {
        module->OnGameLoad();
    }
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

IClientImpl* Fluf::OnContextSwitchDetour(const char* dllName)
{
    contextSwitchDetour->UnDetour();
    fluf->serverClient = contextSwitchDetour->GetOriginalFunc()(dllName);
    contextSwitchDetour->Detour(OnContextSwitchDetour);

    // Swap to Server.dll or RemoteServer.dll, depending on the context
    fluf->clientServer =
        reinterpret_cast<IServerImpl*>(GetProcAddress(GetModuleHandleA(_strcmpi(dllName, "Server.dll") == 0 ? "Server.dll" : dllName), "??_7IClient@@6B@"));

    fluf->clientVTable = std::make_unique<VTableHook<static_cast<DWORD>(IClientVTable::Start), static_cast<DWORD>(IClientVTable::End)>>(dllName);
    fluf->serverVTable = std::make_unique<VTableHook<static_cast<DWORD>(IServerVTable::Start), static_cast<DWORD>(IServerVTable::End)>>(dllName);
    fluf->HookServer();

    return fluf->serverClient;
}

void Fluf::HookServer() const
{
    const void* ptr;
#define HOOK(vtable, func, entry) \
    ptr = &func;                  \
    vtable->Hook(static_cast<DWORD>(entry), &ptr);

    HOOK(clientVTable, ClientReceive::FireWeapon, IClientVTable::FireWeapon);
    HOOK(clientVTable, ClientReceive::ActivateEquip, IClientVTable::ActivateEquip);
    HOOK(clientVTable, ClientReceive::ActivateCruise, IClientVTable::ActivateCruise);
    HOOK(clientVTable, ClientReceive::ActivateThruster, IClientVTable::ActivateThruster);
    HOOK(clientVTable, ClientReceive::SetTarget, IClientVTable::SetTarget);
    HOOK(clientVTable, ClientReceive::EnterTradeLane, IClientVTable::EnterTradeLane);
    HOOK(clientVTable, ClientReceive::StopTradeLane, IClientVTable::LeaveTradeLane);
    HOOK(clientVTable, ClientReceive::JettisonCargo, IClientVTable::JettisonCargo);
    HOOK(clientVTable, ClientReceive::Login, IClientVTable::Login);
    HOOK(clientVTable, ClientReceive::CharacterInformationReceived, IClientVTable::CharacterInfo);
    HOOK(clientVTable, ClientReceive::CharacterSelect, IClientVTable::CharacterSelect);
    HOOK(clientVTable, ClientReceive::AddItem, IClientVTable::AddItemToCharacter);
    HOOK(clientVTable, ClientReceive::StartRoom, IClientVTable::SetStartRoom);
    HOOK(clientVTable, ClientReceive::DestroyCharacter, IClientVTable::DestroyCharacter);
    HOOK(clientVTable, ClientReceive::UpdateCharacter, IClientVTable::UpdateCharacter);
    HOOK(clientVTable, ClientReceive::SetReputation, IClientVTable::SetReputation);
    HOOK(clientVTable, ClientReceive::Land, IClientVTable::Land);
    HOOK(clientVTable, ClientReceive::Launch, IClientVTable::Launch);
    HOOK(clientVTable, ClientReceive::SystemSwitchOut, IClientVTable::SystemSwitchOut);
    HOOK(clientVTable, ClientReceive::JumpInComplete, IClientVTable::JumpInComplete);
    HOOK(clientVTable, ClientReceive::ShipCreate, IClientVTable::CreateShip);
    HOOK(clientVTable, ClientReceive::DamageObject, IClientVTable::DamageObject);
    HOOK(clientVTable, ClientReceive::ItemTractored, IClientVTable::ItemTractored);
    HOOK(clientVTable, ClientReceive::CargoObjectUpdated, IClientVTable::ObjectCargoUpdate);
    HOOK(clientVTable, ClientReceive::FuseBurnStarted, IClientVTable::BeginFuse);
    HOOK(clientVTable, ClientReceive::WeaponGroupSet, IClientVTable::SetWeaponGroup);
    HOOK(clientVTable, ClientReceive::VisitStateSet, IClientVTable::SetVisitState);
    HOOK(clientVTable, ClientReceive::BestPathResponse, IClientVTable::RequestBestPath);
    HOOK(clientVTable, ClientReceive::PlayerInformation, IClientVTable::RequestPlayerStats);
    HOOK(clientVTable, ClientReceive::GroupPositionResponse, IClientVTable::RequestGroupPositions);
    HOOK(clientVTable, ClientReceive::PlayerIsLeavingServer, IClientVTable::PlayerIsLeavingServer);
    HOOK(clientVTable, ClientReceive::FormationUpdate, IClientVTable::FormationUpdate);

#undef HOOK
}

Fluf::Fluf()
{
    instance = this;
    config = std::make_shared<FlufConfiguration>();
    config->Load();

    // Load all dlls as needed
    for (const auto& modulePath : config->modules)
    {
        // Ensure it is not already loaded
        auto lib = GetModuleHandleA(modulePath.c_str());
        if (lib)
        {
            // TODO: LOG
            return;
        }

        lib = LoadLibraryA(modulePath.c_str());
        if (!lib)
        {
            // TODO: LOG
            return;
        }

        const auto factory = reinterpret_cast<std::shared_ptr<FlufModule> (*)()>(GetProcAddress(lib, "ModuleFactory"));
        if (factory)
        {
            // TODO: LOG
            return;
        }

        const auto module = factory();
        if (module->majorVersion != majorVersion)
        {
            // TODO: Log
            return;
        }

        if (module->minorVersion != minorVersion)
        {
            // TODO: Log
        }

        loadedModules.emplace(module);
        // TODO: LOG
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
