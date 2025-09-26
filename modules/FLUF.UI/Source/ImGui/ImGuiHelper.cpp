#include "PCH.hpp"

#include "ImGui/ImGuiHelper.hpp"

void ImGuiHelper::HelpMarker(const char* desc, const char character, const ImU32 color)
{
    if (color)
    {
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, color);
    }

    ImGui::TextDisabled("(%c)", character);
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }

    if (color)
    {
        ImGui::PopStyleColor();
    }
}

void ImGuiHelper::CenterNextWindow(const ImVec2 size)
{
    ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
}

void ImGuiHelper::CenteredText(const char* text, std::optional<float> widthOverride)
{
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (widthOverride.value_or(ImGui::GetWindowWidth()) - ImGui::CalcTextSize(text).x) * 0.5f);
    ImGui::Text(text);
}
