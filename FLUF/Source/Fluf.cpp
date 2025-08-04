#include "PCH.hpp"

#include "FLCore/FLCoreRemoteClient.h"
#include "Typedefs.hpp"

#include "Fluf.hpp"

#include "ClientServerCommunicator.hpp"
#include "FlufModule.hpp"
#include "Internal/FlufConfiguration.hpp"
#include "Internal/Hooks/ClientReceive.hpp"
#include "Internal/Hooks/ClientSend.hpp"
#include "Internal/Hooks/InfocardOverrides.hpp"
#include "KeyManager.hpp"
#include "Utils/MemUtils.hpp"

#include <Exceptions.hpp>
#include <iostream>
#include <spdlog/common.h>
#include <spdlog/sinks/rotating_file_sink.h>

using ScriptLoadPtr = void* (*)(const char* fileName);
using FrameUpdatePtr = void (*)(double delta);
std::unique_ptr<FunctionDetour<ScriptLoadPtr>> thornLoadDetour;
std::unique_ptr<FunctionDetour<FrameUpdatePtr>> frameUpdateDetour;

std::shared_ptr<Fluf> fluf;
std::shared_ptr<spdlog::logger> logFile;

extern "C" __declspec(dllexport) void DummyFunction() {}

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

constexpr auto majorVersion = ModuleMajorVersion::One;
constexpr auto minorVersion = ModuleMinorVersion::Zero;

using FDump = DWORD (*)(DWORD, const char*, ...);
FDump fdumpOriginal;
DWORD FDumpDetour(DWORD unk, const char* fmt, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 4096, fmt, args);
    va_end(args);

    std::string line(buffer);
    LogLevel level = LogLevel::Info;

    auto contains = [](std::string_view a, std::string_view b)
    {
        return std::search(
                   a.begin(), a.end(), b.begin(), b.end(), [](unsigned char ch1, unsigned char ch2) { return std::toupper(ch1) == std::toupper(ch2); }) !=
               a.end();
    };

    if (contains(line, "WARN") || contains(line, "UNKNOWN"))
    {
        level = LogLevel::Warn;
    }
    else if (contains(line, "ERR"))
    {
        level = LogLevel::Error;
    }
    else if (contains(line, "DBG") || contains(line, "DEBUG"))
    {
        level = LogLevel::Debug;
    }

    Fluf::Log(level, line);
    std::time_t rawTime;
    char timestamp[100];
    std::time(&rawTime);
    std::tm* timeInfo = std::localtime(&rawTime);
    std::strftime(timestamp, 80, "%Y/%m/%d - %H:%M:%S", timeInfo);

    return fdumpOriginal(unk, "[%s] %s", timestamp, line.c_str());
}

void Fluf::HookIClient(char* client, bool unhook, bool local)
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

            // Protection against double hooking
            // happens if someone leaves a server and then reconnects without going back to the main menu
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
        auto dllHandle = GetModuleHandleA(dllName);
        using GetChatServer = IChatServer*(__stdcall*)();
