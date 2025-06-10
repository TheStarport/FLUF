#pragma once

#include "FlufModule.hpp"

#include <Utils/Detour.hpp>

#include "ImportFluf.hpp"

#include "Internal/Hooks/ClientSend.hpp"
#include "VTables.hpp"

#include <memory>

class ClientServerCommunicator;
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
class Fluf
{
        friend KeyManager;
        using GetUserDataPathSig = bool (*)(char*);

        friend ClientSend;
        friend ClientReceive;
        friend ClientServerCommunicator;
        friend FlufModule;
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
        void OnGameLoad() const;
        static bool __thiscall OnServerStart(IServerImpl* server, SStartupInfo& info);

        void HookIClient(char* client, bool local);
        void HookIServer(char* server);
        static HINSTANCE __stdcall LoadLibraryDetour(LPCSTR libName);
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

        // Hardcoded patches that we realistically want to always apply
        static void ClientPatches();

        std::unique_ptr<ClientServerCommunicator> clientServerCommunicator;

    public:
        Fluf();
        ~Fluf();

        FLUF_API static void Log(LogLevel logLevel, std::string_view message);
        FLUF_API static void Trace(std::string_view message);
        FLUF_API static void Debug(std::string_view message);
        FLUF_API static void Info(std::string_view message);
        FLUF_API static void Warn(std::string_view message);
        FLUF_API static void Error(std::string_view message);

        FLUF_API static std::weak_ptr<FlufModule> GetModule(std::string_view identifier);
        FLUF_API static CShip* GetPlayerCShip();
        FLUF_API static IObjRW* GetPlayerIObjRW();
        FLUF_API static bool IsRunningOnClient();

        /**
         * @brief Gets the KeyManager for setting up custom key callbacks
         * @return A ptr to the KeyManager, but only if IsRunningOnClient() return true, else a nullptr
         */
        FLUF_API static KeyManager* GetKeyManager();

        FLUF_API static ClientServerCommunicator* GetClientServerCommunicator();
};
