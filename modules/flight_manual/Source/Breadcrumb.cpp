#include <PCH.hpp>

#include "Breadcrumb.hpp"

#include <imgui.h>
#include <imgui_internal.h>

bool Breadcrumb::ArrowButton(ImVec2 pos, const ImVec2 size, const char* id, const std::string_view text, bool lastItem)
{
    const auto drawList = ImGui::GetWindowDrawList();
    pos += ImGui::GetWindowPos();

    const auto buttonRegion = ImRect(pos, pos + size);
    bool hovered, held;
    const bool clicked = ImGui::ButtonBehavior(buttonRegion, ImGui::GetID(id), &hovered, &held, ImGuiButtonFlags_PressedOnRelease);

    static std::array<ImVec2, 7> ratios = {
        {
         { 0.0f, 0.0f },
         { 0.8f, .0f },
         { 1.f, 0.5f },
         { 0.8f, 1.f },
         { 0.0f, 1.0f },
         { 0.2f, 0.5f },
         { 0.0f, 0.0f },
         }
    };

    std::array<ImVec2, 7> points;
    for (int i = 0; i < points.size(); ++i)
    {
        const auto& ratio = ratios[i];
        points[i] = ImVec2(size.x * ratio.x + pos.x, size.y * ratio.y + pos.y);
    }

    ImU32 color = lastItem ? 0x800899FC : 0x80000000;
    drawList->AddConcavePolyFilled(points.data(), points.size(), color);
    for (auto& point : points)
    {
        drawList->PathLineTo(point);
    }

    drawList->PathStroke(0xB00899FC, 0, 2.5f);

    if (!text.empty())
    {
        const ImVec2 textPos = pos + size * 0.5f - ImGui::CalcTextSize(text.data()) * 0.5f;
        drawList->AddText(textPos, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)), text.data(), text.data() + text.length());
    }

    return clicked;
}

void Breadcrumb::Reset()
{
    items.clear();
    AddItem("Flight Manual");
}

Breadcrumb::Breadcrumb(FlWindow* module, const OnBreadcrumbItemClicked onClickCallback, std::string_view root)
    : module(module), onBreadcrumbItemClicked(onClickCallback)
{
    AddItem(root);
}

void Breadcrumb::AddItem(std::string_view text) { items.emplace_back(std::string(text)); }

void Breadcrumb::Render()
{
    if (items.empty())
    {
        return;
    }

    auto pos = ImGui::GetCursorPos();

    uint indexClicked = items.size();
    std::string itemClicked;
    int count = 0;
    for (const auto& item : items)
    {
        const auto width = std::clamp(ImGui::CalcTextSize(item.c_str()).x + 100.f, 125.f, 9999.f);
        if (ArrowButton(pos, ImVec2(width, 30.f), std::format("{}-arrow{}", reinterpret_cast<intptr_t>(module), count).c_str(), item, ++count == items.size()))
        {
            itemClicked = count == 1 ? "" : item;
            indexClicked = count;
        }

        pos.x += width - 5.f;
    }

    ImGui::SetCursorPosY(pos.y + 50.f + ImGui::GetStyle().FramePadding.y);

    if (indexClicked != items.size())
    {
        std::string path;

        if (itemClicked.empty())
        {
            (module->*onBreadcrumbItemClicked)(path);
            return;
        }

        for (auto item : items | std::ranges::views::drop(1))
        {
            if (!path.empty())
            {
                path += "/";
            }

            path += item;
            if (itemClicked == item)
            {
                (module->*onBreadcrumbItemClicked)(path);
                break;
            }
        }
    }
}
