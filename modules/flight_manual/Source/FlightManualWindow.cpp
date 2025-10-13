#include <PCH.hpp>

#include "FlightManualWindow.hpp"

#include "Fluf.hpp"
#include "ImGui/ImGuiInterface.hpp"
#include "Breadcrumb.hpp"
#include "Utils/StringUtils.hpp"
#include "ImGui/IconFontAwesome6.hpp"

#include <imgui_internal.h>

bool SquareButton(std::string_view text, const ImTextureID textureId, const ImVec2& buttonSize, const ImVec4& backgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f))
{
    const ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
    {
        return false;
    }

    auto cursorPos = ImGui::GetCursorPos();
    auto pos = window->DC.CursorPos;

    const auto buttonRegion = ImRect(pos, pos + buttonSize);
    bool hovered, held;
    const bool clicked =
        ImGui::ButtonBehavior(buttonRegion, ImGui::GetID(std::format("{}##button", text).c_str()), &hovered, &held, ImGuiButtonFlags_PressedOnRelease);

    auto drawList = ImGui::GetWindowDrawList();
    static std::array<ImVec2, 8> ratios = {
        { { 0.0f, 0.0f },
         { 1.f, 0.0f },
         { 1.f, 0.75f },
         { 0.85f, 1.f },
         { 0.0f, 1.0f },
         { 0.f, 0.f },
         { 0.f, 1.f }, //
          { 0.f, 0.f } }
    };

    std::array<ImVec2, 8> points;
    for (int i = 0; i < points.size(); ++i)
    {
        const auto& ratio = ratios[i];
        points[i] = ImVec2(buttonSize.x * ratio.x + pos.x, buttonSize.y * ratio.y + pos.y);
    }

    if (backgroundColor.w != 0.0f)
    {
        drawList->AddConcavePolyFilled(points.data(), points.size(), ImGui::ColorConvertFloat4ToU32(backgroundColor));
    }

    for (auto& point : points)
    {
        drawList->PathLineTo(point);
    }

    drawList->PathStroke(0xB00899FC, 0, 2.5f);

    if (textureId)
    {
        float imageTint;
        if (held || hovered)
        {
            imageTint = 1.0f;
        }
        else
        {
            imageTint = 0.5f;
        }

        ImGui::ImageWithBg(textureId, ImVec2(buttonSize.x * 0.95f, buttonSize.y), {}, { 1.f, 1.f }, {}, { imageTint, imageTint, imageTint, 1.f });
    }

    auto textPos = pos;
    const auto font = ImGui::GetFont();
    const auto fontSize = ImGui::CalcTextSize(text.data());
    textPos.x += buttonSize.x * 0.75f - fontSize.x + 20.f;
    textPos.y += buttonSize.y * 0.5f - fontSize.y * 0.5f;
    drawList->AddText(font, fontSize.y, textPos, 0xFFFFFFFF, text.data());

    ImGui::SetCursorPos(ImVec2(cursorPos.x, cursorPos.y + buttonSize.y + g.Style.FramePadding.y));
    ImGui::Dummy({});

    return clicked;
}

void FlightManualWindow::RenderWindowContents()
{
    breadcrumb->Render();

    ImGui::Spacing();

    std::string_view closeButton = ICON_FA_XMARK "##conf-close";
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(closeButton.data()).x);
    if (ImGui::Button(closeButton.data()))
    {
        SetOpenState(false);
        return;
    }

    ImGui::Separator();
    ImGui::Spacing();

    if (!ImGui::BeginTable("##fmw-table", 2))
    {
        return;
    }

    const auto maxWidth = ImGui::GetContentRegionAvail().x;
    ImGui::TableSetupColumn("pages", ImGuiTableColumnFlags_WidthFixed, maxWidth * 0.21f);
    ImGui::TableSetupColumn("content", ImGuiTableColumnFlags_WidthFixed, maxWidth * 0.79f);

    ImGui::TableNextColumn();

    static std::string dataPath = "../data";

    if (!currentPage)
    {
        for (auto* page : topLevelPages)
        {
            uint width = 0, height = 0;
            if (SquareButton(page->title, imguiInterface->LoadTexture(dataPath + page->icon, width, height), ImVec2(225.f, 100.f)))
            {
                PageClicked(page);
            }
        }
    }
    else
    {
        for (auto* page : currentPage->children.value())
        {
            uint width = 0, height = 0;
            if (SquareButton(page->title, imguiInterface->LoadTexture(dataPath + page->icon, width, height), ImVec2(225.f, 100.f)))
            {
                PageClicked(page);
            }
        }
    }

    ImGui::TableNextColumn();

    if (!currentPage)
    {
        markdown->Render(rootPageContent);
    }
    else
    {
        markdown->Render(currentPage->content);
    }

    ImGui::EndTable();
}

