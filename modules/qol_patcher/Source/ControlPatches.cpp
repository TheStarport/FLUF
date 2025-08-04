#include "PCH.hpp"

#include "QolPatcher.hpp"

void QolPatcher::RegisterControlPatches()
{
    auto& category = memoryPatches["Control"];

    OPTION("Hull Screen Shake",
           "Customise how much screen shake occurs when your hull is hit",
           &config->customiseHullScreenShake,
           false,
           new ValuePatch{ "", { 0x11DB5C }, &config->hullScreenShakeValue, 0.0f, 0.1f });

    OPTION("Shield Screen Shake",
           "Customise how much screen shake occurs when your shield is hit",
           &config->customiseShieldScreenShake,
           false,
           new ValuePatch{ "", { 0x11DB89 }, &config->shieldScreenShakeValue, 0.0f, 0.1f });

    OPTION("Disable Auto-level by default",
           "By default your ship will rotate to be level on the Y-axis. When active, this disables it by default, but it can still toggle it back on with "
           "keybind.",
           &config->disableDefaultAutoLevel,
           false,
           PATCH("common.dll", { 0x086542 }, 0x00),
           PATCH("", { 0x212700 }, 0x00));
}
