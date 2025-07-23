#pragma once

struct PatcherConfig
{
        static constexpr char path[] = "qol_patcher.yml";

        bool borderlessWindow = true;
        bool customPowerBarColor = false;
        bool customShieldBarColor = false;
        bool customHealthBarColor = false;
        DWORD newPowerColor = 0;
        DWORD newShieldColor = 0;
        DWORD newHealthColor = 0;
        bool customStatusBarWidth = false;
        double statusBarWidth = 0.85;
};
