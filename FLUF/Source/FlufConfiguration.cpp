#include "PCH.hpp"

#include "Internal/FlufConfiguration.hpp"

#include "FlufModule.hpp"

#include <rfl/yaml/read.hpp>
#include <rfl/yaml/write.hpp>

bool FlufConfiguration::Save()
{
    std::ofstream outFile("FLUF.yml", std::ios::trunc);
    if (!outFile.is_open())
    {
        return false;
    }

    outFile << rfl::yaml::write(this);
    outFile.close();

    return true;
}

void FlufConfiguration::Load()
{
    std::ifstream inFile("FLUF.yml");
    if (!inFile.is_open())
    {
        // Load in all FLUF modules by default!

        std::array<char, MAX_PATH> buffer{};
        GetCurrentDirectoryA(buffer.size(), buffer.data());

        for (const auto dir = std::string(buffer.data()); const auto& entry : std::filesystem::directory_iterator(dir))
        {
            if (!entry.is_regular_file() || !entry.path().generic_string().ends_with(".dll") || !entry.path().generic_string().starts_with("FLUF") ||
                entry.path().generic_string() == "FLUF.dll")
            {
                continue;
            }

            modules.emplace(entry.path().generic_string());
        }

        Save();
        return;
    }

    auto newConfig = rfl::yaml::read<FlufConfiguration>(inFile);
    if (newConfig.error().has_value())
    {
        inFile.close();
        Save();
        return;
    }

    *this = newConfig.value();
}
