#pragma once

#include "ImGui/ImGuiMarkdown.hpp"

#include <FlightManualConfig.hpp>
#include <pugixml.hpp>

class FlightManualMarkdown final : public ImguiMarkdown
{
        int infoboxId = 0;
        float desiredEndHeight = 0.f;
        bool currentlyInInlineImage = false;

        std::function<void(std::string_view)> onPageSelected;
        rfl::Ref<FlightManualConfig> config;
        std::string* tooltipContent = nullptr;

        bool HandleKeyCode(std::string_view html);
        bool HandleToolTip(std::string_view html, bool isEndNode);
        bool HandleInfoBox(std::string_view html, bool isEndNode);
        bool CheckHtml(std::string_view html, std::string_view nodeName, bool isEndNode, bool isSelfClosing) override;

        void OpenUrl() const override;
        void RenderText(const char* str, const char* str_end) override;
        bool GetImage(image_info& nfo) override;
        void RenderImage(image_info& nfo) override;
        void DocumentEnd(const char* end) override;

    public:
        explicit FlightManualMarkdown(ImGuiInterface* imguiInterface, rfl::Ref<FlightManualConfig> config, std::function<void(std::string_view)> pageSelected);
};
