#pragma once

class AbstractVanillaInterface;
class HudManager
{
        std::unordered_set<AbstractVanillaInterface*> huds;

    public:
        bool RegisterHud(AbstractVanillaInterface* hud);
        bool EraseHud(AbstractVanillaInterface* hud);
};
