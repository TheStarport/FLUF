#pragma once

class AbstractVanillaInterface
{
    protected:
    virtual void OnDraw() = 0;
    virtual void OnUpdate() = 0;
    virtual bool OnExecute() = 0;
    virtual void OnExecuteAfter() = 0;
    virtual void OnTerminate() = 0;
    virtual ~AbstractVanillaInterface() = default;
};

extern "C" bool RegisterHud(AbstractVanillaInterface* hud);
extern "C" bool EraseHud(AbstractVanillaInterface* hud);