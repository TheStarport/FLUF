#pragma once

#include "FlufModule.hpp"

struct KillMessage
{
        std::string textMessage;
};

class KillNotifier final : public FlufModule
{
        static constexpr char killMessageHeader[4] = { 'k', 'i', 'l', 'l' };
        ModuleProcessCode OnPayloadReceived(uint sourceClientId, const FlufPayload& payload) override;

    public:
        static constexpr std::string_view moduleName = "kill_notifier";

        KillNotifier();
        std::string_view GetModuleName() override;
};
