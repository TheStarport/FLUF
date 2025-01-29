#pragma once

#include "FlufModule.hpp"

struct KillMessage
{
    std::string textMessage;
};

class KillNotifier final : public FlufModule
{
    static constexpr std::array<char, 4> killMessageHeader{'k', 'i', 'l', 'l'};
    ModuleProcessCode OnPayloadReceived(uint sourceClientId, std::array<char, 4> header, void* data, size_t size) override;

    public:
        static constexpr std::string_view moduleName = "kill_notifier";

        KillNotifier();
        std::string_view GetModuleName() override;
};
