#pragma once

#include "Config.hpp"
#include "CrashCatcher.hpp"
#include "FlufModule.hpp"

#include <rfl/Hex.hpp>

struct ErrorPayload
{
        std::string moduleName;
        rfl::Hex<unsigned long> offset;
        std::string author;
        std::string description;
        int64_t dateAdded;
};

class CrashCatcher;

/**
 * @author Laz
 * @brief FLU CrashWalker wraps Freelancer within a global try/catch using Windows SEH. This allows us to run code when a crash occurs that would normally
 * take Freelancer down.
 * @par Crash Logging
 * On startup, this module will attempt to pull the latest payload from The Starport's crash offset list and cache it.
 * When a crash occurs, a lookup will be done against this list and inform the user why the game crashed, if the reason is known.
 * Additionally on every crash a dump file is generated in the save directory for debugging purposes.
 *
 * @par Crash Prevention
 * Crash Walker also implements a stripped down version of CrashCatcher from FLHook designed for single player. This should prevent some issues that might
 * have crashed the game previously from doing so.
 */
class FlufCrashWalker final : public FlufModule
{
        void OnGameLoad() override;
        void OnDllLoaded(std::string_view dllName, HMODULE dllPtr) override;
        void OnDllUnloaded(std::string_view dllName, HMODULE dllPtr) override;

        void LoadErrorPayloadFromCache(std::string_view path);
        static int __stdcall GlobalExceptionHandler(EXCEPTION_POINTERS* exceptionPointers);

        inline static std::unique_ptr<CrashCatcher> crashCatcher;
        inline static std::unique_ptr<Config> config;
        std::vector<ErrorPayload> possibleErrors;

    public:
        static void TryCatchDetour(double time);
        static constexpr std::string_view moduleName = "FLUF CrashWalker";
        std::string_view GetModuleName() override;
        ErrorPayload* FindError(std::string_view module, size_t offset);

        FlufCrashWalker();
        ~FlufCrashWalker() override;
};
