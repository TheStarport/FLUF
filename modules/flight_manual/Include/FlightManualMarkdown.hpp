#pragma once

#include "ImGui/ImGuiMarkdown.hpp"

#include <FlightManualConfig.hpp>
#include <pugixml.hpp>

class FlightManualMarkdown final : public ImguiMarkdown
{
        std::function<void(std::string_view)> onPageSelected;
        rfl::Ref<FlightManualConfig> config;
        std::string* tooltipContent = nullptr;

        bool HandleKeyCode(std::string_view html);
        bool HandleToolTip(std::string_view html, std::string_view nodeName, bool isEndNode);
        bool CheckHtml(std::string_view html, std::string_view nodeName, bool isEndNode, bool isSelfClosing) override;

        void OpenUrl() const override;
        bool GetImage(image_info& nfo) const override;
        void DocumentEnd(const char* end) override;

    public:
        explicit FlightManualMarkdown(ImGuiInterface* imguiInterface, rfl::Ref<FlightManualConfig> config, std::function<void(std::string_view)> pageSelected);
};
