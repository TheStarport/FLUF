#include <PCH.hpp>

#include <EquipmentDealerWindow.hpp>
#include <imgui_internal.h>
#include <ImGui/ImGuiHelper.hpp>
#include <ImGui/ImGuiInterface.hpp>

void EquipmentDealerWindow::RenderWindowContents()
{
    ImGui::NewLine();

    if (!ImGui::BeginTable("##edw-table", 4, ImGuiTableFlags_SizingFixedFit))
    {
        return;
    }

    auto fl = DWORD(GetModuleHandleA(nullptr));
    EquipDescList* playerEquipDesc = reinterpret_cast<EquipDescList*>(fl + 0x272960);

    ImGui::TableSetupColumn("Ship Preview w/ Stats", ImGuiTableColumnFlags_WidthFixed, 400.f);
    ImGui::TableSetupColumn("Category Icons", ImGuiTableColumnFlags_WidthFixed, 100.f);
    ImGui::TableSetupColumn("Items", ImGuiTableColumnFlags_WidthFixed, 300.f);
    ImGui::TableSetupColumn("Item Info", ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableNextColumn();
    // Ship Preview

    ImGui::TableNextColumn();
    // Icons

    auto iconSize = ImVec2{ 50.f, 50.f };
    uint width = 0, height = 0;

    auto icon = imgui->LoadTexture(".png", width, height);
    ImGui::ImageButton("Weapon Icon", icon, iconSize);

    icon = imgui->LoadTexture("line-graph.webp", width, height);
    ImGui::ImageButton("Ammo Icon", icon, iconSize);

    icon = imgui->LoadTexture("_external_icon.png", width, height);
    ImGui::ImageButton("External Icon", icon, iconSize);

    icon = imgui->LoadTexture("_internal_icon.png", width, height);
    ImGui::ImageButton("Internal Icon", icon, iconSize);

    ImGui::TableNextColumn();
    // Items

    if (ImGui::BeginTable("##edw-items-table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        auto mousePos = ImGui::GetMousePos();

        for (int i = 0; i < 10; i++)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            icon = imgui->LoadTexture("_missile_icon.png", width, height);
            ImGui::Image(icon, { 100.f, 100.f });

            ImGui::TableNextColumn();
            ImGui::Text("Le epic missile launcher");
            ImGui::Text("Class 1");
            ImGui::SameLine();
            std::string_view cost = "$123";
            ImGuiHelper::RightAlignedText(cost.data(), ImGui::GetColumnWidth());

            const auto table = ImGui::GetCurrentTable();
            const ImGuiTableColumn* column1 = &table->Columns[0];
            const ImGuiTableColumn* column2 = &table->Columns[1];

            if (mousePos.x > column1->ClipRect.Min.x && mousePos.x < column2->ClipRect.Max.x && mousePos.y > table->RowPosY1 && mousePos.y < table->RowPosY2)
            {
                ImGui::BeginTooltip();
                ImGui::Text("Tooltip on row %u", i);
                ImGui::EndTooltip();
            }
        }

        ImGui::EndTable();
    }

    /*for (auto& hp : playerEquipDesc->equip)
    {
        ImGui::Text(hp.)
        //
    }*/

    ImGui::TableNextColumn();
    // Item Info

    ImGui::EndTable();
}

void EquipmentDealerWindow::Render()
{
    SetPositionRelative({ 0.4, 0.2 }); // DEBUG
    //SetPositionRelative({ 0.1f, 0.1f });
    SetSizeWithAspectRatio(1800.0f, ImVec2(2.f, 1.f));

    FlWindow::Render();
}

EquipmentDealerWindow::EquipmentDealerWindow(ImGuiInterface* imgui)
    : FlWindow("Equipment Dealer", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove), imgui(imgui)
{
    SetOpenState(true); // DEBUG
}
