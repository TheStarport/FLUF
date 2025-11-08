#include "PCH.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include "ObjectiveTracking.hpp"

#include <FLUF.UI.hpp>
#include <imgui_internal.h>
#include <ImGui/ImGuiInterface.hpp>

#include "ImGui/Fonts/Keycaps.hpp"

#include <ImGui/Fonts/IconFontAwesome6.hpp>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void ObjectiveTracking::Render()
{
    const auto viewport = ImGui::GetMainViewport();
    const ImVec2 mainWindowSize = viewport->Size;

    float yOffset = 0.f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    for (auto& objective : objectives)
    {
        constexpr float windowHeight = 10.f;
        constexpr float widthPadding = 20.f;
        ImGui::SetNextWindowPos(ImVec2((viewport->WorkPos.x + viewport->WorkSize.x) * 0.02f, (viewport->WorkPos.y + viewport->WorkSize.y) * 0.05f + yOffset),
                                ImGuiCond_Always);
        auto contentSize = ImGui::CalcTextSize((objective.icon + objective.message).c_str());
        ImGui::SetNextWindowSize({ contentSize.x + widthPadding + 3, windowHeight + contentSize.y });
        ImGui::Begin(std::format("##objective-{}", objective.message).c_str(),
                     nullptr,
                     ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        ImGui::PushTextWrapPos(mainWindowSize.x / 3.f); // We want to support multi-line text, this will wrap the text after 1/3 of the screen width

        ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();

        ImGui::BeginDisabled(objective.complete);
        ImGui::Text("%s   %s", objective.complete ? ICON_FA_CHECK : objective.icon.c_str(), objective.message.c_str());
        ImGui::EndDisabled();

        /*cursorScreenPos.y += ImGui::GetFontSize() * 0.5f;
        ImGui::GetWindowDrawList()->AddLine(
            { cursorScreenPos.x + 35.f, cursorScreenPos.y }, ImVec2(cursorScreenPos.x + contentSize.x + 3, cursorScreenPos.y), IM_COL32(255, 0, 0, 255), 3.0f);*/

        ImGui::PopTextWrapPos();
        constexpr float padding = 10.f;
        yOffset += padding + ImGui::GetWindowHeight();

        ImGui::End();
    }

    ImGui::PopStyleVar();
}

void ObjectiveTracking::OnGameLoad()
{
    auto flufUi = Fluf::GetModule(FlufUi::moduleName);
    if (flufUi.expired())
    {
        throw ModuleLoadException("Objective Tracking requires FLUF.UI to be enabled.");
    }

    auto ui = std::static_pointer_cast<FlufUi>(flufUi.lock());
    if (!ui || !ui->GetImGuiInterface())
    {
        throw ModuleLoadException("Objective Tracking requires Fluf UI to be in ImGui mode");
    }

    imgui = ui->GetImGuiInterface();
    imgui->RegisterImGuiModule(this);
}

ObjectiveTracking::ObjectiveTracking() { AssertRunningOnClient; }
ObjectiveTracking::~ObjectiveTracking() = default;

std::string_view ObjectiveTracking::GetModuleName() { return moduleName; }

SETUP_MODULE(ObjectiveTracking);
