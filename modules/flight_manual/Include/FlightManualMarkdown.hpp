#pragma once

#include "FLUF.UI/Include/ImGui/ImGuiMarkdown.hpp"

class FlightManualMarkdown final : public ImguiMarkdown
{
        std::function<void(std::string_view)> onPageSelected;
        std::string currentKeyCode;

        bool CheckHtml(const char* str, const char* str_end) override;
        void HtmlDiv(const std::string& dclass, bool e) override;
        void OpenUrl() const override;

    public:
        explicit FlightManualMarkdown(ImGuiInterface* imguiInterface, std::function<void(std::string_view)>);
};
