#include "PCH.hpp"

#include "QolPatcher.hpp"

void QolPatcher::RegisterHudPatches()
{
    auto& category = memoryPatches["HUD"];

    OPTION("Energy Bar Color",
           "Customise the color of the energy bar",
           &config->customPowerBarColor,
           false,
           new ColorPatch{ "", { 0x0D57AC }, &config->newPowerColor });

    OPTION("Shield Bar Color",
           "Customise the color of the shield bar",
           &config->customShieldBarColor,
           false,
           new ColorPatch{ "", { 0x0D5843 }, &config->newShieldColor });

    OPTION("Health Bar Color",
           "Customise the color of the health bar",
           &config->customHealthBarColor,
           false,
           new ColorPatch{ "", { 0x0D588D }, &config->newHealthColor });

    OPTION("Status Bar Width",
           "Change the width of the status bars. 1.0 will make the status bars solid.",
           &config->customStatusBarWidth,
           false,
           new ValuePatch{ "", { 0x1D7E50 }, &config->statusBarWidth, 0.05, 1.0 });

    OPTION("Disable Targeting Brackets",
           "Disable the targeting brackets that appear on ships that are not currently selected.",
           &config->disableTargetingBrackets,
           false,
           PATCH("", { 0x0EC087 }, 0x90, 0xE9));

    OPTION("Damage Indicator Fade Time",
           "The amount of time the damage indicator will be on screen, set to 0 to disable entirely.",
           &config->customiseDamageIndicatorTime,
           false,
           new ValuePatch{ "", { 0x1D800C }, &config->damageIndicatorFadeTime, 0.0f, 1.5f });
}
