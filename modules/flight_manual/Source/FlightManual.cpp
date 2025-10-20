#include "FlightManual.hpp"

#include "Exceptions.hpp"
#include "PCH.hpp"

#include "FLUF.UI.hpp"
#include "FlightManualWindow.hpp"
#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "imgui_markdown.h"

#include "FLCore/Common/CommonMethods.hpp"
#include "ImGui/ImGuiInterface.hpp"
#include "Utils/StringUtils.hpp"
#include "Vanilla/HudManager.hpp"
#include "FlightManualConfig.hpp"

#include <curl/curl.h>
#include <shellapi.h>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void FlightManual::OnGameLoad()
{
    std::array<char, MAX_PATH> totalPath{};
    GetUserDataPath(totalPath.data());

    const auto weakPtr = Fluf::GetModule(FlufUi::moduleName);
    if (weakPtr.expired())
    {
        throw ModuleLoadException("FLUF UI not loaded");
    }

    const auto module = std::static_pointer_cast<FlufUi>(weakPtr.lock());
    if (module->GetConfig()->uiMode != UiMode::ImGui)
    {
        throw ModuleLoadException("FLUF UI mode not set to ImGui");
    }

    config = rfl::make_ref<FlightManualConfig>(*ConfigHelper<FlightManualConfig>::Load(FlightManualConfig::path));

    if (std::filesystem::exists("modules/config/flight_manual/tooltips.yml")) {}

    using namespace std::string_view_literals;
    constexpr std::string_view pathyPath = "modules/config/flight_manual/";
    const std::string tooltipPath = std::string(pathyPath) + "tooltips.yml";
    if (!std::filesystem::exists(pathyPath))
    {
        std::filesystem::create_directories(pathyPath);
    }

    if (!std::filesystem::exists(tooltipPath))
    {
        std::ofstream out(tooltipPath);
        out << R"(test_tip: |
    A tool tip that can be displayed using <tooltip id="test_tip">content you want to hover over</tooltip>)";
        out.close();
    }

    auto tooltips = ConfigHelper<std::unordered_map<std::string, std::string>>::Load(tooltipPath);
    if (!tooltips)
    {
        Fluf::Warn("Could not load tooltips.yml for flight manual. Ensure file exists and is valid tooltip YAML.");
    }
    else
    {
        config->tooltips.value() = *tooltips;
    }

    for (auto& file : std::filesystem::recursive_directory_iterator(pathyPath))
    {
        if (!file.is_regular_file() || file.path().extension() != ".yml" || file.path().filename() == "tooltips.yml")
        {
            continue;
        }

        auto tempConfig = *ConfigHelper<std::vector<FlightManualPage>>::Load(file.path().string());
        config->pages.insert(std::end(config->pages), std::begin(tempConfig), std::end(tempConfig));
    }

    flufUi = module;
    auto imgui = flufUi->GetImGuiInterface();
    imgui->RegisterImGuiModule(this);
    flightManualWindow = std::make_unique<FlightManualWindow>(imgui, config);
}

void FlightManual::Render() { flightManualWindow->Render(); }

FlightManual::FlightManual() = default;

FlightManual::~FlightManual() { flufUi->GetImGuiInterface()->UnregisterImGuiModule(this); }

std::string_view FlightManual::GetModuleName() { return moduleName; }

SETUP_MODULE(FlightManual);
