#pragma once
#include "Vanilla/AbstractVanillaInterface.hpp"

bool HasMoreThanThreeShips(uint& pageCount);
void SetNextShips();
void SetPrevShips();

class ShipDealerInterface final : public AbstractVanillaInterface
{
        inline static bool initializedControls;
        inline static uint currentShipPage = 0;
        inline static uint shipPageCount = 0;

        FlTextControl* textControl;
        FlButtonControl* shipDealerNextControl;
        FlButtonControl* shipDealerPrevControl;

        FlControl* navBarControl;

        inline static std::unordered_map<uint, std::vector<uint>> orderedBaseShipMap;

        void OnUpdate(FlControl* control) override;
        bool OnExecuteButton(FlButtonControl* control) override;
        void OnTerminate(FlControl* control) override;

        [[nodiscard]] FlControl* NavBarFindShipDealer() const;

    public:
        ShipDealerInterface();
        static std::unordered_map<uint, std::vector<uint>>& GetBaseShipMap();
        static void SetCurrentShipPage(uint);
        static uint GetCurrentShipPage();
};
