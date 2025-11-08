#pragma once

#include "ImportFlufUi.hpp"

namespace imanim
{

    /**
 * @brief Provides easing equations
 */
    class FLUF_UI_API Easing
    {
        public:
            /**
     * @brief Easing equation for a simple linear tweening, with no easing
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseNone(double t);

            /**
     * @brief Easing equation for a quadratic (t^2) easing in, accelerating from
     * zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInQuad(double t);

            /**
     * @brief Easing equation for a quadratic (t^2) easing out, decelerating to
     * zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutQuad(double t);

            /**
     * @brief Easing equation for a quadratic (t^2) easing in/out, accelerating
     * until halfway, then decelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInOutQuad(double t);

            /**
     * @brief Easing equation for a quadratic (t^2) easing out/in, decelerating
     * until halfway, then accelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutInQuad(double t);

            /**
     * @brief Easing equation for a cubic (t^3) easing in, accelerating from
     * zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInCubic(double t);

            /**
     * @brief Easing equation for a cubic (t^3) easing out, decelerating to
     * zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutCubic(double t);

            /**
     * @brief Easing equation for a cubic (t^3) easing in/out, accelerating
     * until halfway, then decelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInOutCubic(double t);

            /**
     * @brief Easing equation for a cubic (t^3) easing out/in, decelerating
     * until halfway, then accelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutInCubic(double t);

            /**
     * @brief Easing equation for a quartic (t^4) easing in, accelerating from
     * zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInQuart(double t);

            /**
     * @brief Easing equation for a quartic (t^4) easing out, decelerating to
     * zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutQuart(double t);

            /**
     * @brief Easing equation for a quartic (t^4) easing in/out, accelerating
     * until halfway, then decelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInOutQuart(double t);

            /**
     * @brief Easing equation for a quartic (t^4) easing out/in, decelerating
     * until halfway, then accelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutInQuart(double t);

            /**
     * @brief Easing equation for a quintic (t^5) easing in, accelerating from
     * zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInQuint(double t);

            /**
     * @brief Easing equation for a quintic (t^5) easing out, decelerating to
     * zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutQuint(double t);

            /**
     * @brief Easing equation for a quintic (t^5) easing in/out, accelerating
     * until halfway, then decelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInOutQuint(double t);

            /**
     * @brief Easing equation for a quintic (t^5) easing out/in, decelerating
     * until halfway, then accelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutInQuint(double t);

            /**
     * @brief Easing equation for a sinusoidal (sin(t)) easing in, accelerating
     * from zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInSine(double t);

            /**
     * @brief Easing equation for a sinusoidal (sin(t)) easing out, decelerating
     * to zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutSine(double t);

            /**
     * @brief Easing equation for a sinusoidal (sin(t)) easing in/out,
     * accelerating until halfway, then decelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInOutSine(double t);

            /**
     * @brief Easing equation for a sinusoidal (sin(t)) easing out/in,
     * decelerating until halfway, then accelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutInSine(double t);

            /**
     * @brief Easing equation for an exponential (2^t) easing in, accelerating
     * from zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInExpo(double t);

            /**
     * @brief Easing equation for an exponential (2^t) easing out, decelerating
     * to zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutExpo(double t);

            /**
     * @brief Easing equation for an exponential (2^t) easing in/out,
     * accelerating until halfway, then decelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInOutExpo(double t);

            /**
     * @brief Easing equation for an exponential (2^t) easing out/in,
     * decelerating until halfway, then accelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutInExpo(double t);

            /**
     * @brief Easing equation for a circular (sqrt(1-t^2)) easing in,
     * accelerating from zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInCirc(double t);

            /**
     * @brief Easing equation for a circular (sqrt(1-t^2)) easing out,
     * decelerating to zero velocity
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutCirc(double t);

            /**
     * @brief Easing equation for a circular (sqrt(1-t^2)) easing in/out,
     * accelerating until halfway, then decelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInOutCirc(double t);

            /**
     * @brief Easing equation for a circular (sqrt(1-t^2)) easing out/in,
     * decelerating until halfway, then accelerating
     * @param t current time in frames or seconds
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutInCirc(double t);

            /**
     * @brief Easing equation for an elastic (exponentially decaying sine wave)
     * easing in, accelerating from zero velocity
     * @param t current time in frames or seconds
     * @param a the amplitude of the elastic "spring" effect
     * @param p the period, where a small value will give a high frequency of
     *      the curve and a large period will give it a small frequency
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInElastic(double t, double a, double p);

            /**
     * @brief Easing equation for an elastic (exponentially decaying sine wave)
     * easing out, decelerating to zero velocity
     * @param t current time in frames or seconds
     * @param a the amplitude of the elastic "spring" effect
     * @param p the period, where a small value will give a high frequency of
     *      the curve and a large period will give it a small frequency
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutElastic(double t, double a, double p);

            /**
     * @brief Easing equation for an elastic (exponentially decaying sine wave)
     * easing in/out, accelerating until halfway, then decelerating
     * @param t current time in frames or seconds
     * @param a the amplitude of the elastic "spring" effect
     * @param p the period, where a small value will give a high frequency of
     *      the curve and a large period will give it a small frequency
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInOutElastic(double t, double a, double p);

            /**
     * @brief Easing equation for an elastic (exponentially decaying sine wave)
     * easing out/in, decelerating until halfway, then accelerating
     * @param t current time in frames or seconds
     * @param a the amplitude of the elastic "spring" effect
     * @param p the period, where a small value will give a high frequency of
     *      the curve and a large period will give it a small frequency
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutInElastic(double t, double a, double p);

            /**
     * @brief Easing equation for a back (overshooting cubic easing:
     * (s+1)*t^3 - s*t^2) easing in, accelerating from zero velocity
     * @param t current time in frames or seconds
     * @param s overshoot amount where higher means greater overshoot (0
     *      produces cubic easing with no overshoot, and the default value of
     *      1.70158 produces an overshoot of 10 percent)
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInBack(double t, double s);

            /**
     * @brief Easing equation for a back (overshooting cubic easing:
     * (s+1)*t^3 - s*t^2) easing out, decelerating to zero velocity
     * @param t current time in frames or seconds
     * @param s overshoot amount where higher means greater overshoot (0
     *      produces cubic easing with no overshoot, and the default value of
     *      1.70158 produces an overshoot of 10 percent)
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutBack(double t, double s);

            /**
     * @brief Easing equation for a back (overshooting cubic easing:
     * (s+1)*t^3 - s*t^2)  easing in/out, accelerating until halfway, then
     * decelerating
     * @param t current time in frames or seconds
     * @param s overshoot amount where higher means greater overshoot (0
     *      produces cubic easing with no overshoot, and the default value of
     *      1.70158 produces an overshoot of 10 percent)
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInOutBack(double t, double s);

            /**
     * @brief Easing equation for a back (overshooting cubic easing:
     * (s+1)*t^3 - s*t^2) easing out/in, decelerating until halfway, then
     * accelerating
     * @param t current time in frames or seconds
     * @param s overshoot amount where higher means greater overshoot (0
     *      produces cubic easing with no overshoot, and the default value of
     *      1.70158 produces an overshoot of 10 percent)
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutInBack(double t, double s);

            /**
     * @brief Easing equation for a bounce (exponentially decaying parabolic
     * bounce) easing in, accelerating from zero velocity
     * @param t current time in frames or seconds
     * @param a the amplitude of the bounce effect
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInBounce(double t, double a);

            /**
     * @brief Easing equation for a bounce (exponentially decaying parabolic
     * bounce) easing out, decelerating to zero velocity
     * @param t current time in frames or seconds
     * @param a the amplitude of the bounce effect
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutBounce(double t, double a);

            /**
     * @brief Easing equation for a bounce (exponentially decaying parabolic
     * bounce)  easing in/out, accelerating until halfway, then decelerating
     * @param t current time in frames or seconds
     * @param a the amplitude of the bounce effect
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseInOutBounce(double t, double a);

            /**
     * @brief Easing equation for a bounce (exponentially decaying parabolic
     * bounce) easing out/in, decelerating until halfway, then accelerating
     * @param t current time in frames or seconds
     * @param a the amplitude of the bounce effect
     * @return the calculated time
     */
            [[nodiscard]]
            static double EaseOutInBounce(double t, double a);

        private:
            /**
     * @brief Helper for calculating easing equation for an elastic easing in
     */
            [[nodiscard]]
            static double EaseInElasticHelper(double t, double b, double c, double d, double a, double p);

            /**
     * @brief Helper for calculating easing equation for a bounce easing out
     */
            [[nodiscard]]
            static double EaseOutBounceHelper(double t, double c, double a);

            /**
     * @brief Helper for calculating easing equation for an elastic easing out
     */
            [[nodiscard]]
            static double EaseOutElasticHelper(double t, double b, double c, double d, double a, double p);
    };

} // namespace imanim
