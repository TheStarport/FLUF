#include "PCH.hpp"

#include "FLCore/FLCoreRemoteClient.h"
#include "Typedefs.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "Internal/FlufConfiguration.hpp"
#include "Internal/Hooks/ClientReceive.hpp"
#include "Internal/Hooks/ClientSend.hpp"
#include "KeyManager.hpp"
#include "Utils/MemUtils.hpp"

#include <iostream>

using ScriptLoadPtr = void* (*)(const char* fileName);
using FrameUpdatePtr = void (*)(double delta);
using ContextSwitchPtr = IClientImpl* (*)(const char* dllName);
std::unique_ptr<FunctionDetour<ScriptLoadPtr>> thornLoadDetour;
std::unique_ptr<FunctionDetour<FrameUpdatePtr>> frameUpdateDetour;

std::shared_ptr<Fluf> fluf;
HMODULE thisDll;

extern "C" __declspec(dllexport) void DummyFunction() {}

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

constexpr auto majorVersion = ModuleMajorVersion::One;
constexpr auto minorVersion = ModuleMinorVersion::Zero;

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

FunctionDetour loadLibraryDetour(LoadLibraryA);

HINSTANCE __stdcall Fluf::LoadLibraryDetour(const char* dllName)
{
    loadLibraryDetour.UnDetour();
    const auto res = LoadLibraryA(dllName);
    loadLibraryDetour.Detour(LoadLibraryDetour);

    if (!res)
    {
        return res;
    }

    if (instance->runningOnClient)
    {
#ifndef FLUF_DISABLE_HOOKS
        if (_stricmp(dllName, "rpclocal.dll") == 0)
        {
            instance->localClientVTable =
                std::make_unique<VTableHook<static_cast<DWORD>(IClientVTable::LocalStart), static_cast<DWORD>(IClientVTable::LocalEnd)>>(dllName);
            instance->localServerVTable =
                std::make_unique<VTableHook<static_cast<DWORD>(IServerVTable::LocalStart), static_cast<DWORD>(IServerVTable::LocalEnd)>>(dllName);
            VTableHack::HookClientServer(instance->localClientVTable.get(), instance->localServerVTable.get());
        }
        else if (_stricmp(dllName, "remoteserver.dll") == 0)
        {
            instance->remoteClientVTable =
                std::make_unique<VTableHook<static_cast<DWORD>(IClientVTable::RemoteStart), static_cast<DWORD>(IClientVTable::RemoteEnd)>>(dllName);
            instance->remoteServerVTable =
                std::make_unique<VTableHook<static_cast<DWORD>(IServerVTable::RemoteStart), static_cast<DWORD>(IServerVTable::RemoteEnd)>>(dllName);
            VTableHack::HookClientServer(instance->remoteClientVTable.get(), instance->remoteServerVTable.get());
        }
#endif
    }

    for (const auto& module : instance->loadedModules)
    {
        module->OnDllLoaded(dllName, res);
    }

    return res;
}

FunctionDetour freeLibraryDetour(FreeLibrary);

BOOL __stdcall Fluf::FreeLibraryDetour(HMODULE unloadedDll)
{
    std::array<char, MAX_PATH> dllNameBuf{};
    const uint len = GetModuleFileNameA(unloadedDll, dllNameBuf.data(), MAX_PATH);
    std::string_view dllName{ dllNameBuf.data(), len };
    dllName = dllName.substr(dllName.find_last_of('\\') + 1);

    if (instance->runningOnClient)
    {
        if (dllName == "rpclocal.dll")
        {
            instance->localClientVTable.reset();
            instance->localServerVTable.reset();
        }
        else if (dllName == "remoteserver.dll")
        {
            instance->remoteClientVTable.reset();
            instance->remoteServerVTable.reset();
        }
    }

    for (const auto& module : instance->loadedModules)
    {
        module->OnDllUnloaded(dllName, unloadedDll);
    }

    freeLibraryDetour.UnDetour();
    const auto res = FreeLibrary(unloadedDll);
    freeLibraryDetour.Detour(FreeLibraryDetour);

    return res;
}

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (constexpr const char* path = "modules/config"; !std::filesystem::exists(path) && !std::filesystem::create_directories(path))
        {
            MessageBoxA(nullptr, "Unable to create directories 'modules/config' in working directory.", "Permissions Error?", MB_OK);
            return false;
        }

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

bool __thiscall Fluf::OnServerStart(IServerImpl* server, SStartupInfo& info)
{
    for (const auto& module : fluf->loadedModules)
    {
        module->OnServerStart();
    }

    using StartType = bool(__thiscall*)(IServerImpl * server, SStartupInfo & info);
    const auto startup = reinterpret_cast<StartType>(oldServerStartupFunc);
    return startup(server, info);
}

