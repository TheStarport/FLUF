#include <PCH.hpp>

#include <Utils/MemUtils.hpp>
#include <ini.h>
#include <Fluf.hpp>

#include "Retold.hpp"

// ReSharper disable once CppDFAConstantFunctionResult
static int IniHandler(std::string& iniBuffer, const char* section, const char* key, const char* value)
{
    if (!key && !value)
    {
        iniBuffer += std::format("[{}]\n", section);
        return 1;
    }

    if (_strcmpi(section, "object") == 0 && _strcmpi(key, "nickname") == 0)
    {
        Reputation::Vibe::EnsureExists(static_cast<int>(CreateID(value)));
    }

    iniBuffer += std::format("{} = {}\n", key, value);

    return 1;
}

bool __thiscall Retold::IniReaderOpenDetour(INI_Reader* ini, const char* path, bool unk)
{
    const std::string tempPath = path;
    // ReSharper disable once CppDFANullDereference
    auto overrides = instance->systemFileOverrides.find(tempPath);
    if (overrides == instance->systemFileOverrides.end())
    {
        instance->iniReaderOpenDetour.UnDetour();
        auto res = instance->iniReaderOpenDetour.GetOriginalFunc()(ini, path, unk);
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

    static std::string systemBuffer;
    systemBuffer.clear();

    ini_parse_string(tempBuffer.c_str(), reinterpret_cast<ini_handler>(IniHandler), &systemBuffer);

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
            while (ini.read_value())
            {
                if (ini.is_value("nickname"))
                {
                    nickname = ini.get_value_string();
                }
                else if (ini.is_value("shield_power_usage"))
                {
                    data.shieldPowerUsage = ini.get_value_float(0);
                }
            }

            if (!nickname.empty() && data.shieldPowerUsage != 0.f)
            {
                Fluf::Debug(std::format("Adding custom equipment: {}", nickname));
                extraWeaponData[CreateID(nickname.c_str())] = data;
            }
        }
        else if (ini.is_header("Munition"))
        {
            std::string nickname;
            ExtraMunitionData data{};
            while (ini.read_value())
            {
                if (ini.is_value("nickname"))
                {
                    nickname = ini.get_value_string();
                }
                else if (ini.is_value("equipment_multiplier"))
                {
                    data.equipmentMultiplier = ini.get_value_float(0);
                }
                else if (ini.is_value("hull_dot"))
                {
                    data.hullDot = ini.get_value_float(0);
                }
                else if (ini.is_value("shield_recharge_reduction"))
                {
                    data.shieldRechargeReduction = ini.get_value_float(0);
                }
                else if (ini.is_value("hull_vulnerability"))
                {
                    data.hullVulnerability = ini.get_value_float(0);
                }
            }

            if (!nickname.empty() &&
                (data.equipmentMultiplier != 0.f || data.hullDot != 0.f || data.shieldRechargeReduction != 0.f || data.hullVulnerability != 0.f))
            {
                Fluf::Debug(std::format("Adding custom munition: {}", nickname));
                extraMunitionData[CreateID(nickname.c_str())] = data;
            }
        }
        else if (ini.is_header("ShieldGenerator"))
        {
            std::string nickname;
            ExtraShieldData data{};

            while (ini.read_value())
            {
                if (ini.is_value("nickname"))
                {
                    nickname = ini.get_value_string();
                }
                else if (ini.is_value("shield_strength"))
                {
                    data.shieldStrength = ini.get_value_float(0);
                }
                else if (ini.is_value("offline_regeneration_rate"))
                {
                    data.offlineRegenerationRate = ini.get_value_float(0);
                }
            }

            if (!nickname.empty() && (data.shieldStrength != 0.f || data.offlineRegenerationRate != 0.f))
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
    }

    // Sort so we are high thresholds first
    hullVulnerabilityFuses.sort([](const auto& a, const auto& b) { return b.first > a.second; });
}
