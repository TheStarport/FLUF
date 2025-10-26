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
        bool customFramerateFlag = false;
        int customFramerateIndex = 7;

        // HUD

        bool customPowerBarColor = false;
        bool customShieldBarColor = false;
        bool customHealthBarColor = false;
        DWORD newPowerColor = 0;
        DWORD newShieldColor = 0;
        DWORD newHealthColor = 0;
        bool customStatusBarWidth = false;
        double statusBarWidth = 0.85;
        bool disableTargetingBrackets = false;
        bool customiseDamageIndicatorTime = false;
        float damageIndicatorFadeTime = 0.8f;

        // Chat

        bool disableNewPlayerMessages = false;
        bool disableDepartingPlayerMessages = false;
        bool customNewDepartingPlayerColor = false;
        DWORD newPlayerColor = 0;
        bool disableMultiplayerChat = false;

        // Control
        bool customiseHullScreenShake = false;
        bool customiseShieldScreenShake = false;
        float hullScreenShakeValue = 0.02f;
        float shieldScreenShakeValue = 0.005f;
        bool disableDefaultAutoLevel = false;
        bool backgroundAudio = false;

        // Color
        bool showInactiveShield = true;
        DWORD inactiveShieldColor = 0xFF2E2D85;

        bool customFrozenShieldBarColor = false;
        DWORD frozenShieldBarColor = 0xFFEAEAEA;

};
