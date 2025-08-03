#include "PCH.hpp"

#include "QolPatcher.hpp"

void QolPatcher::RegisterDisplayPatches()
{
    auto& category = memoryPatches["Display"];

    OPTION("Borderless Window",
           "Removes the window borders when in windowed mode",
           &config->borderlessWindow,
           true,
           PATCH("", 0x02477A, 0x00, 0x00),
           PATCH("", 0x002490D, 0x00, 0x00));

    OPTION("Rendering Distance (Solar)",
           "The maximum rendering distance for small solar objects like trade lanes and battleships",
           &config->customSolarRendering,
           false,
           new ValuePatch{ "", 0x213EC8, &config->customSolarRenderDistance, 5'000.f, 100'000.0f });

    OPTION("Rendering Distance (Ships)",
           "The maximum rendering distance for small objects like ships and storage wrecks",
           &config->customShipRendering,
           false,
           new ValuePatch{ "", 0x1C8910, &config->customShipRenderDistance, 10'000.f, 100'000.0f });

    OPTION("LOD Ranges",
           "Configure the 'Level Of Detail' for game assets with a flat multiplier",
           &config->customLod,
           false,
           //
           PATCH("", 0x2389, 0xE8, 0x7B, 0xFF, 0xFF, 0xFF),
           PATCH("", 0x2309, 0xD8, 0x0D),
           PATCH("", 0x230E, 0x00, 0xC3),
           new OptionPatch("",
                           0x230B,
                           &config->lodOptionIndex,
                           {
                               {  "x1.5", { 0x4B, 0x1A, 0x47 } },
                               {  "x2.0", { 0xB4, 0x55, 0x5D } },
                               {  "x3.0", { 0x28, 0x4F, 0x5D } },
                               {  "x4.0", { 0x9C, 0xFC, 0x5C } },
                               {  "x5.0", { 0x64, 0x84, 0x5D } },
                               {  "x6.0", { 0x08, 0x4F, 0x5D } },
                               {  "x7.0", { 0x50, 0x88, 0x5D } },
                               {  "x8.0", { 0x54, 0x23, 0x44 } },
                               { "x10.0", { 0xFA, 0x1F, 0x57 } },
    }));

    OPTION("Window Refresh Rate",
           "How frequently the contact window should be updated",
           &config->customWindowRefreshRate,
           false,
           new ValuePatch{ "", 0x1D7964, &config->windowRefreshRate, 0.0f, 5.0f });
}
