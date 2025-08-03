#include "PCH.hpp"

#include "Internal/CustomHud.hpp"
#include "Internal/CustomOptionsWindow.hpp"

#include "Fluf.hpp"
#include "Random.hpp"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "ImGui/IconFontAwesome6.hpp"
#include "ImGui/ImGuiHelper.hpp"
#include "Utils/StringUtils.hpp"

CustomOptionsWindow::CustomOptionsWindow(std::unordered_map<FlufModule*, RegisterOptionsFunc>& menus)
    : FlWindow("Player Status", ImGuiWindowFlags_NoResize), registeredMenus(menus)
{
    CenterWindow();
}

void CustomOptionsWindow::RenderWindowContents()
{
    static std::string_view lastSavedModule;

    if (!ImGui::BeginTabBar("##option-tabs"))
    {
        return;
    }

    const auto itemSpacing = ImGui::GetStyle().ItemSpacing;
    int counter = 0;
    for (const auto& [module, menuFunc] : registeredMenus)
    {
        const auto nameView = module->GetModuleName();
        auto name = std::string(nameView) + "##";
        ImGui::PushID(counter++);
        if (!ImGui::BeginTabItem(name.c_str()))
        {
            continue;
        }

        constexpr auto saveChangesText = "Save Changes";
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - (ImGui::CalcTextSize(saveChangesText).x + itemSpacing.x * 2.0f));
        const auto saveRequested = ImGui::Button(saveChangesText);

        (module->*menuFunc)(saveRequested);

        if (saveRequested)
        {
            Fluf::Log(LogLevel::Info, "Saved changes");
            lastSavedModule = nameView;
            ImGui::OpenPopup("Changes Saved!");
        }

        if (ImGui::BeginPopupModal("Changes Saved!", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            std::string text = std::format("Successfully saved settings for {}", lastSavedModule);
            ImGui::Text(text.c_str());
            if (ImGui::Button("OK"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::EndTabItem();
    }

    ImGui::PopID();
    ImGui::EndTabBar();
}

void CustomOptionsWindow::SetOpen() { this->isOpen = true; }
