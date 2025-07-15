#pragma once

#include "ImGui/ImGuiInterface.hpp"
#include "ImGui/FlWindow.hpp"

class CustomHud;
class PlayerStatusWindow final : public FlWindow
{
        friend CustomHud;

        std::unordered_map<FlufModule*, RegisterMenuFunc> registeredMenus;
        const std::unordered_map<std::string, std::unordered_map<FlufModule*, OnRenderStatsMenu>>& statsMenus;
        std::string currentFilter{};
        bool filterByLawful = true;
        bool filterByUnlawful = true;
        // clang-format off
        std::array<const char*, 6> factionRelationships{
            "Any",
            "Hostile",    // < -0.55f
            "Unfriendly", // > -0.55f < -0.1f
            "No Opinion", // > -0.1 < 0.1
            "Friendly", // > 0.1 < 0.55
            "Allied" // > 0.55
        };

        std::array<const char*, 3> sortByList{
            "By Rep",
            "By A-Z",
            "By Z-A",
        };
        // clang-format on
        int currentSortByIndex = 0;
        int currentFactionRelationshipIndex = 0;
        std::string currentlySelectedFaction;
        std::optional<RenderDisplayList> renderDisplayList;

        void RenderFactionOverview();
        void RenderIndividualFaction();
        void RenderPlayerStats() const;
        void RenderWindowContents() override;

    public:
        void SetOpen();
        explicit PlayerStatusWindow(const std::unordered_map<std::string, std::unordered_map<FlufModule*, OnRenderStatsMenu>>&);
        void RegisterNewMenu(FlufModule* module, RegisterMenuFunc func);
};
