#include <PCH.hpp>

#include "Internal/ImAnim/SequentialAnimationGroup.hpp"

float imanim::SequentialAnimationGroup::GetDuration() const
{
    float durationInSeconds = 0.0F;

    for (const auto& anim : animationList)
    {
        const int loopCount = anim->GetLoopCount();
        const float currentDurationInSeconds = anim->GetDuration();
        if ((loopCount < 0) || (currentDurationInSeconds < 0.0F))
        {
            // The animation has an infinite time, so set the duration to -1
            // and break loop
            durationInSeconds = -1.0F;
            break;
        }

        durationInSeconds += (static_cast<float>(loopCount) * currentDurationInSeconds);
    }

    return durationInSeconds;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::SequentialAnimationGroup::OnStartAnimation()
{
    if (animationList.empty())
    {
        // No animations so stop
        RunFunctionCallback(State::Stopped);
        animationState = State::Stopped;
        currentLoop = 0;
        return;
    }

    indexOfCurrentAnimation = 0;
    currentAnimation = animationList[0];
    currentAnimation->Start();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::SequentialAnimationGroup::OnStopAnimation()
{
    if (currentAnimation != nullptr)
    {
        currentAnimation->Stop();
    }
    indexOfCurrentAnimation = 0;
    currentAnimation = nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::SequentialAnimationGroup::Update()
{
    if ((animationState != State::Running) || (currentAnimation == nullptr))
    {
        return;
    }

    // Update the current animation
    currentAnimation->Update();

    if (currentAnimation->GetState() == State::Stopped)
    {
        // Move to the next animation in the sequence
        indexOfCurrentAnimation++;
        if (indexOfCurrentAnimation >= animationList.size())
        {
            // Increment the loop
            currentLoop++;
            indexOfCurrentAnimation = 0;
            if ((currentLoop >= loopCount) && (loopCount >= 0))
            {
                // This animation is finished
                RunFunctionCallback(State::Stopped);
                animationState = State::Stopped;
                currentLoop = 0;
                currentAnimation = nullptr;
                return;
            }
        }
        currentAnimation = animationList[indexOfCurrentAnimation];
        currentAnimation->Start();
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::SequentialAnimationGroup::UpdateValueForProgress([[maybe_unused]] double dProgress) {}
