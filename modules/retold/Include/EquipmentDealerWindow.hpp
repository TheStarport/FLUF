#pragma once

#include <ImGui/FlWindow.hpp>

class EquipmentDealerWindow final : public FlWindow
{
        ImGuiInterface* imgui;

        void RenderWindowContents() override;

    public:
        void Render() override;
        explicit EquipmentDealerWindow(ImGuiInterface* imgui);
};
