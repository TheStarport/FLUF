#pragma once

#include "ImportFlufUi.hpp"
#include "AbstractAnimation.hpp"
#include "imgui.h"
#include <vector>

namespace imanim
{
    /**
     * Returns whether the two values are nearly equal to each other within the
     * specified tolerance
     * @param val1 the first value of the comparison
     * @param val2 the second value of the comparison
     * @param tolerance the tolerance that the two values must be within
     * @return true if the values are nearly equal; false otherwise
     */
    [[nodiscard]]
    bool FLUF_UI_API NearlyEqual(double val1, double val2, double tolerance = DBL_EPSILON);

    /**
 * @brief Performs an animation using a ImVec2 object
 */
    class FLUF_UI_API ImVec2Anim final : public AbstractAnimation
    {
        public:
            /**
             * ImVec2Anim constructor
             * @param pVec2 the ImVec2 that this animation is for
             */
            explicit ImVec2Anim(ImVec2* pVec2);

            /**
             * ImVec2Anim destructor
             */
            ~ImVec2Anim() override;

            class KeyValue
            {
                public:
                    KeyValue() : step(0.0) {}
                    explicit KeyValue(double dStep, ImVec2 vValue) : step(dStep), value(vValue) {}

                    /// the step for this key frame; must be in the range of 0 to 1
                    double step;

                    /// the value for this key frame
                    ImVec2 value;
            };

            /**
             * @brief Returns the ending animation value
             * @return this animation's ending value
             */
            const ImVec2& GetEndValue() const;

            /**
             * @brief Returns the starting animation value
             * @return this animation's starting value
             */
            const ImVec2& GetStartValue() const;

            /**
             * @brief Sets the ending animation value
             * @param endVal this animation's ending value
             */
            void SetEndValue(const ImVec2& endVal);

            /**
             * Sets a key frame at the specified step with the specified value
             * @param step the step for the frame; must be in the range 0 to 1
             * @param vValue the value for the animation key frame
             */
            void SetKeyValueAt(double step, const ImVec2& vValue);

            /**
             * @brief Sets the starting animation value
             * @param vStartValue this animation's starting value
             */
            void SetStartValue(const ImVec2& vStartValue);

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
            ImVec2* vec2;

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
