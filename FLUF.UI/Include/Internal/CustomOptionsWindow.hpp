#pragma once

#include "ImGui/ImGuiInterface.hpp"
#include "ImGui/FlWindow.hpp"

class CustomOptionsWindow final : public FlWindow
{
        std::unordered_map<FlufModule*, RegisterOptionsFunc>& registeredMenus;
        void RenderWindowContents() override;

    public:
        void SetOpen();
        explicit CustomOptionsWindow(std::unordered_map<FlufModule*, RegisterOptionsFunc>& menus);
};
