#pragma once
#include "FLUF.UI.hpp"

class FlufUi;
class FlScrollControl;
class FlCycleControl;
class FlTextControl;
class FlButtonControl;
class FlControl;
class AbstractVanillaInterface;
class HudManager
{
        friend FlufUi;
        inline static std::unordered_set<AbstractVanillaInterface*> huds;

        static void OnDraw();
        static void __stdcall OnDrawInner(FlControl* control);
        static void OnUpdate();
        static void __stdcall OnUpdateInner(FlControl* control);
        static void OnExecuteButton();
        static bool __stdcall OnExecuteButtonInner(FlButtonControl* control);
        static void OnExecuteText();
        static bool __stdcall OnExecuteTextInner(FlTextControl* control);
        static void OnExecuteScroll();
        static void __stdcall OnExecuteScrollInner(FlScrollControl* control);
        static void OnExecuteCycle();
        static void __stdcall OnExecuteCycleInner(FlCycleControl* control);
        static void OnTerminate();
        static void __stdcall OnTerminateInner(FlControl* control);

    public:
        HudManager();
        ~HudManager();
        API bool RegisterHud(AbstractVanillaInterface* hud);
        API bool EraseHud(AbstractVanillaInterface* hud);
};
