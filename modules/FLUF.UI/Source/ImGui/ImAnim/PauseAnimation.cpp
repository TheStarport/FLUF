#include <PCH.hpp>

#include "Internal/ImAnim/PauseAnimation.hpp"

imanim::PauseAnimation::PauseAnimation(const float fDurationInSecs)
{
    SetDuration(fDurationInSecs);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::PauseAnimation::UpdateValueForProgress(double dProgress)
{
    // Do nothing
}
