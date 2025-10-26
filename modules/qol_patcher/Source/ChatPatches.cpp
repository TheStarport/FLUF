#include "PCH.hpp"

#include "QolPatcher.hpp"

void QolPatcher::RegisterChatPatches()
{
    auto& category = memoryPatches["Chat"];

    OPTION("Disable Player Joining Messages",
           "Disables the messages that appear when a player joins the server",
           &config->disableNewPlayerMessages,
           false,
           true,
           PATCH("", { 0x06AAF8 }, 0x90, 0xE9));

    OPTION("Disable Player Departing Messages",
           "Disables the messages that appear when a player leaves the server",
           &config->disableDepartingPlayerMessages,
           false,
           true,
           PATCH("", { 0x06AF1E }, 0x90, 0xE9));

    OPTION("New/Departing Player Color",
           "Customise the color of the the new / departing player messages",
           &config->customNewDepartingPlayerColor,
           false,
           true,
           new ColorPatch{ "", { 0x05E01B }, &config->newPlayerColor });

    OPTION("Disable Multiplayer Chat",
           "Disables the chat from appearing on screen entirely. Useful for recording gameplay footage. "
           "Please ensure that this is allowed on the server you are connected to.",
           &config->disableMultiplayerChat,
           false,
           true,
           PATCH("", { 0x1A6250 }, 0xC2, 0x10, 0x00));
}
