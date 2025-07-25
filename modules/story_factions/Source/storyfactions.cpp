/*
  storyfactions.cpp - Patch Freelancer to define the story factions.

  Jason Hood, 23 March, 2010.

  The story factions are those that are excluded from the reputation list.  This
  plugin overrides the defaults and reads them from DATA\storyfactions.ini.

*/

#include "PCH.hpp"

#include "StoryFactions.hpp"
#include "StoryFactionsConfig.hpp"
#include "Fluf.hpp"
#include "Utils/MemUtils.hpp"

#include <filesystem>
#include <fstream>
#include <string>

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

StoryFactions::StoryFactions()
{
    Fluf::Info("Story Factions module initialized");

    // Load configuration
    config = std::make_shared<StoryFactionsConfig>(*ConfigHelper<StoryFactionsConfig, StoryFactionsConfig::path>::Load());

    // If we have factions in the config, use them
    if (!config->factions.empty())
    {
        Fluf::Info("Story Factions: Loaded {} factions from config");
        factions = config->factions;
    }
    else
    {
        Fluf::Warn("Story Factions: No factions configured in YAML file");
    }
}

std::string_view StoryFactions::GetModuleName() { return moduleName; }

void StoryFactions::OnGameLoad()
{
    Fluf::Info("Story Factions: Loading factions");
    if (factions.empty())
    {
        Fluf::Warn("Story Factions: No factions loaded - check YAML configuration");
    }
    ApplyPatches();
}

void StoryFactions::ApplyPatches()
{
    if (factions.empty())
    {
        Fluf::Warn("Story Factions: No factions loaded, skipping patches");
        return;
    }

    // Reads whats on OFFSET11+1 y OFFSET10+1, separated for better readability
    uint32_t offset11_value = *(uint32_t*)(OFFSET11 + 1);

    constexpr uint32_t expected11 = 0x63ed5d8;

    uint32_t offset = 0;

    // If offset is not valid
    if (offset11_value != expected11)
    {
        return;
        
    }
    offset = OFFSET11;

    // Last array
    const uint32_t factionsAddress = reinterpret_cast<uint32_t>(factions.data());

    // Change mem protection
    MemUtils::ProtectExecuteReadWrite(reinterpret_cast<void*>(offset), 0x575);

    // Apply patches
    *(uint32_t*)(offset + 0x001) = factionsAddress;
    *(uint32_t*)(offset + 0x00a) = factionsAddress;
    *(uint32_t*)(offset + 0x4e2) = factionsAddress;
    *(uint32_t*)(offset + 0x530) = factionsAddress;
    *(uint32_t*)(offset + 0x571) = factionsAddress;

    Fluf::Info("Story Factions: Applied patches successfully");
}

SETUP_MODULE(StoryFactions);
