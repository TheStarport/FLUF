#include <utility>

#include "PCH.hpp"

#include "ImGui/FlWindow.hpp"

#include "FLUF.UI.hpp"
#include "Fluf.hpp"
#include "d3d9.h"
#include "ImGui/ImGuiInterface.hpp"
#include <imgui_internal.h>

void FlWindow::DrawScrollbars()
{
    if (!drawScrollbars)
    {
        return;
    }

    static auto getBoundingBox = [](ImGuiWindow* window, const ImGuiAxis axis)
    {
        const auto scrollBarSize = ImGui::GetStyle().ScrollbarSize;
        // Calculate scrollbar bounding box
        const ImRect outerRect = window->Rect();
        const ImRect innerRect = window->InnerRect;
        const float borderSize = window->WindowBorderSize;
        if (axis == ImGuiAxis_X)
        {
            return ImRect(innerRect.Min.x,
                          ImMax(outerRect.Min.y, outerRect.Max.y - borderSize - scrollBarSize),
                          innerRect.Max.x - borderSize,
                          outerRect.Max.y - borderSize);
        }

        return ImRect(
            ImMax(outerRect.Min.x, outerRect.Max.x - borderSize - scrollBarSize), innerRect.Min.y, outerRect.Max.x - borderSize, innerRect.Max.y - borderSize);
    };

    const auto drawScrollbar = [](ImGuiWindow* window, const ImGuiAxis axis)
    {
        const ImGuiID id = ImGui::GetWindowScrollbarID(window, axis);
        auto barBox = getBoundingBox(window, axis);

        if (axis == ImGuiAxis_Y)
        {
            barBox.Min.x -= 20.f;
            barBox.Max.x -= 20.f;
            barBox.Min.y += 20.f;
            barBox.Max.y -= 20.f;
        }
        else
        {
            barBox.Min.x += 20.f;
            barBox.Max.x -= 20.f;
            barBox.Min.y -= 20.f;
            barBox.Max.y -= 20.f;
        }

        ImDrawFlags rounding = ImDrawFlags_RoundCornersNone;

        if (window->Flags & ImGuiWindowFlags_NoTitleBar && !(window->Flags & ImGuiWindowFlags_MenuBar))
        {
            rounding |= ImDrawFlags_RoundCornersTopRight;
        }

        if (!window->ScrollbarX)
        {
            rounding |= ImDrawFlags_RoundCornersBottomRight;
        }

        const float sizeVisible = window->InnerRect.Max[axis] - window->InnerRect.Min[axis];
        const float sizeContents = window->ContentSize[axis] + window->WindowPadding[axis] * 2.0f;
        auto scroll = static_cast<ImS64>(window->Scroll[axis]);
        ImGui::ScrollbarEx(barBox, id, axis, &scroll, (ImS64)sizeVisible, (ImS64)sizeContents, rounding);
        window->Scroll[axis] = static_cast<float>(scroll);
    };

    const auto scrollBarSize = ImGui::GetStyle().ScrollbarSize;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    static ImVec2 scrollBarSizesLastFrame{};

    bool useCurrentSizeForX = window->Appearing;
    bool useCurrentSizeForY = window->Appearing;
    bool windowSizeXSetByApi = false;
    bool windowSizeYSetByApi = false;

    const ImGuiContext* g = ImGui::GetCurrentContext();
    windowSizeXSetByApi = (window->SetWindowSizeAllowFlags & g->NextWindowData.SizeCond) != 0 && g->NextWindowData.SizeVal.x > 0.0f;
    windowSizeYSetByApi = (window->SetWindowSizeAllowFlags & g->NextWindowData.SizeCond) != 0 && g->NextWindowData.SizeVal.y > 0.0f;

    if (windowSizeXSetByApi && window->ContentSizeExplicit.x != 0.0f)
    {
        useCurrentSizeForX = true;
    }
    if (windowSizeYSetByApi && window->ContentSizeExplicit.y != 0.0f) // #7252
    {
        useCurrentSizeForY = true;
    }

    const ImVec2 currentFrameAvail = ImVec2(window->SizeFull.x, window->SizeFull.y - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2));
    const ImVec2 lastFrameAvail = window->InnerRect.GetSize() + scrollBarSizesLastFrame;
    const ImVec2 neededSizeFromLastFrame = window->Appearing ? ImVec2(0, 0) : window->ContentSize + window->WindowPadding * 2.0f;
    const float scrollBarSizeX = useCurrentSizeForX ? currentFrameAvail.x : lastFrameAvail.x;
    const float scrollBarSizeY = useCurrentSizeForY ? currentFrameAvail.y : lastFrameAvail.y;

    const auto scrollbarY = neededSizeFromLastFrame.y > scrollBarSizeY;
    const auto scrollbarX = neededSizeFromLastFrame.x > scrollBarSizeX - (scrollbarY ? scrollBarSize : 0.0f);
    if (window->ScrollbarX && !window->ScrollbarY)
    {
        window->ScrollbarY = neededSizeFromLastFrame.y > scrollBarSizeY - scrollBarSize;
    }

    if (scrollbarY)
    {
        drawScrollbar(window, ImGuiAxis_Y);
    }

    if (scrollbarX)
    {
        drawScrollbar(window, ImGuiAxis_X);
    }

    scrollBarSizesLastFrame = ImVec2(scrollbarY ? scrollBarSize : 0.0f, scrollbarX ? scrollBarSize : 0.0f);
}

