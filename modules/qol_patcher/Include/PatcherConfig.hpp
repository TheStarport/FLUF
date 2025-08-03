#pragma once

struct PatcherConfig
{
        static constexpr char path[] = "qol_patcher.yml";

        // Display
        bool borderlessWindow = true;
        bool customLod = false;
        int lodOptionIndex = 0;
        bool customSolarRendering = false;
        float customSolarRenderDistance = 10000.f;
        bool customShipRendering = false;
        float customShipRenderDistance = 20000.f;
        bool customWindowRefreshRate = false;
        float windowRefreshRate = 3.0f;

        // HUD

        bool customPowerBarColor = false;
        bool customShieldBarColor = false;
        bool customHealthBarColor = false;
        DWORD newPowerColor = 0;
        DWORD newShieldColor = 0;
        DWORD newHealthColor = 0;
        bool customStatusBarWidth = false;
        double statusBarWidth = 0.85;
};
