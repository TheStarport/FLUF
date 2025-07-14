#include "PCH.hpp"

#include "Internal/CustomHud.hpp"
#include "Internal/PlayerStatusWindow.hpp"

#include "Fluf.hpp"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "ImGui/IconFontAwesome6.hpp"
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

void PlayerStatusWindow::RenderFactionOverview()
{
    if (!currentlySelectedFaction.empty())
    {
        RenderIndividualFaction();
        return;
    }

    ImGui::Dummy(ImVec2(0.0f, 60.0f));

    auto pos = ImGui::GetCursorScreenPos();
    const auto startingPos = pos;
    const auto drawList = ImGui::GetWindowDrawList();
    constexpr auto barSize = ImVec2(550.f, 100.f);

    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 0.f);

    constexpr std::array factions = { "Liberty Navy",     "Rheinland Military", "Kusari Naval Forces",     "Nomads", "Bretonia Mining and Metals",
                                      "Gateway Shipping", "Blood Dragons",      "Independent Miners Guild" };

    for (const auto text : factions)
    {
        constexpr auto windowWidth = barSize.x;
        const auto textSize = ImGui::CalcTextSize(text);

        ImRect region{ pos, pos + barSize };
        drawList->_FringeScale = 3.0f;
        drawList->AddRect(region.Min, region.Max, 0xFFAF9019, ImDrawFlags_None, 3.0f);
        drawList->_FringeScale = 1.0f;

        ImGui::BeginGroup();
        constexpr auto headerSpace = ImVec2{ 0.f, barSize.y * 0.03f };
        ImGui::Dummy(headerSpace);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (windowWidth - textSize.x) * 0.5f);
        ImGui::Text(text);
        ImGui::Dummy(ImVec2(0.0f, barSize.y - (headerSpace.y + textSize.y)));

        // Draw the boxes
        for (uint box = 1; box <= 20; ++box)
        {
            auto boxOffset = ImVec2{ barSize.x * 0.1f, barSize.y * 0.60f };
            drawList->AddRect(pos + boxOffset, pos + boxOffset + ImVec2(barSize.x * (0.04f * static_cast<float>(box)), barSize.y * 0.3f), UINT_MAX);
        }

        ImGui::EndGroup();
        if (ImGui::IsItemClicked())
        {
            currentlySelectedFaction = text;
        }

        pos = ImGui::GetCursorScreenPos();
    }

    ImGui::PopStyleVar();
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 20.f);

    pos = startingPos + ImVec2(100.f + barSize.x, 0.f);
    const auto filterBoxSize = ImVec2(530.f, 800.f);
    ImGui::SetCursorScreenPos(ImVec2(pos.x + 20.f, pos.y));
    ImGui::BeginChild("Filters", filterBoxSize);

    const auto filterTitle = "Filters";
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (filterBoxSize.x - ImGui::CalcTextSize(filterTitle).x) * 0.5f);
    ImGui::Text(filterTitle);

    ImGui::Text("By Text:");
    ImGui::InputText("##fo-by-text", &currentFilter);

    ImGui::Text("By Alignment:");
    ImGui::Checkbox("Lawful", &filterByLawful);
    ImGui::SameLine(ImGui::GetCursorPosX() + filterBoxSize.x * 0.4f);
    ImGui::Checkbox("Unlawful", &filterByUnlawful);

    ImGui::Text("By Relationship:");
    ImGui::Combo("##fo-by-relationship", &currentFactionRelationshipIndex, factionRelationships.data(), factionRelationships.size());

    ImGui::Text("Sort by:");
    ImGui::Combo("##fo-sort-by", &currentSortByIndex, sortByList.data(), sortByList.size());

    ImGui::PopStyleVar();

    // Add border to the child
    drawList->_FringeScale = 3.0f;
    drawList->AddRect(pos, pos + filterBoxSize + ImVec2(20.f, 0.f), 0xFFAF9019, ImDrawFlags_None, 3.0f);
    drawList->_FringeScale = 1.0f;

    ImGui::EndChild();
}

void PlayerStatusWindow::RenderIndividualFaction()
{
    if (ImGui::Button(ICON_FA_ARROW_LEFT "##fo-go-back"))
    {
        currentlySelectedFaction = "";
        return;
    }

    ImGui::Text(currentlySelectedFaction.c_str());
}

void PlayerStatusWindow::RenderPlayerStats() const
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
}

void PlayerStatusWindow::RenderWindowContents()
{
    if (!ImGui::BeginTabBar("##ps-tabs"))
    {
        return;
    }

    if (ImGui::BeginTabItem("Faction Overview"))
    {
        RenderFactionOverview();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Player Statistics"))
    {
        RenderPlayerStats();
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
