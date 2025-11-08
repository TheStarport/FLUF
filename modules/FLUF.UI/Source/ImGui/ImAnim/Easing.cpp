/*
Disclaimer for Robert Penner's Easing Equations license:
TERMS OF USE - EASING EQUATIONS
Open source under the BSD License.
Copyright © 2001 Robert Penner
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of contributors may be used
      to endorse or promote products derived from this software without specific
      prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <PCH.hpp>

#include "Internal/ImAnim/Easing.hpp"
#include <algorithm>
#include <cmath>

double imanim::Easing::EaseNone(const double t) { return t; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInQuad(const double t) { return t * t; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutQuad(const double t) { return -t * (t - 2.0); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInOutQuad(double t)
{
    t *= 2.0;
    if (t < 1.0)
    {
        return t * t / 2.0;
    }
    --t;
    return -0.5 * (t * (t - 2.0) - 1.0);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutInQuad(const double t)
{
    if (t < 0.5)
    {
        return EaseOutQuad(t * 2.0) / 2.0;
    }
    return EaseInQuad((2.0 * t) - 1.0) / 2.0 + 0.5;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInCubic(const double t) { return t * t * t; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutCubic(double t)
{
    t -= 1.0;
    return t * t * t + 1.0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInOutCubic(double t)
{
    t *= 2.0;
    if (t < 1.0)
    {
        return 0.5 * t * t * t;
    }
    t -= 2.0;
    return 0.5 * (t * t * t + 2.0);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutInCubic(const double t)
{
    if (t < 0.5)
    {
        return EaseOutCubic(2.0 * t) / 2.0;
    }
    return EaseInCubic(2.0 * t - 1.0) / 2.0 + 0.5;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInQuart(const double t) { return t * t * t * t; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutQuart(double t)
{
    t -= 1.0;
    return -(t * t * t * t - 1.0);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInOutQuart(double t)
{
    t *= 2.0;
    if (t < 1.0)
    {
        return 0.5 * t * t * t * t;
    }
    t -= 2.0;
    return -0.5 * (t * t * t * t - 2.0);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutInQuart(const double t)
{
    if (t < 0.5)
    {
        return EaseOutQuart(2.0 * t) / 2.0;
    }
    return EaseInQuart(2.0 * t - 1.0) / 2.0 + 0.5;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInQuint(const double t) { return t * t * t * t * t; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutQuint(double t)
{
    t -= 1.0;
    return t * t * t * t * t + 1.0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInOutQuint(double t)
{
    t *= 2.0;
    if (t < 1.0)
    {
        return 0.5 * t * t * t * t * t;
    }
    t -= 2.0;
    return 0.5 * (t * t * t * t * t + 2.0);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutInQuint(const double t)
{
    if (t < 0.5)
    {
        return EaseOutQuint(2.0 * t) / 2.0;
    }
    return EaseInQuint(2.0 * t - 1.0) / 2.0 + 0.5;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInSine(const double t) { return (t == 1.0) ? 1.0 : -cos(t * M_PI_2) + 1.0; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutSine(const double t) { return sin(t * M_PI_2); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInOutSine(const double t) { return -0.5 * (cos(M_PI * t) - 1.0); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutInSine(const double t)
{
    if (t < 0.5)
    {
        return EaseOutSine(2.0 * t) / 2.0;
    }
    return EaseInSine(2.0 * t - 1.0) / 2.0 + 0.5;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInExpo(const double t) { return (t == 0.0 || t == 1.0) ? t : pow(2.0, 10.0 * (t - 1.0)) - 0.001; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutExpo(const double t) { return (t == 1.0) ? 1.0 : 1.001 * (-pow(2.0, -10 * t) + 1.0); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInOutExpo(double t)
{
    if (t == 0.0)
    {
        return 0.0;
    }
    if (t == 1.0)
    {
        return 1.0;
    }

    t *= 2.0;
    if (t < 1.0)
    {
        return 0.5 * pow(2.0, 10.0 * (t - 1.0)) - 0.0005;
    }
    return 0.5 * 1.0005 * (-pow(2.0, -10.0 * (t - 1.0)) + 2.0);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutInExpo(const double t)
{
    if (t < 0.5)
    {
        return EaseOutExpo(2.0 * t) / 2.0;
    }
    return EaseInExpo(2.0 * t - 1.0) / 2.0 + 0.5;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInCirc(const double t) { return -(sqrt(1.0 - t * t) - 1.0); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutCirc(double t)
{
    t -= 1.0;
    return sqrt(1.0 - t * t);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInOutCirc(double t)
{
    t *= 2.0;
    if (t < 1.0)
    {
        return -0.5 * (sqrt(1.0 - t * t) - 1.0);
    }
    t -= 2.0;
    return 0.5 * (sqrt(1.0 - t * t) + 1.0);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutInCirc(const double t)
{
    if (t < 0.5)
    {
        return EaseOutCirc(2.0 * t) / 2.0;
    }
    return EaseInCirc(2.0 * t - 1.0) / 2.0 + 0.5;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInElastic(const double t, const double a, const double p) { return EaseInElasticHelper(t, 0.0, 1.0, 1.0, a, p); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutElastic(const double t, const double a, const double p) { return EaseOutElasticHelper(t, 0.0, 1.0, 1.0, a, p); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInOutElastic(double t, double a, const double p)
{
    if (t == 0.0)
    {
        return 0.0;
    }
    t *= 2.0;
    if (t == 2.0)
    {
        return 1.0;
    }

    double s;
    if (a < 1.0)
    {
        a = 1.0;
        s = p / 4.0;
    }
    else
    {
        s = p / (2.0 * M_PI) * asin(1.0 / a);
    }

    if (t < 1.0)
    {
        return -0.5 * (a * pow(2.0, 10.0 * (t - 1.0)) * sin((t - 1.0 - s) * (2.0 * M_PI) / p));
    }
    return a * pow(2.0, -10.0 * (t - 1.0)) * sin((t - 1.0 - s) * (2.0 * M_PI) / p) * 0.5 + 1.0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutInElastic(const double t, const double a, const double p)
{
    if (t < 0.5)
    {
        return EaseOutElasticHelper(t * 2.0, 0.0, 0.5, 1.0, a, p);
    }
    return EaseInElasticHelper(2.0 * t - 1.0, 0.5, 0.5, 1.0, a, p);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInBack(const double t, const double s) { return t * t * ((s + 1.0) * t - s); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutBack(double t, const double s)
{
    t -= 1.0;
    return t * t * ((s + 1.0) * t + s) + 1.0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInOutBack(double t, double s)
{
    t *= 2.0;
    if (t < 1.0)
    {
        s *= 1.525;
        return 0.5 * (t * t * ((s + 1.0) * t - s));
    }

    t -= 2.0;
    s *= 1.525;
    return 0.5 * (t * t * ((s + 1.0) * t + s) + 2.0);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutInBack(const double t, const double s)
{
    if (t < 0.5)
    {
        return EaseOutBack(2.0 * t, s) / 2.0;
    }

    return EaseInBack(2.0 * t - 1.0, s) / 2.0 + 0.5;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInBounce(const double t, const double a) { return 1.0 - EaseOutBounceHelper(1.0 - t, 1.0, a); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutBounce(const double t, const double a) { return EaseOutBounceHelper(t, 1.0, a); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseInOutBounce(const double t, const double a)
{
    if (t < 0.5)
    {
        return EaseInBounce(2.0 * t, a) / 2.0;
    }
    return (t == 1.0) ? 1.0 : EaseOutBounce(2.0 * t - 1.0, a) / 2.0 + 0.5;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutInBounce(const double t, const double a)
{
    if (t < 0.5)
    {
        return EaseOutBounceHelper(t * 2.0, 0.5, a);
    }

    return 1.0 - EaseOutBounceHelper(2.0 - 2.0 * t, 0.5, a);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// private methods

double imanim::Easing::EaseInElasticHelper(const double t, const double b, const double c, const double d, double a, const double p)
{
    if (t == 0.0)
    {
        return b;
    }

    double t_adj = t / d;
    if (t_adj == 1.0)
    {
        return b + c;
    }

    double s;
    if (a < fabs(c))
    {
        a = c;
        s = p / 4.0;
    }
    else
    {
        s = p / (2.0 * M_PI) * asin(c / a);
    }

    t_adj -= 1.0;
    return -(a * pow(2.0, 10.0 * t_adj) * sin((t_adj * d - s) * (2.0 * M_PI) / p)) + b;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutBounceHelper(double t, const double c, const double a)
{
    if (t == 1.0)
    {
        return c;
    }
    if (t < (4.0 / 11.0))
    {
        return c * (7.5625 * t * t);
    }
    if (t < (8.0 / 11.0))
    {
        t -= (6.0 / 11.0);
        return -a * (1.0 - (7.5625 * t * t + 0.75)) + c;
    }
    if (t < (10.0 / 11.0))
    {
        t -= (9.0 / 11.0);
        return -a * (1.0 - (7.5625 * t * t + 0.9375)) + c;
    }
    t -= (21.0 / 22.0);
    return -a * (1.0 - (7.5625 * t * t + 0.984375)) + c;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

double imanim::Easing::EaseOutElasticHelper(const double t, double /*b*/, const double c, double /*d*/, double a, const double p)
{
    if (t == 0.0)
    {
        return 0.0;
    }
    if (t == 1.0)
    {
        return c;
    }

    double s;
    if (a < c)
    {
        a = c;
        s = p / 4.0;
    }
    else
    {
        s = p / (2.0 * M_PI) * asin(c / a);
    }

    return (a * pow(2.0, -10.0 * t) * sin((t - s) * (2.0 * M_PI) / p) + c);
}
