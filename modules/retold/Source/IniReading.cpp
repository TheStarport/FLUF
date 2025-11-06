#include <PCH.hpp>

#include <Utils/MemUtils.hpp>
#include <ini.h>
#include <Fluf.hpp>

#include "Retold.hpp"

struct IniUserData
{
        inline static const std::unordered_map<std::string, std::unordered_set<std::string>> multiKeySections{
            {      "zone", { "faction", "encounter", "density_restriction" } },
            { "archetype", { "ship", "snd", "simple", "equipment", "solar" } },
        };

        inline static const std::unordered_set<std::string> sectionsWithoutNickname{ "archetype", "systeminfo", "texturepanels", "dust",      "music",
                                                                                     "asteroids", "nebula",     "ambient",       "background" };

        struct IniSection
        {
                std::string name;
                std::string nickname = "empty";
                std::unordered_map<std::string, std::string> singleKeys;
                std::list<std::pair<std::string, std::string>> multiKeys;
        };

        std::list<IniSection> sections;
        IniSection* currentSection;

        std::string currentNickname;
        uint minStage = 0;
        uint maxStage = 0;
        bool remove = false;

        uint currentStage = 0;
};

// ReSharper disable once CppDFAConstantFunctionResult
static int IniHandler(IniUserData& data, const char* section, const char* key, const char* value)
{
    if (!key && !value)
    {
        if (!data.sections.empty() && !data.currentNickname.empty())
        {
            // If section was marked for removal, or level requirement is not met,
            // we remove all sections with the specified nickname
            if (data.remove)
            {
                data.sections.remove_if([&data](const IniUserData::IniSection& it) { return it.nickname == data.currentNickname; });
            }
            else if ((data.minStage != 0 && data.currentStage < data.minStage) || (data.maxStage != 0 && data.currentStage > data.maxStage))
            {
                // Not meeting the level requirement, ignore it for now
                data.sections.pop_back();
            }
            else
            {
                // Otherwise we look for existing sections to merge as needed
                for (auto existingSection = data.sections.begin(); existingSection != data.sections.end(); ++existingSection)
                {
                    if (&*existingSection == data.currentSection || _strcmpi(data.currentNickname.c_str(), existingSection->nickname.c_str()) != 0)
                    {
                        continue;
                    }

                    for (auto& [multiKey, values] : data.currentSection->multiKeys)
                    {
                        existingSection->multiKeys.emplace_back(multiKey, values);
                    }

                    for (auto& [key, value] : data.currentSection->singleKeys)
                    {
                        bool found = false;
                        for (const auto& existingKey : existingSection->singleKeys | std::views::keys)
                        {
                            if (_strcmpi(existingKey.c_str(), key.c_str()) == 0)
                            {
                                existingSection->singleKeys[existingKey] = value;
                                found = true;
                            }
                        }

                        if (!found)
                        {
                            existingSection->singleKeys[key] = value;
                        }
                    }

                    // Now remove this current section, as it has been merged
                    data.sections.pop_back();
                    data.currentSection = nullptr;
                    break;
                }
            }
        }

        // If we encounter the 'null' section, we return and stop
        if (_strcmpi(section, "null") == 0)
        {
            return 0;
        }

        data.currentSection = &data.sections.emplace_back();
        data.currentSection->name = section;
        data.minStage = 0;
        data.maxStage = 0;
        data.currentNickname = "";
        data.remove = false;
        return 1;
    }

    if (_strcmpi(key, "min_stage") == 0)
    {
        data.minStage = std::stoi(value);
    }
    else if (_strcmpi(key, "max_stage") == 0)
    {
        data.maxStage = std::stoi(value);
    }
    else if (_strcmpi(key, "remove") == 0)
    {
        data.remove = _strcmpi(value, "true") == 0 || _strcmpi(value, "1") == 0;
    }
    else
    {
        bool found = false;
        for (const auto& [mkSection, mkKey] : IniUserData::multiKeySections)
        {
            if (_strcmpi(mkSection.c_str(), section) == 0 && mkKey.contains(key))
            {
                found = true;
                data.currentSection->multiKeys.emplace_back(key, value);
                break;
            }
        }

        if (!found)
        {
            if (_strcmpi(key, "nickname") == 0)
            {
                data.currentNickname = value;
                data.currentSection->nickname = data.currentNickname;
            }

            data.currentSection->singleKeys[key] = value;
        }
    }

    return 1;
}

