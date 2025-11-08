#include <PCH.hpp>

#include "Internal/ImAnim/ParallelAnimationGroup.hpp"

float imanim::ParallelAnimationGroup::GetDuration() const
{
    float durationInSecs = 0.0F;

    // Traverse our animations and find the one with the maximum duration
    for (const auto& anim : animationList)
    {
        const int loopCount = anim->GetLoopCount();
        const float currDuration = anim->GetDuration();
        if (loopCount < 0 || currDuration < 0.0F)
        {
            // The animation has an infinite time, so set the duration to -1
            // and break loop
            durationInSecs = -1.0F;
            break;
        }

        if (const float totalDurationInSecs = static_cast<float>(loopCount) * currDuration; totalDurationInSecs > durationInSecs)
        {
            // Save this duration as it is the current max duration
            durationInSecs = totalDurationInSecs;
        }
    }

    return durationInSecs;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ParallelAnimationGroup::OnStartAnimation()
{
    if (animationList.empty())
    {
        // No animations so stop
        RunFunctionCallback(State::Stopped);
        animationState = State::Stopped;
        currentLoop = 0;
        return;
    }

    // Start all our animations
    for (const auto& anim : animationList)
    {
        anim->Start();
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ParallelAnimationGroup::OnStopAnimation()
{
    // Make sure that all our animations are stopped
    for (const auto& anim : animationList)
    {
        anim->Stop();
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ParallelAnimationGroup::Update()
{
    if (animationState != State::Running)
    {
        return;
    }

    // Update all our animations
    bool bIsRunning = false;
    for (const auto& anim : animationList)
    {
        anim->Update();
        if (anim->GetState() == State::Running)
        {
            bIsRunning = true;
        }
    }

    if (!bIsRunning)
    {
        // Increment the loop
        currentLoop++;
        if (currentLoop >= loopCount && loopCount >= 0)
        {
            // This animation is finished
            RunFunctionCallback(State::Stopped);
            animationState = State::Stopped;
            currentLoop = 0;
            return;
        }

        // Start all our animations since starting a new loop
        for (const auto& anim : animationList)
        {
            anim->Start();
        }
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ParallelAnimationGroup::UpdateValueForProgress([[maybe_unused]] double dProgress) {}
