#pragma once

#include "ImportFlufUi.hpp"
#include "AnimationGroup.hpp"

namespace imanim
{

    /**
     * @brief A group of animations that are run in parallel.
     */
    class FLUF_UI_API ParallelAnimationGroup : public AnimationGroup
    {
        public:
            /**
         * Retrieves the duration of this animation.  This is the duration for a
         * single loop of this group, so it is the maximum duration from the
         * animations in this group since the animation runs in parallel.
         * @return this animation's duration in seconds; will return -1 if this
         *      group contains an animation with a loop count of -1.
         */
            [[nodiscard]]
            float GetDuration() const override;

            /**
         * Performs an update of this animation
         */
            void Update() override;

        protected:
            /**
         * Extending classes should override to provide any extra handling at the
         * start of animation
         */
            void OnStartAnimation() override;

            /**
         * Extending classes should override to provide extra handling at the stop
         * of animation
         */
            void OnStopAnimation() override;

            /**
         * @brief Extending classes should update their value being interpolated
         * based on the specified current progress
         * @param dProgress the current progress for this animation
         */
            void UpdateValueForProgress(double dProgress) override;
    };

} // namespace imanim