bool __thiscall Retold::IniReaderOpenDetour(INI_Reader* ini, const char* path, bool unk)
{
    // ReSharper disable once CppDFANullDereference
    const auto overrides = instance->systemFileOverrides.find(std::string(path));
    if (overrides == instance->systemFileOverrides.end())
    {
        instance->iniReaderOpenDetour.UnDetour();
        const auto res = instance->iniReaderOpenDetour.GetOriginalFunc()(ini, path, unk);
        instance->iniReaderOpenDetour.Detour(IniReaderOpenDetour);
        return res;
    }

    const auto sz = std::filesystem::file_size(path);
    std::string tempBuffer;
    tempBuffer.resize(sz);
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    stream.read(tempBuffer.data(), sz);
    stream.close();

    tempBuffer += overrides->second;
    tempBuffer += "\n[null]\n"; // A dummy section at the end to ensure we process the last section in the document

    IniUserData data;

    if (instance->contentStory)
    {
        data.currentStage = instance->contentStory->missionStage;
    }

    ini_parse_string(tempBuffer.c_str(), reinterpret_cast<ini_handler>(IniHandler), &data);
    data.sections.remove_if(
        [](const IniUserData::IniSection& it)
        {
            if (it.nickname != "empty")
            {
                return false;
            }

            for (const auto& header : IniUserData::sectionsWithoutNickname)
            {
                if (_strcmpi(header.c_str(), it.name.c_str()) == 0)
                {
                    return false;
                }
            }

            Fluf::Info("Removing section (missing nickname) " + it.nickname);
            return true;
        });

    static std::string systemBuffer;
    systemBuffer.clear();

    for (const auto& [name, nickname, singleKeys, multiKeys] : data.sections)
    {
        systemBuffer += std::format("[{}]\n", name);
        for (auto [k, v] : singleKeys)
        {
            systemBuffer += std::format("{} = {}\n", k, v);
        }

        for (auto [k, v] : multiKeys)
        {
            systemBuffer += std::format("{} = {}\n", k, v);
        }
    }

    std::ofstream temp{ "_test.ini", std::ios::trunc };
    temp << systemBuffer;
    temp.close();

    // We now process the file AGAIN, this time with INI reader to ensure all objects are initialised correctly
    // This does mean we end up reading the file 4 times in total, but should be no problem for modern hardware
    ini->open_memory(systemBuffer.c_str(), systemBuffer.size());
    using IniObjReader = bool (*)(INI_Reader*);
    static auto iniObjReaderFunc = reinterpret_cast<IniObjReader>(0x62B8DE0);
    iniObjReaderFunc(ini);
    ini->reset();

    ini->open_memory(systemBuffer.c_str(), systemBuffer.size());
    return true;
}

void Retold::ReadUniverseIni()
{
    INI_Reader ini;
    if (!ini.open("../DATA/UNIVERSE/universe.ini", false))
    {
        throw ModuleLoadException("universe.ini not found");
    }

    while (ini.read_header())
    {
        if (!ini.is_header("system"))
        {
            continue;
        }

        std::string systemFile{};
        std::list<std::string> fileOverrides;
        while (ini.read_value())
        {
            if (ini.is_value("file"))
            {
                systemFile = ini.get_value_string();
            }
            else if (ini.is_value("file_override"))
            {
                fileOverrides.emplace_back(ini.get_value_string());
            }
        }

        if (!systemFile.empty() && !fileOverrides.empty())
        {
            std::string overrideFile;

            for (auto& filePath : fileOverrides)
            {
                if (!std::filesystem::exists(filePath))
                {
                    filePath = std::string(R"(..\DATA\UNIVERSE\)") + filePath;
                }

                Fluf::Debug(std::format("Loading override file: {}", filePath));

                std::ifstream file(filePath, std::ios::in | std::ios::binary);
                if (!file)
                {
                    continue;
                }

                const auto sz = std::filesystem::file_size(filePath);
                std::string result(sz, '\0');
                file.read(result.data(), sz);
                file.close();

                overrideFile += result + "\n";
            }

            if (!overrideFile.empty())
            {
                systemFileOverrides[std::format(R"(..\data\universe\{})", systemFile)] = overrideFile;
            }
        }
    }
}

