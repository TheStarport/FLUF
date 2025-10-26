#include "PCH.hpp"

#include "QolPatcher.hpp"

void QolPatcher::RegisterControlPatches()
{
    auto& category = memoryPatches["Control"];

    OPTION("Background audio",
           "Enables game audio to continue playing in the background",
           &config->backgroundAudio,
           false,
           true,
           PATCH("soundmanager.dll", { 0x00A021 }, 0x80),
           PATCH("soundstreamer.dll", { 0x0018A9 }, 0x80));

    OPTION("Hull Screen Shake",
           "Customise how much screen shake occurs when your hull is hit",
           &config->customiseHullScreenShake,
           false,
           true,
           new ValuePatch{ "", { 0x11DB5C }, &config->hullScreenShakeValue, 0.0f, 0.1f });

    OPTION("Shield Screen Shake",
           "Customise how much screen shake occurs when your shield is hit",
           &config->customiseShieldScreenShake,
           false,
           true,
           new ValuePatch{ "", { 0x11DB89 }, &config->shieldScreenShakeValue, 0.0f, 0.1f });

    OPTION("Disable Auto-level by default",
           "By default your ship will rotate to be level on the Y-axis. When active, this disables it by default, but it can still toggle it back on with "
           "keybind.",
           &config->disableDefaultAutoLevel,
           false,
           true,
           PATCH("common.dll", { 0x086542 }, 0x00),
           PATCH("", { 0x212700 }, 0x00));
}
