#include "PCH.hpp"

#include "QolPatcher.hpp"

void QolPatcher::RegisterDisplayPatches()
{
    auto& category = memoryPatches["Display"];

    OPTION("Borderless Window",
           "Removes the window borders when in windowed mode",
           &config->borderlessWindow,
           true,
           PATCH("", { 0x02477A }, 0x00, 0x00),
           PATCH("", { 0x002490D }, 0x00, 0x00));

    OPTION("Rendering Distance (Solar)",
           "The maximum rendering distance for small solar objects like trade lanes and battleships",
           &config->customSolarRendering,
           false,
           new ValuePatch{ "", { 0x213EC8 }, &config->customSolarRenderDistance, 5'000.f, 100'000.0f });

    OPTION("Rendering Distance (Ships)",
           "The maximum rendering distance for small objects like ships and storage wrecks",
           &config->customShipRendering,
           false,
           new ValuePatch{ "", { 0x1C8910 }, &config->customShipRenderDistance, 10'000.f, 100'000.0f });

    OPTION("LOD Ranges",
           "Configure the 'Level Of Detail' for game assets with a flat multiplier",
           &config->customLod,
           false,
           // clang-format off
           PATCH("", { 0x2389 }, 0xE8, 0x7B, 0xFF, 0xFF, 0xFF),
           // clang-format on
           PATCH("", { 0x2309 }, 0xD8, 0x0D),
           PATCH("", { 0x230E }, 0x00, 0xC3),
           new OptionPatch("",
                           { 0x230B },
                           3,
                           &config->lodOptionIndex,
                           {
                               { "x1.5", { 0x4B, 0x1A, 0x47 } },
                               { "x2.0", { 0xB4, 0x55, 0x5D } },
                               { "x3.0", { 0x28, 0x4F, 0x5D } },
                               { "x4.0", { 0x9C, 0xFC, 0x5C } },
                               { "x5.0", { 0x64, 0x84, 0x5D } },
                               { "x6.0", { 0x08, 0x4F, 0x5D } },
                               { "x7.0", { 0x50, 0x88, 0x5D } },
                               { "x8.0", { 0x54, 0x23, 0x44 } },
                               { "x10.0", { 0xFA, 0x1F, 0x57 } },
                           }));

    OPTION("Contact Window Refresh Rate",
           "How frequently the contact window should be updated",
           &config->customWindowRefreshRate,
           false,
           new ValuePatch{ "", { 0x1D7964 }, &config->windowRefreshRate, 0.0f, 5.0f });

    auto gameRefreshRatePatch = new OptionPatch{
        "",
        { 0x0210A0C },
        sizeof(float),
        &config->customFramerateIndex,
        {
          { "30 FPS", RawValue(30.f).data() },
          { "45 FPS", RawValue(45.f).data() },
          { "60 FPS", RawValue(60.f).data() },
          { "75 FPS", RawValue(75.f).data() },
          { "90 FPS", RawValue(90.f).data() },
          { "120 FPS", RawValue(120.f).data() },
          { "144 FPS", RawValue(144.f).data() },
          { "154 FPS", RawValue(154.f).data() },
          }
    };

    OPTION("Game Refresh Rate",
           "Use Freelancer's inbuilt framerate limiter to cap your framerate",
           &config->customFramerateFlag,
           false,
           PATCH("common.dll", { 0x01A892 }, 0x4C, 0xA7, 0x27),
           gameRefreshRatePatch,
           new ReusablePatch{ "common.dll", { 0x01A74C }, gameRefreshRatePatch });
}