void Retold::ReadFreelancerIni()
{
    INI_Reader ini;
    ini.open("freelancer.ini", false);
    ini.find_header("DATA");

    std::list<std::string> allEquipment;
    std::list<std::string> allShips;
    std::string constants;
    while (ini.read_value())
    {
        if (ini.is_value("equipment"))
        {
            allEquipment.emplace_back(ini.get_value_string());
        }
        else if (ini.is_value("ships"))
        {
            allShips.emplace_back(ini.get_value_string());
        }
        else if (ini.is_value("constants"))
        {
            constants = ini.get_value_string();
        }
    }

    if (!constants.empty())
    {
        ReadConstantsIni(constants);
    }

    for (auto& equipment : allEquipment)
    {
        Fluf::Debug(std::format("Loading equipment file: {}", equipment));
        ReadEquipmentIni(equipment);
    }

    for (auto& ship : allShips)
    {
        Fluf::Debug(std::format("Loading ship file: {}", ship));
        ReadShipArchFile(ship);
    }
}

void Retold::ReadEquipmentIni(const std::string& file)
{
    INI_Reader ini;
    if (!ini.open(std::format("../DATA/{}", file).c_str(), false))
    {
        return;
    }

    while (ini.read_header())
    {
        if (ini.is_header("Gun"))
        {
            std::string nickname;
            ExtraWeaponData data{};
            bool found = false;
            while (ini.read_value())
            {
                if (ini.is_value("nickname"))
                {
                    nickname = ini.get_value_string();
                }

                data.shieldPowerUsage = ini.GetFloat("shield_power_usage", found);
            }

            if (!nickname.empty() && found)
            {
                Fluf::Debug(std::format("Adding custom equipment: {}", nickname));
                extraWeaponData[CreateID(nickname.c_str())] = data;
            }
        }
        else if (ini.is_header("Munition"))
        {
            std::string nickname;
            ExtraMunitionData data{};
            bool found = false;
            while (ini.read_value())
            {
                if (ini.is_value("nickname"))
                {
                    nickname = ini.get_value_string();
                }

                data.equipmentMultiplier = ini.GetFloat("equipment_multiplier", found);
                data.hullDot = ini.GetFloat("hull_dot", found);
                data.shieldRechargeReduction = ini.GetFloat("shield_recharge_reduction", found);
                data.hullVulnerability = ini.GetFloat("hull_vulnerability", found);
            }

            if (!nickname.empty() && found)
            {
                Fluf::Debug(std::format("Adding custom munition: {}", nickname));
                extraMunitionData[CreateID(nickname.c_str())] = data;
            }
        }
        else if (ini.is_header("ShieldGenerator"))
        {
            std::string nickname;
            ExtraShieldData data{};
            bool found = false;
            while (ini.read_value())
            {
                if (ini.is_value("nickname"))
                {
                    nickname = ini.get_value_string();
                }

                data.shieldStrength = ini.GetFloat("shield_strength", found);
                data.offlineRegenerationRate = ini.GetFloat("offline_regeneration_rate", found);
            }

            if (!nickname.empty() && found)
            {
                Fluf::Debug(std::format("Adding custom shield: {}", nickname));
                extraShieldData[CreateID(nickname.c_str())] = data;
            }
        }
    }
}

