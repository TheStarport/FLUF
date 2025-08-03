#pragma once

#include "FlufModule.hpp"
#include "ShipDealerInterface.hpp"

class FlufUi;

class ShipDealerEnhancements final : public FlufModule
{
        std::shared_ptr<FlufUi> flufUi;

        std::unique_ptr<ShipDealerInterface> shipDealerInterface;

        void OnGameLoad() override;
        static void InitShipDealerHooks();

    public:
        static constexpr std::string_view moduleName = "Ship Dealer Enhancements";

        ShipDealerEnhancements();
        std::string_view GetModuleName() override;
};
