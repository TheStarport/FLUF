#include <PCH.hpp>

#include "Internal/ImAnim/FloatAnim.hpp"

#include <Internal/ImAnim/ImVec2Anim.hpp>

imanim::FloatAnim::FloatAnim(float* val) : floatingPointValue(val)
{
    // Add dummy start and end key frames
    keyValues.emplace_back(0.0, 1.f);
    keyValues.emplace_back(1.0, 1.f);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

imanim::FloatAnim::~FloatAnim() = default;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const float& imanim::FloatAnim::GetEndValue() const { return keyValues[keyValues.size() - 1].value; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const float& imanim::FloatAnim::GetStartValue() const { return keyValues[0].value; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::FloatAnim::OnStartAnimation()
{
    currentStartKeyFrame = keyValues[0];
    currentEndKeyFrame = keyValues[1];
    endKeyFrameIndex = 1;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::FloatAnim::SetEndValue(const float& endVal) { keyValues[keyValues.size() - 1] = KeyValue(1.0, endVal); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::FloatAnim::SetKeyValueAt(const double step, const float& vValue)
{
    if ((step < 0.0) || (step > 1.0))
    {
        // Invalid step
        return;
    }

    // Set or insert the key frame
    for (size_t index = 0; index < keyValues.size(); ++index)
    {
        const double currentStep = keyValues[index].step;
        if (NearlyEqual(currentStep, step))
        {
            // Replace the current key frame
            keyValues[index] = KeyValue(step, vValue);
            break;
        }
        if (currentStep > step)
        {
            // Insert the new key frame
            keyValues.insert(keyValues.begin() + index, KeyValue(step, vValue));
            break;
        }
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::FloatAnim::SetStartValue(const float& vStartValue) { keyValues[0] = KeyValue(0.0, vStartValue); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::FloatAnim::UpdateValueForProgress(const double prog)
{
    // Make sure that the current start and end frames are valid
    if (prog > currentEndKeyFrame.step)
    {
        // Find the new start and end frames
        for (size_t index = (endKeyFrameIndex + 1); index < keyValues.size(); ++index)
        {
            if (const KeyValue kv = keyValues[index]; kv.step >= prog)
            {
                currentEndKeyFrame = kv;
                currentStartKeyFrame = keyValues[index - 1];
                endKeyFrameIndex = index;
                break;
            }
        }
    }

    // Calculate the new value
    float v = currentEndKeyFrame.value - currentStartKeyFrame.value;
    const double localProgress = (prog - currentStartKeyFrame.step) / (currentEndKeyFrame.step - currentStartKeyFrame.step);
    v *= static_cast<float>(localProgress);
    *floatingPointValue = currentStartKeyFrame.value + v;
}
