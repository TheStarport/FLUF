#include "PCH.hpp"

#include "QolPatcher.hpp"

void QolPatcher::RegisterHudPatches()
{
    auto& category = memoryPatches["HUD"];

    OPTION("Energy Bar Color",
           "Customise the color of the energy bar",
           &config->customPowerBarColor,
           false,
           new ColorPatch{ "", 0x0D57AC, &config->newPowerColor });

    OPTION("Shield Bar Color",
           "Customise the color of the shield bar",
           &config->customShieldBarColor,
           false,
           new ColorPatch{ "", 0x0D5843, &config->newShieldColor });

    OPTION("Health Bar Color",
           "Customise the color of the health bar",
           &config->customHealthBarColor,
           false,
           new ColorPatch{ "", 0x0D588D, &config->newHealthColor });

    OPTION("Status Bar Width",
           "Change the width of the status bars. 1.0 will make the status bars solid.",
           &config->customStatusBarWidth,
           false,
           new ValuePatch{ "", 0x1D7E50, &config->statusBarWidth, 0.05, 1.0 });
}