#ifndef FLUF_DISABLE_HOOKS
        if (_stricmp(dllName, "rpclocal.dll") == 0)
        {
            instance->HookIClient(reinterpret_cast<char*>(GetProcAddress(dllHandle, "Client")), false, true);
            instance->HookIServer(reinterpret_cast<char*>(GetProcAddress(dllHandle, "Server")), false);
            instance->clientServerCommunicator->clientChatServer = reinterpret_cast<GetChatServer>(GetProcAddress(res, "GetChatServerInterface"))();
        }
        else if (_stricmp(dllName, "remoteserver.dll") == 0)
        {
            instance->HookIClient(reinterpret_cast<char*>(GetProcAddress(GetModuleHandleA(nullptr), "Client")), false, false);
            instance->HookIServer(reinterpret_cast<char*>(GetProcAddress(dllHandle, "Server")), false);
            instance->clientServerCommunicator->clientChatServer = reinterpret_cast<GetChatServer>(GetProcAddress(res, "GetChatServerInterface"))();

            // disable no clip, godmode, and see everything, if they are on
            *reinterpret_cast<BYTE*>(0x679cc4) = 0;
            *reinterpret_cast<BYTE*>(0x67ecc0) = 0;
            constexpr byte visitPatch = 0xc3;
            MemUtils::WriteProcMem(0x4c4df1, &visitPatch, 1);
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

BOOL __stdcall Fluf::FreeLibraryDetour(const HMODULE unloadedDll)
{
    std::array<char, MAX_PATH> dllNameBuf{};
    const uint len = GetModuleFileNameA(unloadedDll, dllNameBuf.data(), MAX_PATH);
    std::string_view dllName{ dllNameBuf.data(), len };
    dllName = dllName.substr(dllName.find_last_of('\\') + 1);

    if (instance->runningOnClient)
    {
        if (dllName == "rpclocal.dll")
        {
            instance->HookIClient(reinterpret_cast<char*>(GetProcAddress(unloadedDll, "Client")), true, true);
            instance->HookIServer(reinterpret_cast<char*>(GetProcAddress(unloadedDll, "Server")), true);
            instance->clientServerCommunicator->clientChatServer = nullptr;
        }
        else if (dllName == "remoteserver.dll")
        {
            instance->HookIClient(reinterpret_cast<char*>(GetProcAddress(GetModuleHandleA(nullptr), "Client")), true, false);
            instance->HookIServer(reinterpret_cast<char*>(GetProcAddress(unloadedDll, "Server")), true);
            instance->clientServerCommunicator->clientChatServer = nullptr;
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

        fluf = std::make_shared<Fluf>(mod);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        fluf.reset();
    }

    return TRUE;
}

void Fluf::OnGameLoad()
{
    InfocardOverrides::Initialise();

    Log(LogLevel::Info, "Data loaded, Freelancer ready.");
    for (auto module = loadedModules.begin(); module != loadedModules.end();)
    {
        Log(LogLevel::Trace, std::format("OnGameLoad - {}", module->get()->GetModuleName()));

        try
        {
            module->get()->OnGameLoad();
            ++module;
        }
        catch (std::exception& ex)
        {
            Log(LogLevel::Error, std::format("Exception thrown during OnGameLoad: {}", ex.what()));
            Log(LogLevel::Info, "Unloading module");
            module = loadedModules.erase(module);
        }
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

std::string SetLogMetadata(void* address, const LogLevel level)
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

            return std::format("({}) {}: ", fullPath.substr(fullPath.find_last_of("\\") + 1), levelStr);
        }
    }

    return "";
}

void Fluf::Log(const LogLevel logLevel, const std::string_view message)
{
    if (logLevel < instance->config->logLevel)
    {
        return;
    }

    const std::string paddedMessage = SetLogMetadata(_ReturnAddress(), logLevel) + std::string(message);

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
        switch (logLevel)
        {
            case LogLevel::Trace: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongCyan)); break;
            case LogLevel::Debug: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongGreen)); break;
            case LogLevel::Info: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongWhite)); break;
            case LogLevel::Warn: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongYellow)); break;
            case LogLevel::Error: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongRed)); break;
        }

        std::cout << paddedMessage << std::endl;
    }

    if (logFile)
    {
        logFile->log(static_cast<spdlog::level::level_enum>(logLevel), paddedMessage);
    }
}
void Fluf::Trace(const std::string_view message) { return Log(LogLevel::Trace, message); }
void Fluf::Debug(const std::string_view message) { return Log(LogLevel::Debug, message); }
void Fluf::Info(const std::string_view message) { return Log(LogLevel::Info, message); }
void Fluf::Warn(const std::string_view message) { return Log(LogLevel::Warn, message); }
void Fluf::Error(const std::string_view message) { return Log(LogLevel::Error, message); }

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

uint Fluf::GetPlayerClientId()
{
    const auto* id = reinterpret_cast<uint*>(0x673344);
    return *id;
}

const Universe::ISystem* Fluf::GetPlayerSystem()
{
    const auto* id = reinterpret_cast<uint*>(0x273354);
    return Universe::get_system(*id);
}

