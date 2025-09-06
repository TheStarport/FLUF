#pragma once

#include "FLUF.UI/Include/ImGui/ImGuiMarkdown.hpp"

class FlightManualMarkdown final : public ImguiMarkdown
{
        std::function<void(std::string_view)> onPageSelected;

    public:
        explicit FlightManualMarkdown(ImGuiInterface* imguiInterface, std::function<void(std::string_view)>);
        void OpenUrl() const override;
};
