#pragma once

#include "FlufModule.hpp"
#include "FLUF.UI.hpp"

#include "Utils/MemUtils.hpp"

class FlufUi;

class ScreenControl final : public FlufModule
{
        std::shared_ptr<FlufUi> flufUi;
        static void SetBackGroundRunPatch(bool bInWindowedMode);
        static int __stdcall AltEnter(int a1);
        inline static DWORD freelancerExe;
    public:
        static constexpr std::string_view moduleName = "Screen Control";

        static void Patch();

        ScreenControl();
        std::string_view GetModuleName() override;
};