Archetype::Ship* Fluf::GetPlayerShipArch()
{
    const auto* shipId = reinterpret_cast<uint*>(0x67337C);
    return Archetype::GetShip(*shipId);
}

EquipDesc* Fluf::GetPlayerEquipDesc() { return reinterpret_cast<EquipDesc*>(0x272960); }

bool Fluf::IsRunningOnClient() { return instance->runningOnClient; }

std::wstring Fluf::GetInfocardName(uint ids)
{
    const auto resourceTable = *reinterpret_cast<void**>(0x67eca8);
    // TODO: Handle infocard overrides

    using GetIdsNameType = size_t (*)(void* rsc, uint number, wchar_t* buffer, size_t bufferSize);
    auto getIdsName = reinterpret_cast<GetIdsNameType>(0x4347e0);
    static std::array<wchar_t, 4096> buffer;

    auto charactersRead = getIdsName(resourceTable, ids, buffer.data(), buffer.size());
    if (!charactersRead)
    {
        return {};
    }

    std::wstring ret{ buffer.data(), charactersRead };
    std::memset(buffer.data(), 0, buffer.size()); // Clear buffer when done
    return ret;
}

bool Fluf::GetInfocard(uint ids, RenderDisplayList* rdl)
{
    using GetInfocardType = bool (*)(uint number, RenderDisplayList* rdl);
    static auto getInfocard = reinterpret_cast<GetInfocardType>(0x57DA40);

    return getInfocard(ids, rdl);
}

KeyManager* Fluf::GetKeyManager() { return instance->keyManager.get(); }
ClientServerCommunicator* Fluf::GetClientServerCommunicator() { return instance->clientServerCommunicator.get(); }

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

Fluf::Fluf(const HMODULE dll)
{
    thisDll = dll;
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

        if (config->logSinks.contains(LogSink::File))
        {
            std::array<char, MAX_PATH> path;
            GetUserDataPath(path.data());

            std::string_view pathPartial{ path.data(), strlen(path.data()) };
            logFile = spdlog::rotating_logger_st("file_logger", std::format("{}\\fluf.log", pathPartial), 1048576 * 5, 3);
            logFile->set_pattern("[%H:%M:%S %z] %v");
        }

        if (config->writeSpewToLogSinks)
        {
            auto fdump = reinterpret_cast<FDump*>(GetProcAddress(GetModuleHandleA("dacom.dll"), "FDUMP"));
            fdumpOriginal = *fdump;
            *fdump = reinterpret_cast<FDump>(FDumpDetour);
        }

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
             NULL_CLIENT_PATCH(unknown_126),
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
             NULL_SERVER_PATCH(Startup),
             NULL_SERVER_PATCH(Shutdown),
             NULL_SERVER_PATCH(Update),
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

    clientServerCommunicator = std::make_unique<ClientServerCommunicator>();

    SetDllDirectoryA("modules/");

    // Load all dlls as needed
    std::vector<std::string> preloadModules;
    std::copy(config->modules.begin(), config->modules.end(), std::back_inserter(preloadModules));
    std::ranges::sort(preloadModules, [](const std::string& a, const std::string& b) { return a.starts_with("FLUF") && !b.starts_with("FLUF"); });
    for (const auto& modulePath : preloadModules)
    {
        Log(LogLevel::Info, std::format("Loading Module: {}", modulePath));
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
            Log(LogLevel::Error, std::format("Failed to load module: {}\nReason: {}", modulePathWithDir, GetLastErrorAsString()));
            continue;
        }

        const auto factory = reinterpret_cast<std::shared_ptr<FlufModule> (*)()>(GetProcAddress(lib, "ModuleFactory"));
        if (!factory)
        {
            Log(LogLevel::Error, std::format("Loaded module did not have ModuleFactory export: {}", modulePath));
            continue;
        }

        std::shared_ptr<FlufModule> module;
        try
        {
            module = factory();
        }
        catch (const ModuleLoadException& e)
        {
            Log(LogLevel::Error, e.what());
            continue;
        }

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
    constexpr char newSavePath[] = "..\\SAVES";
    memcpy_s(path, MAX_PATH, newSavePath, sizeof(newSavePath));
    return true;
}
