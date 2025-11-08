#include <PCH.hpp>

#include "Internal/ImAnim/AbstractAnimation.hpp"

const imanim::EasingCurve& imanim::AbstractAnimation::GetEasingCurve() const { return easingCurve; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::AbstractAnimation::SetDuration(const float timeInSecs)
{
    // TODO: Handle if animation is already in progress
    durationInSecs = timeInSecs;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::AbstractAnimation::SetEasingCurve(const EasingCurve::Type easingCurveType, const double amplitude, const double period, const double overshoot)
{
    easingCurve.SetType(easingCurveType);
    easingCurve.SetAmplitude(amplitude);
    easingCurve.SetPeriod(period);
    easingCurve.SetOvershoot(overshoot);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::AbstractAnimation::SetLoopCount(const int count) { loopCount = count; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::AbstractAnimation::Start()
{
    if (loopCount == 0)
    {
        // No loop count, so ignore start
        return;
    }
    if (animationState == State::Running)
    {
        // Animation is already in progress
        return;
    }

    RunFunctionCallback(State::Running);
    animationState = State::Running;
    startTime = std::chrono::high_resolution_clock::now();
    currentLoop = 0;

    OnStartAnimation();
    UpdateValueForProgress(easingCurve.CalculateValueForProgress(0.0));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::AbstractAnimation::Stop()
{
    if (animationState != State::Stopped)
    {
        RunFunctionCallback(State::Stopped);
        animationState = State::Stopped;
        currentLoop = 0;
        OnStopAnimation();
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::AbstractAnimation::Update()
{
    if (animationState != State::Running)
    {
        return;
    }

    // Calculate the amount of time we've been fading
    auto duration = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime).count();

    if (duration >= durationInSecs)
    {
        // Last animation update
        UpdateValueForProgress(easingCurve.CalculateValueForProgress(1.0));
        currentLoop++;
        if (currentLoop >= loopCount && loopCount >= 0)
        {
            // This animation is finished
            RunFunctionCallback(State::Stopped);
            animationState = State::Stopped;
            currentLoop = 0;
            OnStopAnimation();
            return;
        }
        startTime = std::chrono::high_resolution_clock::now();
        OnStartAnimation();
    }
    else
    {
        // Calculate the current progress
        const double t = 1.0 - (durationInSecs - duration) / durationInSecs;
        UpdateValueForProgress(easingCurve.CalculateValueForProgress(t));
    }
}

void imanim::AbstractAnimation::SetAnimationStateChangedCallback(const OnAnimationStateChangedCallback& callback)
{
    onAnimationStateChangedCallback = callback;
}

void imanim::AbstractAnimation::RunFunctionCallback(State newState)
{
    if (onAnimationStateChangedCallback)
    {
        onAnimationStateChangedCallback(this, newState);
    }
}
