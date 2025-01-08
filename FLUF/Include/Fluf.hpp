#pragma once

#include <Utils/Detour.hpp>

#include "ImportFluf.hpp"

#include "VTables.hpp"
#include <memory>

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
    Spew,
    Interface,
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
class Fluf
{
        friend KeyManager;
        using GetUserDataPathSig = bool (*)(char*);

        friend ClientSend;
        friend ClientReceive;
        friend FlufModule;
        inline static Fluf* instance;

        std::vector<std::shared_ptr<FlufModule>> loadedModules{};
        std::shared_ptr<FlufConfiguration> config;

        // FLUF can run on the client or server. This becomes true when the process is 'freelancer.exe'
        bool runningOnClient = false;
        inline static FARPROC oldServerStartupFunc;
        inline static auto getUserDataPathDetour =
            FunctionDetour(reinterpret_cast<GetUserDataPathSig>(GetProcAddress(GetModuleHandleA("common.dll"), "?GetUserDataPath@@YA_NQAD@Z")));

        // The serverClient receives data from the server
        IClientImpl* serverClient = nullptr;
        // The clientServer sends data to the server
        IServerImpl* clientServer = nullptr;

        std::unique_ptr<VTableHook<static_cast<DWORD>(IClientVTable::LocalStart), static_cast<DWORD>(IClientVTable::LocalEnd)>> localClientVTable;
        std::unique_ptr<VTableHook<static_cast<DWORD>(IServerVTable::LocalStart), static_cast<DWORD>(IServerVTable::LocalEnd)>> localServerVTable;
        std::unique_ptr<VTableHook<static_cast<DWORD>(IClientVTable::RemoteStart), static_cast<DWORD>(IClientVTable::RemoteEnd)>> remoteClientVTable;
        std::unique_ptr<VTableHook<static_cast<DWORD>(IServerVTable::RemoteStart), static_cast<DWORD>(IServerVTable::RemoteEnd)>> remoteServerVTable;

        std::unique_ptr<KeyManager> keyManager;

        static void OnUpdateHook(double delta);
        static void* OnScriptLoadHook(const char* file);
        void OnGameLoad() const;
        static bool __thiscall OnServerStart(IServerImpl* server, SStartupInfo& info);

        static HINSTANCE __stdcall LoadLibraryDetour(LPCSTR libName);
        static BOOL __stdcall FreeLibraryDetour(HMODULE module);
        static bool GetUserDataPathDetour(char* path);

        template <typename R, typename... Args>
        struct ReturnType;

        template <class Class, typename R, typename... Args>
        struct ReturnType<R (Class::*)(Args...)>
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
                }
            }

            return true;
        }

    public:
        Fluf();
        ~Fluf();

        FLUF_API static void Log(LogLevel level, std::string_view message);
        FLUF_API static std::weak_ptr<FlufModule> GetModule(std::string_view identifier);
        FLUF_API static CShip* GetPlayerCShip();
        FLUF_API static IObjRW* GetPlayerIObjRW();
        FLUF_API static bool IsRunningOnClient();

        /**
         * @brief Gets the KeyManager for setting up custom key callbacks
         * @return A ptr to the KeyManager, but only if IsRunningOnClient() return true, else a nullptr
         */
        FLUF_API static KeyManager* GetKeyManager();
};
