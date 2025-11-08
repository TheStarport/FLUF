#include <PCH.hpp>

#include "Internal/ImAnim/EasingCurve.hpp"
#include "Internal/ImAnim/Easing.hpp"
#include <algorithm>

imanim::EasingCurve::EasingCurve(const Type eType) : easeType(eType) {}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::EasingCurve::CalculateValueForProgress(double progress) const
{
    double val = progress;
    progress = std::clamp(progress, 0.0, 1.0);

    switch (easeType)
    {
        case Type::Linear: val = Easing::EaseNone(progress); break;
        case Type::InQuad: val = Easing::EaseInQuad(progress); break;
        case Type::OutQuad: val = Easing::EaseOutQuad(progress); break;
        case Type::InOutQuad: val = Easing::EaseInOutQuad(progress); break;
        case Type::OutInQuad: val = Easing::EaseOutInQuad(progress); break;
        case Type::InCubic: val = Easing::EaseInCubic(progress); break;
        case Type::OutCubic: val = Easing::EaseOutCubic(progress); break;
        case Type::InOutCubic: val = Easing::EaseInOutCubic(progress); break;
        case Type::OutInCubic: val = Easing::EaseOutInCubic(progress); break;
        case Type::InQuart: val = Easing::EaseInQuart(progress); break;
        case Type::OutQuart: val = Easing::EaseOutQuart(progress); break;
        case Type::InOutQuart: val = Easing::EaseInOutQuart(progress); break;
        case Type::OutInQuart: val = Easing::EaseOutInQuart(progress); break;
        case Type::InQuint: val = Easing::EaseInQuint(progress); break;
        case Type::OutQuint: val = Easing::EaseOutQuint(progress); break;
        case Type::InOutQuint: val = Easing::EaseInOutQuint(progress); break;
        case Type::OutInQuint: val = Easing::EaseOutInQuint(progress); break;
        case Type::InSine: val = Easing::EaseInSine(progress); break;
        case Type::OutSine: val = Easing::EaseOutSine(progress); break;
        case Type::InOutSine: val = Easing::EaseInOutSine(progress); break;
        case Type::OutInSine: val = Easing::EaseOutInSine(progress); break;
        case Type::InExpo: val = Easing::EaseInExpo(progress); break;
        case Type::OutExpo: val = Easing::EaseOutExpo(progress); break;
        case Type::InOutExpo: val = Easing::EaseInOutExpo(progress); break;
        case Type::OutInExpo: val = Easing::EaseOutInExpo(progress); break;
        case Type::InCirc: val = Easing::EaseInCirc(progress); break;
        case Type::OutCirc: val = Easing::EaseOutCirc(progress); break;
        case Type::InOutCirc: val = Easing::EaseInOutCirc(progress); break;
        case Type::OutInCirc: val = Easing::EaseOutInCirc(progress); break;
        case Type::InElastic: val = Easing::EaseInElastic(progress, amplitude, period); break;
        case Type::OutElastic: val = Easing::EaseOutElastic(progress, amplitude, period); break;
        case Type::InOutElastic: val = Easing::EaseInOutElastic(progress, amplitude, period); break;
        case Type::OutInElastic: val = Easing::EaseOutInElastic(progress, amplitude, period); break;
        case Type::InBack: val = Easing::EaseInBack(progress, overshoot); break;
        case Type::OutBack: val = Easing::EaseOutBack(progress, overshoot); break;
        case Type::InOutBack: val = Easing::EaseInOutBack(progress, overshoot); break;
        case Type::OutInBack: val = Easing::EaseOutInBack(progress, overshoot); break;
        case Type::InBounce: val = Easing::EaseInBounce(progress, amplitude); break;
        case Type::OutBounce: val = Easing::EaseOutBounce(progress, amplitude); break;
        case Type::InOutBounce: val = Easing::EaseInOutBounce(progress, amplitude); break;
        case Type::OutInBounce: val = Easing::EaseOutInBounce(progress, amplitude); break;
    }

    return val;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::EasingCurve::GetAmplitude() const { return amplitude; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::EasingCurve::GetOvershoot() const { return overshoot; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::EasingCurve::GetPeriod() const { return period; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

imanim::EasingCurve::Type imanim::EasingCurve::GetType() const { return easeType; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::EasingCurve::SetAmplitude(const double amp)
{
    if (amp >= 0.0)
    {
        amplitude = amp;
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::EasingCurve::SetOvershoot(const double val)
{
    if (val >= 0.0)
    {
        overshoot = val;
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::EasingCurve::SetPeriod(const double val)
{
    if (val >= 0.0)
    {
        period = val;
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::EasingCurve::SetType(const Type type) { easeType = type; }