void FlWindow::DrawWindowDecorations(const ImVec2 startingPos, const ImVec2 windowSize)
{
    static std::array<std::pair<float, float>, 12> windowRatios = {
        {
         { 0.03125f, 0.f },
         { 0.1640625f, 0.f },
         { 0.1875f, 0.00900f },
         { 0.2421875f, 0.00900f },
         { 0.2421875f, 0.02734375f },
         { 0.02734375f, 0.02734375f },
         { 0.02734375f, 0.09375f },
         { 0.01953125f, 0.12890625f },
         { 0.01953125f, 0.18359375f },
         { 0.f, 0.21875f },
         { 0.f, 0.0234375f },
         { 0.03125f, 0.f },
         },
    };

    static auto drawCorner = [](ImVec2 pos, const ImVec2 size, const bool invertX, const bool invertY)
    {
        pos.x += invertX ? 20.f : -20.f;
        pos.y += invertY ? 20.f : -20.f;

        ImDrawList* drawList = ImGui::GetForegroundDrawList();
        std::array<ImVec2, 12> points;
        for (auto i = 0; i < 12; i++)
        {
            const auto& ratio = windowRatios[i];
            const float x = (invertX ? -size.x : size.x) * ratio.first;
            const float y = (invertY ? -size.y : size.y) * ratio.second;
            points[i] = ImVec2(pos.x + x, pos.y + y);
        }

        drawList->AddConvexPolyFilled(points.data(), points.size(), 0xFF371A00);
        for (auto& p : points)
        {
            drawList->PathLineTo(p);
        }

        drawList->PathStroke(0xFFAF9019, 0, 5.f);
    };

    drawCorner(startingPos, windowSize, false, false);
    drawCorner(ImVec2(startingPos.x + windowSize.x, startingPos.y), windowSize, true, false);
    drawCorner(ImVec2(startingPos.x, startingPos.y + windowSize.y), windowSize, false, true);
    drawCorner(startingPos + windowSize, windowSize, true, true);
}

void FlWindow::SetTitle(const std::string& title) { this->title = title; }

void FlWindow::CenterWindow(const bool center) { centered = center; }

void FlWindow::SetSize(const ImVec2 size) { this->size = size; }
void FlWindow::SetPosition(const ImVec2 position) { this->position = position; }
void FlWindow::SetPivot(const ImVec2 pivot) { this->pivot = pivot; }

void FlWindow::Render()
{
    if (!isOpen)
    {
        return;
    }

    if (centered)
    {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, conditionFlag, ImVec2(0.5f, 0.5f));
    }
    else
    {
        ImGui::SetNextWindowPos(position, conditionFlag, pivot);
    }

    ImGui::SetNextWindowSize(size, conditionFlag);
    ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 35.f);
    ImGui::PushStyleVarY(ImGuiStyleVar_WindowPadding, 35.f);
    ImGui::Begin(title.c_str(), &isOpen, windowFlags | ImGuiWindowFlags_NoTitleBar);

    const auto windowPos = ImGui::GetWindowPos();
    const auto windowSize = ImGui::GetWindowSize();

    if (backgroundTexture)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddCallback(
            [](const ImDrawList*, const ImDrawCmd*)
            {
                if (renderingBackend == RenderingBackend::Dx9)
                {
                    auto* dx = static_cast<IDirect3DDevice9*>(dxDevice);
                    dx->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
                    dx->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
                }
            },
            nullptr);
        drawList->AddImage(backgroundTexture, windowPos, windowPos + windowSize, ImVec2(0.f, 0.f), windowSize / imageSize, 0x80FFFFFF);
        drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
    }

    RenderWindowContents();

    /*ImDrawList* drawList = ImGui::GetForegroundDrawList();

    windowPos.x -= 20;
    windowPos.y -= 30;

    // clang-format off
    const std::array points {
        ImVec2(windowPos.x + 40, windowPos.y),
        ImVec2(windowPos.x + 210, windowPos.y),
        ImVec2(windowPos.x + 240, windowPos.y + 15),
        ImVec2(windowPos.x + 310, windowPos.y + 15),
        ImVec2(windowPos.x + 310, windowPos.y + 35),
        ImVec2(windowPos.x + 35, windowPos.y + 35),
        ImVec2(windowPos.x + 35, windowPos.y + 120),
        ImVec2(windowPos.x + 25, windowPos.y + 165),
        ImVec2(windowPos.x + 25, windowPos.y + 235),
        ImVec2(windowPos.x, windowPos.y + 280),
        ImVec2(windowPos.x, windowPos.y + 30),
        ImVec2(windowPos.x + 40, windowPos.y),
    };
    // clang-format on

    drawList->AddConvexPolyFilled(points.data(), points.size(), 0xFF371A00);
    for (auto& p : points)
    {
        drawList->PathLineTo(p);
    }

    drawList->PathStroke(0xFFAF9019, 0, 5.f);*/
    DrawWindowDecorations(windowPos, windowSize);
    DrawScrollbars();

    ImGui::End();
    ImGui::PopStyleVar(2);
}

FlWindow::FlWindow(std::string windowName, const ImGuiWindowFlags flags, const ImGuiCond condition)
    : title(std::move(windowName)), windowFlags(flags), conditionFlag(condition)
{
    drawScrollbars = (flags & ImGuiWindowFlags_NoScrollbar) == 0;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;

    if (backgroundTexture)
    {
        return;
    }

    const auto flufUi = std::static_pointer_cast<FlufUi>(Fluf::GetModule(FlufUi::moduleName).lock());
    renderingBackend = flufUi->GetRenderingBackend();

    const auto imguiInterface = flufUi->GetImGuiInterface();
    dxDevice = imguiInterface->GetRenderingContext();

    // TODO: Make this texture customizable
    uint width = 0, height = 0;
    backgroundTexture = reinterpret_cast<ImTextureID>(imguiInterface->LoadTexture("backgroundpattern.bmp", width, height));
    imageSize = { static_cast<float>(width), static_cast<float>(height) };
}
