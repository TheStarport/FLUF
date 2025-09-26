#pragma once

#include "../Vanilla/AbstractVanillaInterface.hpp"

class PlayerStatusWindow;
class CustomHud final : public AbstractVanillaInterface
{
        PlayerStatusWindow* statusWindow;

        bool OnExecuteButton(FlButtonControl* control) override;

    public:
        explicit CustomHud(PlayerStatusWindow*);
};
