#pragma once

#include <Windows.h>
#include "FlufModule.hpp"
#include "StoryFactionsConfig.hpp"
#include <vector>
#include <string>
#include <memory>
#include <cstdint>

class StoryFactions final : public FlufModule
{
private:
    std::shared_ptr<StoryFactionsConfig> config;
    std::vector<std::string> factions;
    
    // Memory addresses for patching
    static constexpr uint32_t OFFSET10 = 0x631f800;
    static constexpr uint32_t OFFSET11 = 0x631f930;
    
    // Load factions from configuration file
    void LoadFactions();
    
    // Apply memory patches
    void ApplyPatches();

public:
    static constexpr std::string_view moduleName = "story_factions";
    
    StoryFactions();
    ~StoryFactions() override = default;
    
    // Required override from FlufModule
    std::string_view GetModuleName() override;
    
    // Called when the game loads
    void OnGameLoad() override;
};