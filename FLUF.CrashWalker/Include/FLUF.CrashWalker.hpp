#pragma once

#include "Config.hpp"
#include "CrashCatcher.hpp"
#include "FLUF/Include/FlufModule.hpp"
#include "FlufWalker.hpp"

#include <rfl/Hex.hpp>

struct ErrorPayload
{
        std::string moduleName;
        rfl::Hex<unsigned long> offset;
        std::string author;
        std::string description;
        i64 dateAdded;
};

class CrashCatcher;
class FlufCrashWalker final : public FlufModule
{
        void LoadErrorPayloadFromCache(std::string_view path);
        void OnGameLoad() override;
        static void __stdcall GlobalExceptionHandler(EXCEPTION_POINTERS* exceptionPointers);
        static HINSTANCE __stdcall LoadLibraryDetour(LPCSTR libName);
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