void FlightManualWindow::PageClicked(FlightManualPage* page)
{
    currentPage = page;
    breadcrumb->AddItem(page->title);
}

void FlightManualWindow::Render()
{
    SetSizeWithAspectRatio(1440, ImVec2(4.f, 3.f));
    CenterWindow();
    FlWindow::Render();
}

void FlightManualWindow::ChangePage(const std::string_view fullPath)
{
    breadcrumb->Reset();
    if (fullPath.empty() || fullPath == "/")
    {
        currentPage = nullptr;
        return;
    }

    auto path = StringUtils::GetParams(fullPath, '/');
    const auto expectedParents = static_cast<uint>(std::ranges::distance(path));

    bool foundDest = false;

    const std::vector<FlightManualPage*>* children = &topLevelPages;
    for (int i = 0; i < expectedParents; i++)
    {
        auto component = path.begin();
        std::advance(component, i);

        for (auto* child : *children)
        {
            if (!StringUtils::CompareCaseInsensitive(std::string_view(child->title), *component))
            {
                continue;
            }

            if (i + 1 == expectedParents)
            {

                foundDest = true;
                PageClicked(child);
                break;
            }

            children = &child->children.value();
            PageClicked(child);
            break;
        }
    }

    if (!foundDest)
    {
        Fluf::Error(std::format("Bad link located within flight manual: {}", fullPath));
        currentPage = nullptr;
    }
}

FlightManualWindow::FlightManualWindow(ImGuiInterface* imguiInterface, rfl::Ref<FlightManualConfig> config)
    : FlWindow("Flight Manual", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)
{
    SetOpenState(true);
    FlightManualWindow::imguiInterface = imguiInterface;
    breadcrumb = std::make_unique<Breadcrumb>(this, static_cast<OnBreadcrumbItemClicked>(&FlightManualWindow::ChangePage), "Flight Manual");
    rootPageContent = config->rootPageContent;

    markdown = std::make_unique<FlightManualMarkdown>(imguiInterface, [this](const std::string_view path) { ChangePage(path); });

    std::vector<FlightManualPage*> pages;
    for (auto& page : config->pages)
    {
        if (page.path.empty() || !page.path.starts_with("/"))
        {
            Fluf::Error(std::format("Page found without a valid path: {}", page.title));
            continue;
        }

        // Remove trailing and leading slashes
        if (page.path.back() == '/' && page.path.size() > 1)
        {
            page.path.pop_back();
        }

        page.path = page.path.substr(1);

        pages.emplace_back(&page);
    }

    // Sort based on the amount of '/' they have
    std::ranges::sort(pages,
                      [](const FlightManualPage* a, const FlightManualPage* b)
                      {
                          int aCount = 0, bCount = 0;
                          for (const auto c : a->path)
                          {
                              if (c == '/')
                              {
                                  aCount++;
                              }
                          }

                          for (const auto c : b->path)
                          {
                              if (c == '/')
                              {
                                  bCount++;
                              }
                          }

                          if (aCount > bCount)
                          {
                              return false;
                          }

                          if (bCount > aCount)
                          {
                              return true;
                          }

                          const auto orderL = a->order.value_or(0);
                          const auto orderR = b->order.value_or(0);
                          if (orderL < orderR)
                          {
                              return false;
                          }

                          if (orderL > orderR)
                          {
                              return true;
                          }

                          // Same count, strcmp
                          return strcmp(a->path.c_str(), b->path.c_str()) < 0;
                      });

    // Iterate through all pages, ensuring each one is parented
    for (auto* page : pages)
    {
        if (page->path.empty())
        {
            topLevelPages.emplace_back(page);
            continue;
        }

        auto path = StringUtils::GetParams(page->path, '/');
        auto expectedParents = static_cast<uint>(std::ranges::distance(path));

        std::vector<FlightManualPage*>* children = &topLevelPages;
        for (int i = 0; i < expectedParents; i++)
        {
            FlightManualPage* parent = nullptr;
            auto component = path.begin();
            std::advance(component, i);

            for (auto* child : *children)
            {
                if (child->title != *component)
                {
                    continue;
                }

                if (i + 1 == expectedParents)
                {
                    // Add and reset
                    child->children.value().emplace_back(page);
                    children = &topLevelPages;
                    parent = child;
                    break;
                }

                parent = child;
                children = &child->children.value();
                break;
            }

            if (i + 1 == expectedParents && !parent)
            {
                Fluf::Error(std::format("Page parent not found! Please check the path for this page: {}", page->title));
            }
        }
    }
}
