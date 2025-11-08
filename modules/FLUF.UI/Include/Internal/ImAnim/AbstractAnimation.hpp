#pragma once

#include "ImportFlufUi.hpp"

#include "EasingCurve.hpp"
#include <chrono>

namespace imanim
{
    /**
     * @brief The base for all animations
     */
    class FLUF_UI_API AbstractAnimation
    {
        public:
            /**
             * AbstractAnimation constructor
             */
            AbstractAnimation() = default;

            /**
             * AbstractAnimation destructor
             */
            virtual ~AbstractAnimation() = default;

            enum class State
            {
                Stopped,
                Running,
            };

            using OnAnimationStateChangedCallback = std::function<void(AbstractAnimation* anim, State newState)>;

            /**
             * Returns the current loop of this animation; once the animation completes
             * its duration, this count will be incremented.  If the loop count is 1,
             * this will always be 0.
             * @return the current loop of this animation
             */
            [[nodiscard]]
            int GetCurrentLoop() const
            {
                return currentLoop;
            }

            /**
             * Retrieves the duration of this animation
             * @return this animation's duration in seconds
             */
            [[nodiscard]]
            virtual float GetDuration() const
            {
                return durationInSecs;
            }

            /**
             * Retrieves the easing curve being used for this animation
             * @return this animation's easing curve
             */
            [[nodiscard]]
            const EasingCurve& GetEasingCurve() const;

            /**
             * Returns the loop count for this animation, which is the numer of loops
             * that this animation should loop before stopping.  A value of 0 indicates
             * that this animation will not run at all; a value of < 0 indicates that
             * this animation will loop forever.
             * @return this animation's loop count
             */
            [[nodiscard]]
            int GetLoopCount() const
            {
                return loopCount;
            }

            /**
             * Returns the state of this animate
             * @return this animation's state
             */
            [[nodiscard]]
            State GetState() const
            {
                return animationState;
            }

            /**
             * Sets the duration for this animation
             * @param timeInSecs the duration time in seconds
             */
            void SetDuration(float timeInSecs);

            /**
             * Sets the easing curve equation used by this animation
             * @param easingCurveType the type of easing curve
             * @param amplitude the amplitude for the easing curve.  This is only
             *      applicable to bounce and elastic curves and will be ignored for
             *      other curves.  It is the amplitude of the bounce or elastic
             *      (spring) effect.  Values < 0.0 are invalid and will be ignored.
             * @param period  the period for the easing curve, where a small value will
             *      give a high frequency of the curve and a large period will give it a
             *      small frequency.  This is only applicable to elastic curves and will
             *      be ignored for other curves.  Values < 0.0 are invalid and will be
             *      ignored.
             * @param overshoot the overshoot amount where higher means greater
             *      overshoot (0 produces cubic easing with no overshoot, and the
             *      default value of 1.70158 produces an overshoot of 10 percent).
             *      This is only applicable to back curves and will be ignored for other
             *      curves.  Values < 0.0 are invalid and will be ignored.
             */
            void SetEasingCurve(EasingCurve::Type easingCurveType, double amplitude = 1.0, double period = 0.3, double overshoot = 1.70158);

            /**
             * Sets the loop count for this animation, which is the numer of loops that
             * this animation should loop before stopping.  A value of 0 indicates that
             * this animation will not run at all; a value of -1 indicates that this
             * animation will loop forever.
             * @param count this animation's loop count
             */
            void SetLoopCount(int count);

            /**
             * Starts this animation
             */
            void Start();

            /**
             * Stops this animation.
             */
            void Stop();

            /**
             * Performs an update of this animation
             */
            virtual void Update();

            /**
             * Execute a function when the state changes, e.g animation Start()/End() is called, or the animation reaches the end of a loop
             * @param callback The function to execute when the state changes
             */
            void SetAnimationStateChangedCallback(const OnAnimationStateChangedCallback& callback);

        protected:
            /**
             * Extending classes should override to provide any extra handling at the
             * start of animation
             */
            virtual void OnStartAnimation() {}

            /**
             * Extending classes should override to provide extra handling at the stop
             * of animation
             */
            virtual void OnStopAnimation() {}

            /**
             * @brief Extending classes should update their value being interpolated
             * based on the specified current progress
             * @param progress the current progress for this animation
             */
            virtual void UpdateValueForProgress(double progress) = 0;

            /// the state of this animation
            State animationState{ State::Stopped };

            /// the loop count for this animation, which is the numer of loops that this
            /// animation should loop before stopping.  A value of 0 indicates that this
            /// animation will not run at all; a value of -1 indicates that this
            /// animation will loop forever.
            int loopCount{ 1 };

            /// the current loop of this animation; once the animation completes its
            /// duration, this count will be incremented.  If the loop count is 1, this
            /// will always be 0.
            int currentLoop{ 0 };

            void RunFunctionCallback(State newState);

        private:
            /// the easing curve used for this animation
            EasingCurve easingCurve;

            /// the duration for this animation in seconds
            float durationInSecs{ 0.0F };

            /// The time stamp upon starting animation
            std::chrono::steady_clock::time_point startTime;

            /// A callback to be fired when the animation starts or finishes
            OnAnimationStateChangedCallback onAnimationStateChangedCallback;
    };

} // namespace imanim