void Retold::ReadShipArchFile(const std::string& file)
{
    INI_Reader ini;
    if (!ini.open(std::format("../DATA/{}", file).c_str(), false))
    {
        return;
    }

    while (ini.read_header())
    {
        if (!ini.is_header("Ship"))
        {
            continue;
        }

        std::string nickname;
        ExtraShipData data{};
        while (ini.read_value())
        {
            if (ini.is_value("nickname"))
            {
                nickname = ini.get_value_string();
            }
            else if (ini.is_value("hull_dot_max"))
            {
                data.hullDotMax = ini.get_value_float(0);
            }
            else if (ini.is_value("hull_vulnerability_fuse"))
            {
                float threshold = ini.get_value_float(0);
                uint fuse = CreateID(ini.get_value_string(1));

                if (threshold > 0.f && fuse)
                {
                    data.hullVulnerabilityFuses.emplace_back(threshold, fuse);
                }
            }
            else if (ini.is_value("shield_offline_fuse"))
            {
                data.shieldOfflineFuse = CreateID(ini.get_value_string(0));
            }
        }

        // Sort so we are high thresholds first
        data.hullVulnerabilityFuses.sort([](const auto& a, const auto& b) { return b.first > a.second; });

        if (!nickname.empty() && (data.hullDotMax != 0.f || !data.hullVulnerabilityFuses.empty()))
        {
            Fluf::Debug(std::format("Adding custom ship: {}", nickname));
            extraShipData[CreateID(nickname.c_str())] = data;
        }
    }
}

void Retold::ReadConstantsIni(const std::string& file)
{
    INI_Reader ini;
    if (!ini.open(std::format("../DATA/{}", file).c_str(), false) || !ini.find_header("RetoldConsts"))
    {
        return;
    }

    while (ini.read_value())
    {
        if (ini.is_value("HULL_DOT_MAX"))
        {
            hullDotMax = ini.get_value_float(0);
        }
        else if (ini.is_value("HULL_DOT_DURATION"))
        {
            hullDotDuration = ini.get_value_float(0);
        }
        else if (ini.is_value("SHIELD_RECHARGE_REDUCTION_DURATION"))
        {
            shieldRechargeReductionDuration = ini.get_value_float(0);
        }
        else if (ini.is_value("SHIELD_RECHARGE_REDUCTION_MIN"))
        {
            shieldRechargeReductionMax = ini.get_value_float(0);
            if (shieldRechargeReductionMax >= 1.f)
            {
                shieldRechargeReductionMax = 0.f;
            }
        }
        else if (ini.is_value("HULL_VULNERABILITY_MAX"))
        {
            hullVulnerabilityMax = ini.get_value_float(0);
            if (hullVulnerabilityMax < 0.f)
            {
                hullVulnerabilityMax = 1.5f;
            }
        }
        else if (ini.is_value("HULL_VULNERABILITY_DURATION"))
        {
            hullVulnerabilityDuration = ini.get_value_float(0);
            if (hullVulnerabilityDuration < 0.f)
            {
                hullVulnerabilityDuration = 5.f;
            }
        }
        else if (ini.is_value("HULL_VULNERABILITY_FUSE"))
        {
            float threshold = ini.get_value_float(0);
            uint fuse = CreateID(ini.get_value_string(1));

            if (threshold > 0.f && fuse)
            {
                hullVulnerabilityFuses.emplace_back(threshold, fuse);
            }
        }
        else if (ini.is_value("NANOBOT_HEAL_PER_SECOND"))
        {
            nanobotHealingPerSecond = ini.get_value_float(0);
            if (nanobotHealingPerSecond < 0.f)
            {
                nanobotHealingPerSecond = 0.12f;
            }
        }
        else if (ini.is_value("NANOBOT_HEAL_DURATION"))
        {
            nanobotHealingDuration = ini.get_value_float(0);
            if (nanobotHealingDuration < 0.f)
            {
                nanobotHealingDuration = 10.f;
            }
        }
        else if (ini.is_value("BATTERY_HEAL_PER_SECOND"))
        {
            batteryHealingPerSecond = ini.get_value_float(0);
            if (batteryHealingPerSecond < 0.f)
            {
                batteryHealingPerSecond = 0.12f;
            }
        }
        else if (ini.is_value("BATTERY_HEAL_DURATION"))
        {
            batteryHealingDuration = ini.get_value_float(0);
            if (batteryHealingDuration < 0.f)
            {
                batteryHealingDuration = 0.10f;
            }
        }
    }

    // Sort so we are high thresholds first
    hullVulnerabilityFuses.sort([](const auto& a, const auto& b) { return b.first > a.second; });
}
