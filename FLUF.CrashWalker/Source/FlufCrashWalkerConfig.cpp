#include "PCH.hpp"

#include "FLCore/Common/CommonMethods.hpp"
#include "FlufCrashWalkerConfig.hpp"
#include <rfl/yaml.hpp>

std::string_view FlufCrashWalkerConfig::GetSaveLocation()
{
    static std::string savePath;
    if (savePath.empty())
    {
        std::array<char, MAX_PATH> totalPath{};
        GetUserDataPath(totalPath.data());
        savePath = std::format("{}/FLUF.CrashWalker.yml", std::string(totalPath.data()));
    }

    return { savePath };
}

bool FlufCrashWalkerConfig::Save()
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

void FlufCrashWalkerConfig::Load()
{
    std::ifstream inFile(GetSaveLocation().data());
    if (!inFile.is_open())
    {
        Save();
        return;
    }

    auto newConfig = rfl::yaml::read<FlufCrashWalkerConfig>(inFile);
    if (newConfig.error().has_value())
    {
        inFile.close();
        Save();
        return;
    }

    // Replace this config with the new one
    memcpy(this, &newConfig.value(), sizeof(FlufCrashWalkerConfig));
}
