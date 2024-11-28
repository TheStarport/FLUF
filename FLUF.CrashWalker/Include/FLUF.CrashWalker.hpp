#pragma once

#include "Config.hpp"
#include "CrashCatcher.hpp"
#include "FLUF/Include/FlufModule.hpp"

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
class FlufCrashWalker final : public FlufModule
{
        void LoadErrorPayloadFromCache(std::string_view path);
        void OnGameLoad() override;
        void OnDllLoaded(std::string_view dllName, HMODULE dllPtr) override;
        void OnDllUnloaded(std::string_view dllName, HMODULE dllPtr) override;
        static int __stdcall GlobalExceptionHandler(EXCEPTION_POINTERS* exceptionPointers);
        inline static std::unique_ptr<CrashCatcher> crashCatcher;
        inline static std::unique_ptr<Config> config;
        std::vector<ErrorPayload> possibleErrors;

    public:
        static void TryCatchDetour(double time);
        static constexpr std::string_view moduleName = "FLUF.CrashWalker";
        std::string_view GetModuleName() override;
        ErrorPayload* FindError(std::string_view module, size_t offset);

        FlufCrashWalker();
        ~FlufCrashWalker() override;
};
