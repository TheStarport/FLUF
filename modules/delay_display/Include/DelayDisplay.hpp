#pragma once

#include "FlufModule.hpp"

#include <FLCore/Common/CEquip/CAttachedEquip/CELauncher.hpp>
#include "Utils/MemUtils.hpp"

class DelayDisplay final : public FlufModule
{
    public:
        static constexpr std::string_view moduleName = "delay_display";

        DelayDisplay();
        std::string_view GetModuleName() override;
};