void Fluf::OnUpdateHook(const double delta)
{
    constexpr float SixtyFramesPerSecond = 1.0f / 60.0f;
    static double timeCounter = 0.0f;

    timeCounter += delta;
    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (timeCounter > SixtyFramesPerSecond)
    {
        for (auto& mod : fluf->loadedModules)
        {
            mod->OnFixedUpdate(SixtyFramesPerSecond);
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

std::string SetLogMetadata(void* address, LogLevel level)
{
    if (HMODULE dll; RtlPcToFileHeader(address, reinterpret_cast<void**>(&dll)))
    {
        std::array<char, MAX_PATH> path{};
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

CShip* Fluf::GetPlayerCShip()
{
    const auto* obj = GetPlayerIObjRW();
    if (!obj)
    {
        return nullptr;
    }

    return dynamic_cast<CShip*>(obj->cobject());
}

IObjRW* Fluf::GetPlayerIObjRW()
{
    using objFunc = IObjRW* (*)();
    static auto getIObjRW = reinterpret_cast<objFunc>(0x54BAF0);
    return getIObjRW();
}

bool Fluf::IsRunningOnClient() { return instance->runningOnClient; }

KeyManager* Fluf::GetKeyManager() { return instance->keyManager.get(); }

// Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    // Get the error message ID, if any.
    const DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
    {
        return {}; // No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    // Ask Win32 to give us the string version of that message ID.
    // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 nullptr,
                                 errorMessageID,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 reinterpret_cast<LPSTR>(&messageBuffer),
                                 0,
                                 nullptr);

    // Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    // Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

Fluf::Fluf()
{
    instance = this;
    config = std::make_shared<FlufConfiguration>();
    config->Load();

    if (config->setSaveDirectoryRelativeToExecutable)
    {
        getUserDataPathDetour.Detour(GetUserDataPathDetour);
    }

    std::array<char, MAX_PATH> fileNameBuffer{};
    GetModuleFileNameA(nullptr, fileNameBuffer.data(), MAX_PATH);
    std::string_view fileName = fileNameBuffer.data();
    fileName = fileName.substr(fileName.find_last_of('\\') + 1);
    runningOnClient = _strcmpi(fileName.data(), "freelancer.exe") == 0;

    if (runningOnClient)
    {
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

        keyManager = std::make_unique<KeyManager>();
    }

    // Load all dlls as needed
    std::vector<std::string> preloadModules;
    std::copy(config->modules.begin(), config->modules.end(), std::back_inserter(preloadModules));
    std::ranges::sort(preloadModules, [](const std::string& a, const std::string& b) { return a.starts_with("FLUF") && !b.starts_with("FLUF"); });
    for (const auto& modulePath : preloadModules)
    {
        // Ensure it is not already loaded
        auto lib = GetModuleHandleA(modulePath.c_str());
        if (lib)
        {
            Log(LogLevel::Warn, std::format("Module already loaded: {}", modulePath));
            continue;
        }

        std::string modulePathWithDir = std::format("modules/{}", modulePath);
        lib = LoadLibraryA(modulePathWithDir.c_str());
        if (!lib)
        {
            Log(LogLevel::Error, std::format("Failed to load module: {}\nReason: {}", modulePath, GetLastErrorAsString()));
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

        loadedModules.emplace_back(module);
        Log(LogLevel::Info, std::format("Loaded Module: {}", module->GetModuleName()));
    }

    if (runningOnClient)
    {
        const HMODULE common = GetModuleHandleA("common");
        thornLoadDetour = std::make_unique<FunctionDetour<ScriptLoadPtr>>(
            reinterpret_cast<ScriptLoadPtr>(GetProcAddress(common, "?ThornScriptLoad@@YAPAUIScriptEngine@@PBD@Z"))); // NOLINT

        thornLoadDetour->Detour(OnScriptLoadHook);
    }
    else
    {
        const auto newServerStartup = reinterpret_cast<FARPROC>(OnServerStart);
        const auto serverStartupAddr = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr)) + 0x1BABC;
        MemUtils::ReadProcMem(serverStartupAddr, &oldServerStartupFunc, 4);
        MemUtils::WriteProcMem(serverStartupAddr, &newServerStartup, 4);
    }

    const auto fl = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    frameUpdateDetour = std::make_unique<FunctionDetour<FrameUpdatePtr>>(reinterpret_cast<FrameUpdatePtr>(fl + 0x1B2890));
    frameUpdateDetour->Detour(OnUpdateHook);
    loadLibraryDetour.Detour(LoadLibraryDetour);
    freeLibraryDetour.Detour(FreeLibraryDetour);
}

Fluf::~Fluf()
{
    while (!loadedModules.empty())
    {
        loadedModules.pop_back();
    }

    if (config->setSaveDirectoryRelativeToExecutable)
    {
        getUserDataPathDetour.UnDetour();
    }

    loadLibraryDetour.UnDetour();
    freeLibraryDetour.UnDetour();
}

bool Fluf::GetUserDataPathDetour(char* path)
{
    constexpr std::string_view newSavePath = R"(..\SAVES)";
    strcpy_s(path, newSavePath.size(), newSavePath.data());
    return true;
}
