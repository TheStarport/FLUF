#pragma once

#include "FlufModule.hpp"

struct HexEdit
{
        std::optional<std::string> description;
        rfl::Hex<size_t> offset;
        std::vector<rfl::Hex<size_t>> bytes;
        rfl::Skip<std::vector<uint8_t>> originalBytes;
};

class FlHex final : public FlufModule
{
        static constexpr std::string_view configPath = "modules/config/flhex.yml";
        std::unordered_map<std::string, std::vector<HexEdit>> hexEdits;

        void OnDllLoaded(std::string_view dllName, HMODULE dllPtr) override;

        void Patch(bool restore, std::string_view loadedModuleName = "");

    public:
        static constexpr std::string_view moduleName = "FlHex";

        FlHex();
        ~FlHex() override;
        EXPORT std::string_view GetModuleName() override;
};
