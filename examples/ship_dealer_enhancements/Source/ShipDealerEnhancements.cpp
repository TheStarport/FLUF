#include "PCH.hpp"

#include "ShipDealerEnhancements.hpp"
#include "ShipDealerInterface.hpp"

#include "FLCore/Common/CEquip/CAttachedEquip/CEShield.hpp"
#include "FLCore/Common/CObjects/CSimple/CEqObj/CShip.hpp"
#include "FLCore/Common/CommonMethods.hpp"

#include "FLUF/Include/Fluf.hpp"
#include "FLUF/Include/FlufModule.hpp"

#include "FLUF.UI/Include/FLUF.UI.hpp"
#include "FLUF.UI/Include/Rml/RmlContext.hpp"
#include "Utils/StringUtils.hpp"
#include "Vanilla/HudManager.hpp"

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void ShipDealerEnhancements::OnGameLoad()
{
    InitShipDealerHooks();

    INI_Reader ini;

    char szCurDir[MAX_PATH];
    GetCurrentDirectoryA(sizeof(szCurDir), szCurDir);
    const std::string currDir = std::string(szCurDir);
    const std::string freelancerIniFile = currDir + R"(\freelancer.ini)";

    std::string gameDir = currDir.substr(0, currDir.length() - 4);
    gameDir += std::string(R"(\DATA\)");

    if (!ini.open(freelancerIniFile.c_str(), false))
    {
        return;
    }

    std::vector<std::string> marketFiles;

    while (ini.read_header())
    {
        if (!ini.is_header("Data"))
        {
            continue;
        }
        while (ini.read_value())
        {
            if (ini.is_value("markets"))
            {
                marketFiles.emplace_back(gameDir + ini.get_value_string());
            }
        }
    }
    ini.close();

    auto& baseShipMap = ShipDealerInterface::GetBaseShipMap();
    for (auto& marketFile : marketFiles)
    {
        if (!ini.open(marketFile.c_str(), false))
        {
            continue;
        }

        while (ini.read_header())
        {
            if (!ini.is_header("BaseGood"))
            {
                continue;
            }

            uint currentBase;
            while (ini.read_value())
            {
                if (ini.is_value("base"))
                {
                    currentBase = CreateID(ini.get_value_string());
                }
                else if (ini.is_value("marketgood"))
                {
                    const auto goodInfo = GoodList_get()->find_by_id(CreateID(ini.get_value_string(0)));
                    if (!goodInfo)
                    {
                        Fluf::Log(LogLevel::Warn, std::format("Invalid good found in file {}, good Id: {}", marketFile, ini.get_value_string(0)));
                        continue;
                    }
                    if (goodInfo->type != GoodType::Ship)
                    {
                        continue;
                    }
                    baseShipMap[currentBase].push_back(CreateID(ini.get_value_string(0)));
                }
            }
        }

        ini.close();
    }

    shipDealerInterface = std::make_unique<ShipDealerInterface>();

    const auto lockHudMng = flufUi->GetHudManager().lock();
    lockHudMng->RegisterHud(shipDealerInterface.get());
}

ShipDealerEnhancements::ShipDealerEnhancements()
{
    const auto weakPtr = Fluf::GetModule(FlufUi::moduleName);
    if (weakPtr.expired())
    {
        Fluf::Log(LogLevel::Error, "Ship Dealer Enhancements was loaded, but FLUF UI was not loaded. Crashes are likely.");
        return;
    }

    const auto module = std::static_pointer_cast<FlufUi>(weakPtr.lock());
    flufUi = module;
}

std::string_view ShipDealerEnhancements::GetModuleName() { return moduleName; }

SETUP_MODULE(ShipDealerEnhancements);
