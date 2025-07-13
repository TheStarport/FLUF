#include "PCH.hpp"

#include "ImGui/FlWindow.hpp"

#include "FLUF.UI.hpp"
#include "Fluf.hpp"
#include "d3d9.h"
#include "ImGui/ImGuiInterface.hpp"

void FlWindow::DrawWindowDecorations(ImVec2 startingPos, ImVec2 windowSize)
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

    static auto drawCorner = [](ImVec2 pos, ImVec2 size, bool invertX, bool invertY)
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
        //return;
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
    ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 30.f);
    ImGui::PushStyleVarY(ImGuiStyleVar_WindowPadding, 30.f);
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

    ImGui::End();
    ImGui::PopStyleVar(2);
}

FlWindow::FlWindow(const std::string& windowName, const ImGuiWindowFlags flags, const ImGuiCond condition)
    : title(windowName), windowFlags(flags), conditionFlag(condition)
{
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
