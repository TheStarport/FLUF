#pragma once

#include <vector>
#include <string>

struct StoryFactionsConfig
{
    static constexpr char path[] = "modules/config/story_factions.yml";
    
    // List of faction names that should be considered story factions
    std::vector<std::string> factions;
};