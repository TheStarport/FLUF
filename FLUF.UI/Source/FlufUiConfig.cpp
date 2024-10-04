#include "PCH.hpp"

#include <FLCore/FLCoreDefs.hpp>
#include <rfl/yaml.hpp>

#include <FLCore/Common.hpp>

#include "FlufUiConfig.hpp"

std::string_view FlufUiConfig::GetSaveLocation()
{
    static std::string savePath;
    if (savePath.empty())
    {
        std::array<char, MAX_PATH> totalPath;
        GetUserDataPath(totalPath.data());
        savePath = std::format("{}/FLUF.UI.yml", std::string(totalPath.data()));
    }

    return { savePath };
}

bool FlufUiConfig::Save()
{
    std::ofstream outFile(GetSaveLocation().data(), std::ios::trunc);
    if (!outFile.is_open())
    {
        return false;
    }

    outFile << rfl::yaml::write(this);
    outFile.close();

    return true;
}

void FlufUiConfig::Load()
{
    std::ifstream inFile(GetSaveLocation().data());
    if (!inFile.is_open())
    {
        Save();
        return;
    }

    auto newConfig = rfl::yaml::read<FlufUiConfig>(inFile);
    if (newConfig.error().has_value())
    {
        inFile.close();
        Save();
        return;
    }

    // Replace this config with the new one
    memcpy(this, &newConfig.value(), sizeof(FlufUiConfig));
}
