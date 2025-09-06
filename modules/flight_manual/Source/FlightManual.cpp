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

    config = rfl::make_ref<FlightManualConfig>(*ConfigHelper<FlightManualConfig, FlightManualConfig::path>::Load());

    flufUi = module;
    auto imgui = flufUi->GetImGuiInterface();
    imgui->RegisterImGuiModule(this);
    flightManualWindow = std::make_unique<FlightManualWindow>(imgui, config);
}

void FlightManual::Render() { flightManualWindow->Render(); }

FlightManual::FlightManual() = default;

FlightManual::~FlightManual()
{
    flufUi->GetImGuiInterface()->UnregisterImGuiModule(this);
    flufUi->GetHudManager().lock()->EraseHud(this);
}

std::string_view FlightManual::GetModuleName() { return moduleName; }

SETUP_MODULE(FlightManual);
