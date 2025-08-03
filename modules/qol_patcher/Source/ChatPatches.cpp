#include "PCH.hpp"

#include "QolPatcher.hpp"

void QolPatcher::RegisterChatPatches()
{
    auto& category = memoryPatches["Chat"];

    OPTION("Disable Player Joining Messages",
           "Disables the messages that appear when a player joins the server",
           &config->disableNewPlayerMessages,
           false,
           PATCH("", 0x06AAF8, 0x90, 0xE9));

    OPTION("Disable Player Departing Messages",
           "Disables the messages that appear when a player leaves the server",
           &config->disableDepartingPlayerMessages,
           false,
           PATCH("", 0x06AF1E, 0x90, 0xE9));

    OPTION("New/Departing Player Color",
           "Customise the color of the the new / departing player messages",
           &config->customNewDepartingPlayerColor,
           false,
           new ColorPatch{ "", 0x05E01B, &config->newPlayerColor });
}
