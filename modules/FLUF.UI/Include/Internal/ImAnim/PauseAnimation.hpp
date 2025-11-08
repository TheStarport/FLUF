#pragma once

#include "ImportFlufUi.hpp"
#include "AbstractAnimation.hpp"

namespace imanim
{

    /**
     * @brief An animation that acts like a pause and does nothing during its
     * duration
     */
    class FLUF_UI_API PauseAnimation : public AbstractAnimation
    {
        public:
            /**
         * PauseAnimation constructor
         */
            PauseAnimation() = default;

            /**
         * PauseAnimation constructor
         * @param fDurationInSecs the duration for this pause in seconds
         */
            explicit PauseAnimation(float fDurationInSecs);

            /**
         * PauseAnimation destructor
         */
            virtual ~PauseAnimation() = default;

        protected:
            /**
         * @brief Extending classes should update their value being interpolated
         * based on the specified current progress
         * @param dProgress the current progress for this animation
         */
            void UpdateValueForProgress(double dProgress) override;
    };

} // namespace imanim
