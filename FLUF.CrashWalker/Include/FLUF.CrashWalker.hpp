#pragma once

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
        void OnGameLoad() override;
        static HINSTANCE __stdcall LoadLibraryDetour(LPCSTR libName);
        inline static std::unique_ptr<CrashCatcher> crashCatcher;
        std::vector<ErrorPayload> possibleErrors;

    public:
        static constexpr std::string_view moduleName = "FLUF.CrashWalker";
        std::string_view GetModuleName() override;
        ErrorPayload* FindError(std::string_view module, size_t offset);

        FlufCrashWalker();
        ~FlufCrashWalker() override;
};
