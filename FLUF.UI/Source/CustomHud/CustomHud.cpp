#include "PCH.hpp"

#include "FLUF.UI/Include/Vanilla/Components/FlButtonControl.hpp"
#include "Internal/PlayerStatusWindow.hpp"
#include "Internal/CustomHud.hpp"

CustomHud::CustomHud(PlayerStatusWindow* statusWindow) : statusWindow(statusWindow) {}

bool CustomHud::OnExecuteButton(FlButtonControl* control)
{
    if (control->GetName() == "NNPlayerInfoButtonRoot")
    {
        // TODO: Add flashy animation!
        statusWindow->windowOpen = true;
        return false;
    }

    return true;
}
