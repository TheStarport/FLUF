#pragma once

#include "ImportFlufUi.hpp"
#include "AbstractAnimation.hpp"
#include "imgui.h"
#include <vector>

namespace imanim
{

    /**
     * @brief Performs an animation using a ImVec2 object
     */
    class FLUF_UI_API FloatAnim final : public AbstractAnimation
    {
        public:
            explicit FloatAnim(float* fl);
            ~FloatAnim() override;

            class KeyValue
            {
                public:
                    KeyValue() : step(0.0) {}
                    explicit KeyValue(double dStep, float val) : step(dStep), value(val) {}

                    /// the step for this key frame; must be in the range of 0 to 1
                    double step;

                    /// the value for this key frame
                    float value;
            };

            /**
             * @brief Returns the ending animation value
             * @return this animation's ending value
             */
            const float& GetEndValue() const;

            /**
             * @brief Returns the starting animation value
             * @return this animation's starting value
             */
            const float& GetStartValue() const;

            /**
             * @brief Sets the ending animation value
             * @param endVal this animation's ending value
             */
            void SetEndValue(const float& endVal);

            /**
             * Sets a key frame at the specified step with the specified value
             * @param step the step for the frame; must be in the range 0 to 1
             * @param val the value for the animation key frame
             */
            void SetKeyValueAt(double step, const float& val);

            /**
             * @brief Sets the starting animation value
             * @param startValue this animation's starting value
             */
            void SetStartValue(const float& startValue);

        protected:
            /**
             * Extending classes should override to provide any extra handling at the
             * start of animation
             */
            void OnStartAnimation() override;

            /**
             * @brief Extending classes should update their value being interpolated
             * based on the specified current progress
             * @param prog the current progress for this animation
             */
            void UpdateValueForProgress(double prog) override;

            /// the ImVec2 being animated
            float* floatingPointValue;

            /// the list of key values (frames) for this animation; it will always have
            /// a minimum of two values: the start and end frames
            std::vector<KeyValue> keyValues;

            /// the key value for the start of the current frame
            KeyValue currentStartKeyFrame;

            /// the key value for the end of the current frame
            KeyValue currentEndKeyFrame;

            /// the index of the key value in m_vecKeyValues for the end of the current
            /// frame
            size_t endKeyFrameIndex{ 0 };
    };

} // namespace imanim
