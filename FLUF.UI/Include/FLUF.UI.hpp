#pragma once

#ifdef FLUF_UI
    #define API __declspec(dllexport)
#else
    #define API __declspec(dllimport)
#endif

class HudManager;
class FlufUi
{
        std::shared_ptr<HudManager> hudManager;

    public:
        API static std::weak_ptr<FlufUi> Instance();
        API std::weak_ptr<HudManager> GetHudManager();

        FlufUi();
        ~FlufUi();
};
