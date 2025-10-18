#include <PCH.hpp>

#include <Utils/MemUtils.hpp>
#include <ini.h>
#include <Fluf.hpp>

#include "Retold.hpp"

// ReSharper disable once CppDFAConstantFunctionResult
static int IniHandler(std::string& iniBuffer, const char* section, const char* name, const char* value)
{
    if (!name && !value)
    {
        iniBuffer += std::format("[{}]\n", section);
        return 1;
    }

    iniBuffer += std::format("{} = {}\n", name, value);

    return 1;
}

const char* systemIniBuffer;
DWORD systemReturnAddress;
DWORD Retold::OnSystemIniOpen(INI_Reader& iniReader, const char* file, bool unk)
{
    if (!iniReader.open(file, unk))
    {
        return 0;
    }

    iniReader.close();

    static std::string systemBuffer;
    systemBuffer.clear();

    for (auto& overrideFile : systemFileOverrides)
    {
        if (_strcmpi(file, overrideFile.first.c_str()) == 0)
        {
            systemBuffer += overrideFile.second;
            break;
        }
    }

    Fluf::Info(file);
    ini_parse(file, reinterpret_cast<ini_handler>(IniHandler), &systemBuffer);

    systemIniBuffer = systemBuffer.c_str();
    return systemBuffer.size();
}

void __declspec(naked) Retold::SystemIniOpenNaked()
{
    constexpr static DWORD openMemory = 0x0630FC50;
    constexpr static DWORD open = 0x630F9B0;
    __asm
    {
        mov eax, [esp]
        mov systemReturnAddress, eax
        push ecx // Store for restoration
        push [esp+12] // bool unk
        push [esp+12] // const char* path
        push ecx // INI_Reader
        mov ecx, Retold::instance
        call OnSystemIniOpen // should pop 12
        test eax, eax
        jz normal_operation

        pop ecx
        add esp, 12 // Remove the previous two parameters + return address
        push eax
        push systemIniBuffer
        push systemReturnAddress
        jmp openMemory

        normal_operation:
            pop ecx
            jmp open
    }
}

void Retold::SetupHooks()
{
    const auto fl = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    const auto common = reinterpret_cast<DWORD>(GetModuleHandleA("common.dll"));

    // Hook System INI file Reading
    static auto systemIniOpenRedirectionAddress = &SystemIniOpenNaked;
    static auto systemIniOpenRedirectionAddress2 = &systemIniOpenRedirectionAddress;
    MemUtils::WriteProcMem(fl + 0x15379D, &systemIniOpenRedirectionAddress2, sizeof(systemIniOpenRedirectionAddress2));

    RetoldHooks::gunCanFireDetour.Detour(GunCanFireDetour);
    RetoldHooks::consumeFireResourcesDetour.Detour(LauncherConsumeFireResourcesDetour);
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
    while (ini.read_value())
    {
        if (ini.is_value("equipment"))
        {
            allEquipment.emplace_back(ini.get_value_string());
        }
    }

    for (auto& equipment : allEquipment)
    {
        Fluf::Debug(std::format("Loading equipment file: {}", equipment));
        ReadEquipmentIni(equipment);
    }
}

void Retold::ReadEquipmentIni(std::string file)
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
    }
}
