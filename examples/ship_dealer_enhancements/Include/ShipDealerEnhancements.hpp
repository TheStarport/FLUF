#pragma once

#include "FLUF/Include/FlufModule.hpp"
#include "ShipDealerInterface.hpp"

class FlufUi;

class ShipDealerEnhancements final : public FlufModule
{
        std::shared_ptr<FlufUi> flufUi;

        std::unique_ptr<ShipDealerInterface> shipDealerInterface;

        void OnGameLoad() override;
        static void InitShipDealerHooks();

    public:
        static constexpr std::string_view moduleName = "ship_dealer_enhancements";

        ShipDealerEnhancements();
        std::string_view GetModuleName() override;
};
