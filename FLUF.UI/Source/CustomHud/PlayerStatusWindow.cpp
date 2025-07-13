#include "PCH.hpp"

#include "Internal/CustomHud.hpp"
#include "Internal/PlayerStatusWindow.hpp"

#include "Fluf.hpp"
#include "imgui_internal.h"
#include "ImGui/ImGuiHelper.hpp"
#include <imgui_gradient/imgui_gradient.hpp>

PlayerStatusWindow::PlayerStatusWindow(const std::unordered_map<std::string, std::unordered_map<FlufModule*, OnRenderStatsMenu>>& statsMenus)
    : FlWindow("Player Status", ImGuiWindowFlags_NoResize), statsMenus(statsMenus)
{}

void PlayerStatusWindow::RegisterNewMenu(FlufModule* module, const RegisterMenuFunc func)
{
    Fluf::Log(LogLevel::Info, std::format("({}) Registering Status Menu", module->GetModuleName()));
    registeredMenus[module] = func;
}

void PlayerStatusWindow::RenderWindowContents()
{
    if (!ImGui::BeginTabBar("##ps-tabs"))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabItem("Faction Overview"))
    {
        // TODO: Implement me

        ImGG::GradientWidget gradient{};
        static ImGG::Settings settings = { .flags = ImGG::Flag::NoAddAndRemoveButtons | ImGG::Flag::NoColorEdit | ImGG::Flag::NoPositionSlider |
                                                    ImGG::Flag::NoMarkOptions | ImGG::Flag::NoResetButton | ImGG::Flag::NoDragDownToDelete,
                                           .mark_color = 0,
                                           .mark_hovered_color = 0,
                                           .mark_selected_color = 0,
                                           .mark_selected_hovered_color = 0 };
        gradient.gradient().clear();
        /*for (auto i = 0.05f; i <= 1.0f; i += 0.05f)
        {
            gradient.gradient().add_mark({ ImGG::RelativePosition(i), ImGG::ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f) });
        }*/
        gradient.gradient().add_mark({
            ImGG::RelativePosition{ 0.0f },
            ImGG::ColorRGBA{ 0.85f, 0.15f, 0.13f, 1.f }
        });
        gradient.gradient().add_mark({
            ImGG::RelativePosition{ 0.5f },
            ImGG::ColorRGBA{ 1.f, 1.f, 1.f, 1.f }
        });
        gradient.gradient().add_mark({
            ImGG::RelativePosition{ 0.500001f },
            ImGG::ColorRGBA{ 0.f, 0.f, 0.f, 0.f }
        });
        gradient.widget("Liberty Navy", settings);

        gradient.gradient().clear();
        gradient.gradient().add_mark({
            ImGG::RelativePosition{ 0.49999999f },
            ImGG::ColorRGBA{ 0.f, 0.f, 0.f, 0.f }
        });
        gradient.gradient().add_mark({
            ImGG::RelativePosition{ 0.5f },
            ImGG::ColorRGBA{ 1.f, 1.f, 1.f, 1.f }
        });
        gradient.gradient().add_mark({
            ImGG::RelativePosition{ 1.f },
            ImGG::ColorRGBA{ 0.27f, 0.83f, 0.09f, 1.f }
        });
        gradient.widget("Liberty Police", settings);

        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Player Statistics"))
    {
        if (ImGui::BeginTable("stats-table", 2))
        {
            for (auto& [category, menus] : statsMenus)
            {
                ImGui::TableNextColumn();
                ImGui::SeparatorText(category.c_str());

                if (category == "Exploration")
                {
                    ImGui::Text("Total Missions: PRETEND VALUE");
                    ImGui::Text("Total Kills: PRETEND VALUE");
                    ImGui::Text("Total Time: PRETEND VALUE");
                    ImGui::Text("Systems Visited: PRETEND VALUE");
                    ImGui::Text("Bases Visited: PRETEND VALUE");
                    ImGui::Text("Jump Holes Visited: PRETEND VALUE");
                }
                else if (category == "Kill Counts")
                {
                    ImGui::Text("Fighters Killed: PRETEND VALUE");
                    ImGui::Text("Freighters Killed: PRETEND VALUE");
                    ImGui::Text("Transports Killed: PRETEND VALUE");
                    ImGui::Text("Battleships Killed: PRETEND VALUE");
                }

                for (auto& [module, function] : menus)
                {
                    if (module && function)
                    {
                        (module->*function)();
                    }
                }
            }

            ImGui::EndTable();
        }

        // TODO: Implement me
        ImGui::EndTabItem();
    }

    int counter = 0;
    for (const auto& [module, function] : registeredMenus)
    {
        const auto nameView = module->GetModuleName();
        auto name = std::string(nameView) + "##";
        ImGui::PushID(counter++);
        if (ImGui::BeginTabItem(name.c_str()))
        {
            (module->*function)();
            ImGui::EndTabItem();
        }
        ImGui::PopID();
    }

    ImGui::EndTabBar();
}

void PlayerStatusWindow::SetOpen() { this->isOpen = true; }
