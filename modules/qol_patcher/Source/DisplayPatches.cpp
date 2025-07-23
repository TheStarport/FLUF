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
}
