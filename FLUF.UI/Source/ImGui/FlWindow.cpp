#include "PCH.hpp"

#include "ImGui/FlWindow.hpp"

#include "FLUF.UI.hpp"
#include "Fluf.hpp"
#include "d3d9.h"
#include "ImGui/ImGuiInterface.hpp"

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

    ImGui::SetNextWindowSize(size, conditionFlag);

    if (centered)
    {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, conditionFlag, ImVec2(0.5f, 0.5f));
    }
    else
    {
        ImGui::SetNextWindowPos(position, conditionFlag, pivot);
    }

    ImGui::Begin(title.c_str(), &isOpen, windowFlags);

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
        drawList->AddImage(backgroundTexture,
                           ImGui::GetWindowPos(),
                           ImGui::GetWindowPos() + ImGui::GetWindowSize(),
                           ImVec2(0.f, 0.f),
                           ImGui::GetWindowSize() / imageSize,
                           0x80FFFFFF);
        drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
    }

    RenderWindowContents();

    ImGui::End();
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
