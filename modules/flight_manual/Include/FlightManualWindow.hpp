#pragma once
#include "Breadcrumb.hpp"
#include "FlightManual.hpp"
#include "FlightManualMarkdown.hpp"
#include "ImGui/FlWindow.hpp"

#include <FlightManualConfig.hpp>

class FlightManualWindow final : public FlWindow
{
        inline static ImGuiInterface* imguiInterface;
        std::unique_ptr<Breadcrumb> breadcrumb;
        std::vector<FlightManualPage*> topLevelPages;

        std::string_view rootPageContent;
        FlightManualPage* currentPage = nullptr;
        std::unique_ptr<FlightManualMarkdown> markdown;

        void RenderWindowContents() override;
        void PageClicked(FlightManualPage* page);

    public:
        explicit FlightManualWindow(ImGuiInterface* imguiInterface, rfl::Ref<FlightManualConfig> config);
        void Render() override;
        void ChangePage(std::string_view fullPath);
        virtual ~FlightManualWindow() = default;
};
