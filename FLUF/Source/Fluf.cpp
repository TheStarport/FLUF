#include "Fluf.hpp"
#include "PCH.hpp"

#include "Typedefs.hpp"

#include "ClientServerCommunicator.hpp"
#include "FlufModule.hpp"
#include "Internal/FlufConfiguration.hpp"
#include "Internal/Hooks/ClientReceive.hpp"
#include "Internal/Hooks/InfocardOverrides.hpp"
#include "Utils/MemUtils.hpp"
#include "KeyManager.hpp"

#include <Exceptions.hpp>
#include <spdlog/common.h>
#include <spdlog/sinks/rotating_file_sink.h>

std::shared_ptr<Fluf> fluf;
using ScriptLoadPtr = void* (*)(const char* fileName);
using FrameUpdatePtr = void (*)(double delta);
std::unique_ptr<FunctionDetour<ScriptLoadPtr>> thornLoadDetour;
std::unique_ptr<FunctionDetour<FrameUpdatePtr>> frameUpdateDetour;

extern "C" __declspec(dllexport) void DummyFunction() {}

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

constexpr auto majorVersion = ModuleMajorVersion::One;
constexpr auto minorVersion = ModuleMinorVersion::Zero;

FunctionDetour loadLibraryDetour(LoadLibraryA);

HINSTANCE __stdcall Fluf::LoadLibraryDetour(LPCSTR dllName)
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
        const auto dllHandle = GetModuleHandleA(dllName);
        using GetChatServer = IChatServer*(__stdcall*)();
#ifndef FLUF_DISABLE_HOOKS
        if (_stricmp(dllName, "rpclocal.dll") == 0)
        {
            auto handle = GetModuleHandleA("rpclocal");
            if (delayedRPCLocalDetour)
            {
                delayedRPCLocalDetour.reset();
            }
            delayedRPCLocalDetour = std::make_unique<FunctionDetour<RPCLocalDetourType>>(reinterpret_cast<RPCLocalDetourType>(DWORD(res) + 0xEED0));
            delayedRPCLocalDetour->Detour(DelayedRPCLocalDetour);
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
        if (_stricmp(dllName.data(), "rpclocal.dll") == 0)
        {
            instance->HookIClient(reinterpret_cast<char*>(GetProcAddress(unloadedDll, "Client")), true, true);
            instance->HookIServer(reinterpret_cast<char*>(GetProcAddress(unloadedDll, "Server")), true);
            instance->clientServerCommunicator->clientChatServer = nullptr;
        }
        else if (_stricmp(dllName.data(), "remoteserver.dll") == 0)
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

    if (dllName == "gundll.dll") // prevent unloading of gundll which just loves to hang the process on game exit.
    {
        return true;
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
    // MANUAL PATCH DO NOT COMMIT LAZ
    const auto fl = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    constexpr byte m = 127;
    MemUtils::WriteProcMem(fl + 0x16835B, &m, sizeof(m));
    MemUtils::WriteProcMem(fl + 0x169D2B, &m, sizeof(m));

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

void Fluf::OnUpdateHook(const double delta)
{
    constexpr double SixtyFramesPerSecond = 1.0 / 60.0;
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

Id Fluf::GetPlayerSystemId()
{
    const auto* id = reinterpret_cast<Id*>(0x273354);
    return *id;
}

Archetype::Ship* Fluf::GetPlayerShipArch()
{
    const auto* shipId = reinterpret_cast<uint*>(0x67337C);
    return Archetype::GetShip(*shipId);
}

Id Fluf::GetPlayerShipArchId()
{
    const auto* shipId = reinterpret_cast<Id*>(0x67337C);
    return *shipId;
}

EquipDescList* Fluf::GetPlayerEquipDesc() { return reinterpret_cast<EquipDescList*>(0x672960); }

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
    config = std::make_shared<FlufConfiguration>(*ConfigHelper<FlufConfiguration>::Load(FlufConfiguration::path));

    if (config->setSaveDirectoryRelativeToExecutable)
    {
        getUserDataPathDetour.Detour(GetUserDataPathDetour);
    }

    std::array<char, MAX_PATH> fileNameBuffer{};
    GetModuleFileNameA(nullptr, fileNameBuffer.data(), MAX_PATH);
    std::string_view fileName = fileNameBuffer.data();
    fileName = fileName.substr(fileName.find_last_of('\\') + 1);
    runningOnClient = _strcmpi(fileName.data(), "freelancer.exe") == 0;

    SetupHooks();

    clientServerCommunicator = std::make_unique<ClientServerCommunicator>();

    SetDllDirectoryA("modules/");

    // Load all dlls as needed
    std::vector<std::string> preloadModules;
    if (Fluf::IsRunningOnClient())
    {
        std::copy(config->clientModules.begin(), config->clientModules.end(), std::back_inserter(preloadModules));
    }
    else
    {
        std::copy(config->serverModules.begin(), config->serverModules.end(), std::back_inserter(preloadModules));
    }

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

        const auto fl = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
        frameUpdateDetour = std::make_unique<FunctionDetour<FrameUpdatePtr>>(reinterpret_cast<FrameUpdatePtr>(fl + 0x1B2890));
        frameUpdateDetour->Detour(OnUpdateHook);
    }

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
