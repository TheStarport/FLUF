/*
  storyfactions.cpp - Patch Freelancer to define the story factions.

  Jason Hood, 23 March, 2010.

  The story factions are those that are excluded from the reputation list.  This
  plugin overrides the defaults and reads them from DATA\storyfactions.ini.

  Fluf adaptation by Josbyte, 2025 
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

void StoryFactions::LoadFactions()
{
    // Deprecated - kept for backward compatibility but does nothing
    Fluf::Warn("Story Factions: LoadFactions() is deprecated - configure factions in YAML instead");
}
void StoryFactions::ApplyPatches()
{
    if (factions.empty())
    {
        Fluf::Warn("Story Factions: No factions loaded, skipping patches");
        return;
    }

    // Lee los valores que hay en OFFSET11+1 y OFFSET10+1 (para inspección con breakpoints)
    uint32_t offset11_value = *(uint32_t*)(OFFSET11 + 1);
    uint32_t offset10_value = *(uint32_t*)(OFFSET10 + 1);

    // Variables separadas para depuración
    constexpr uint32_t expected11 = 0x63ed5d8;
    constexpr uint32_t expected10 = 0x63ec5d8;

    uint32_t offset = 0;

    // Elige el offset en función del contenido
    if (offset11_value == expected11)
    {
        offset = OFFSET11;
    }
    else if (offset10_value == expected10)
    {
        offset = OFFSET10;
        ;
    }
    else
    {
        return;
    }

    // Agrega null terminator al final
    factions.push_back("");

    // Dirección del array final
    const uint32_t factionsAddress = reinterpret_cast<uint32_t>(factions.data());

    // Muestra por consola para depuración
    Fluf::Debug("Factions memory address: 0x{:X}");

    // Cambia protección de memoria
    MemUtils::ProtectExecuteReadWrite(reinterpret_cast<void*>(offset), 0x575);

    // Aplica todos los parches
    *(uint32_t*)(offset + 0x001) = factionsAddress;
    *(uint32_t*)(offset + 0x00a) = factionsAddress;
    *(uint32_t*)(offset + 0x4e2) = factionsAddress;
    *(uint32_t*)(offset + 0x530) = factionsAddress;
    *(uint32_t*)(offset + 0x571) = factionsAddress;

    Fluf::Info("Story Factions: Applied patches successfully");
}

SETUP_MODULE(StoryFactions);
