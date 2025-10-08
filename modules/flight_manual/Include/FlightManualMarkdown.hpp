#pragma once

#include "FLUF.UI/Include/ImGui/ImGuiMarkdown.hpp"

class FlightManualMarkdown final : public ImguiMarkdown
{
        std::function<void(std::string_view)> onPageSelected;

        bool CheckHtml(const char* str, const char* str_end) override;
        void OpenUrl() const override;

    public:
        explicit FlightManualMarkdown(ImGuiInterface* imguiInterface, std::function<void(std::string_view)>);
};
