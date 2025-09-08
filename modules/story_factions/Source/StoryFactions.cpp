/*
 * Originally written by Jason Hood, 2010
 * Ported to FLUF by Josbyte, 2025
 */

#include "PCH.hpp"

#include "StoryFactions.hpp"
#include "StoryFactionsConfig.hpp"
#include "Fluf.hpp"
#include "Utils/MemUtils.hpp"

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

StoryFactions::StoryFactions()
{
    AssertRunningOnClient;

    Fluf::Info("Story Factions module initialized");

    // Load configuration
    config = std::make_shared<StoryFactionsConfig>(*ConfigHelper<StoryFactionsConfig>::Load(StoryFactionsConfig::path));

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
    const uint32_t offset11 = *reinterpret_cast<uint32_t*>(OFFSET11 + 1);

    constexpr uint32_t expected11 = 0x63ed5d8;

    uint32_t offset = 0;

    // If offset is not valid
    if (offset11 != expected11)
    {
        return;
    }

    offset = OFFSET11;

    // Last array
    const uint32_t factionsAddress = reinterpret_cast<uint32_t>(factions.data());

    MemUtils::WriteProcMem(offset + 0x1, &factionsAddress, sizeof(uint32_t));
    MemUtils::WriteProcMem(offset + 0xA, &factionsAddress, sizeof(uint32_t));
    MemUtils::WriteProcMem(offset + 0x4E2, &factionsAddress, sizeof(uint32_t));
    MemUtils::WriteProcMem(offset + 0x530, &factionsAddress, sizeof(uint32_t));
    MemUtils::WriteProcMem(offset + 0x571, &factionsAddress, sizeof(uint32_t));

    Fluf::Info("Story Factions: Applied patches successfully");
}

SETUP_MODULE(StoryFactions);
