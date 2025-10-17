#pragma once

#include "FlufModule.hpp"

#include <Utils/Detour.hpp>

#include "ImportFluf.hpp"

#include "Internal/Hooks/ClientSend.hpp"
#include "VTables.hpp"

#include <memory>

class ClientServerCommunicator;
class IEngineHook;
enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warn,
    Error,
};

enum class LogSink
{
    Console,
    File
};

class FlufModule;
struct IClientImpl;
struct IServerImpl;
class ClientSend;
class ClientReceive;
class FlufConfiguration;
struct CShip;
struct SStartupInfo;
class KeyManager;
struct EquipDesc;

namespace Universe
{
    struct ISystem;
}

namespace Archetype
{
    struct Ship;
}

class Fluf
{
        friend KeyManager;
        using GetUserDataPathSig = bool (*)(char*);

        friend ClientSend;
        friend ClientReceive;
        friend ClientServerCommunicator;
        friend IEngineHook;
        friend FlufModule;
        inline static HMODULE thisDll;
        inline static Fluf* instance;

        std::vector<std::shared_ptr<FlufModule>> loadedModules{};
        std::shared_ptr<FlufConfiguration> config;

        // FLUF can run on the client or server. This becomes true when the process is 'freelancer.exe'
        bool runningOnClient = false;
        inline static FARPROC oldServerStartupFunc;
        inline static auto getUserDataPathDetour =
            FunctionDetour(reinterpret_cast<GetUserDataPathSig>(GetProcAddress(GetModuleHandleA("common.dll"), "?GetUserDataPath@@YA_NQAD@Z")));

        std::array<VTablePatch, static_cast<int>(IClientVTable::Count)> clientPatches;
        std::array<VTablePatch, static_cast<int>(IServerVTable::Count)> serverPatches;

        std::unique_ptr<KeyManager> keyManager;

        static void OnUpdateHook(double delta);
        static void* OnScriptLoadHook(const char* file);
        static void LoadCommonHooks();
        static void LoadServerHooks();
        void OnGameLoad();
        static bool __thiscall OnServerStart(IServerImpl* server, SStartupInfo& info);

        void HookIClient(char* client, bool unhook, bool local);
        void HookIServer(char* server, bool unhook);
        void SetupHooks();

        static HINSTANCE __stdcall LoadLibraryDetour(LPCSTR libName);
        static bool __fastcall DelayedRPCLocalDetour(void* _this, void* edx, void* dunno1, void* dunno2);
        static void OnPhysicsUpdateDetour(uint system, float delta);
        using RPCLocalDetourType = bool(__fastcall*)(void* _this, void* edx, void* dunno1, void* dunno2);
        inline static std::unique_ptr<FunctionDetour<RPCLocalDetourType>> delayedRPCLocalDetour;
        static BOOL __stdcall FreeLibraryDetour(HMODULE module);
        static bool GetUserDataPathDetour(char* path);

        template <typename R, typename... Args>
        struct [[maybe_unused]] ReturnType;

        template <class Class, typename R, typename... Args>
        struct [[maybe_unused]] ReturnType<R (Class::*)(Args...)>
        {
                using Type = R;
        };

        template <typename FuncPtr, typename... Args>
        bool CallModuleEvent(FuncPtr target, Args&&... args) const
        {
            using ReturnType = typename ReturnType<FuncPtr>::Type;
            constexpr bool returnTypeIsVoid = std::is_same_v<ReturnType, void>;
            using NoVoidReturnType = std::conditional_t<returnTypeIsVoid, int, ReturnType>;

            NoVoidReturnType ret{};
            bool handled = false;
            for (const auto& module : loadedModules)
            {
                auto& moduleRef = *module;
                if constexpr (returnTypeIsVoid)
                {
                    (moduleRef.*target)(std::forward<Args>(args)...);
                }
                else
                {
                    ret = (moduleRef.*target)(std::forward<Args>(args)...);
                    if constexpr (std::is_same_v<NoVoidReturnType, bool>)
                    {
                        if (!static_cast<bool>(ret))
                        {
                            return false;
                        }
                    }
                    else if constexpr (std::is_enum_v<NoVoidReturnType>)
                    {
                        const auto code = static_cast<FlufModule::ModuleProcessCode>(ret);
                        if (code == FlufModule::ModuleProcessCode::Handled)
                        {
                            return true;
                        }

                        if (code == FlufModule::ModuleProcessCode::Continue)
                        {
                            handled = true;
                        }
                    }
                }
            }

            return !handled;
        }

        using LoadResourceDll = bool (*)(const char*);
        using ForceLoadResource = DWORD (*)(HINSTANCE);
        inline static std::unique_ptr<FunctionDetour<LoadResourceDll>> loadResourceDllDetour;
        static bool ResourceDllLoadDetour(const char* unk);
        // What IDS range is given to FLUF.dll
        inline static DWORD startingResourceIndex = 0;

        // Hardcoded patches that we realistically want to always apply
        static void ClientPatches();

        std::unique_ptr<ClientServerCommunicator> clientServerCommunicator;

    public:
        explicit Fluf(HMODULE dll);
        ~Fluf();

        FLUF_API static void Log(LogLevel logLevel, std::string_view message);
        FLUF_API static void Trace(std::string_view message);
        FLUF_API static void Debug(std::string_view message);
        FLUF_API static void Info(std::string_view message);
        FLUF_API static void Warn(std::string_view message);
        FLUF_API static void Error(std::string_view message);
        void SetupLogging() const;

        FLUF_API static std::weak_ptr<FlufModule> GetModule(std::string_view identifier);
        FLUF_API static CShip* GetPlayerCShip();
        FLUF_API static Ship* GetPlayerIObj();
        FLUF_API static unsigned int GetPlayerClientId();
        FLUF_API static const Universe::ISystem* GetPlayerSystem();
        FLUF_API static Id GetPlayerSystemId();
        FLUF_API static Archetype::Ship* GetPlayerShipArch();
        FLUF_API static Id GetPlayerShipArchId();
        FLUF_API static EquipDescList* GetPlayerEquipDesc();
        FLUF_API static bool IsRunningOnClient();
        FLUF_API static std::wstring GetInfocardName(uint ids);
        FLUF_API static bool GetInfocard(uint ids, RenderDisplayList* rdl);

        /**
         * @brief Gets the KeyManager for setting up custom key callbacks
         * @return A ptr to the KeyManager, but only if IsRunningOnClient() return true, else a nullptr
         */
        FLUF_API static KeyManager* GetKeyManager();

        FLUF_API static ClientServerCommunicator* GetClientServerCommunicator();
};
