#include <PCH.hpp>

#include "Internal/ImAnim/ImVec2Anim.hpp"

bool imanim::NearlyEqual(const double val1, const double val2, const double tolerance) { return fabs(val1 - val2) < tolerance; }

imanim::ImVec2Anim::ImVec2Anim(ImVec2* pVec2) : vec2(pVec2)
{
    // Add dummy start and end key frames
    keyValues.emplace_back(0.0, ImVec2(1, 1));
    keyValues.emplace_back(1.0, ImVec2(1, 1));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

imanim::ImVec2Anim::~ImVec2Anim() = default;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const ImVec2& imanim::ImVec2Anim::GetEndValue() const { return keyValues[keyValues.size() - 1].value; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const ImVec2& imanim::ImVec2Anim::GetStartValue() const { return keyValues[0].value; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ImVec2Anim::OnStartAnimation()
{
    currentStartKeyFrame = keyValues[0];
    currentEndKeyFrame = keyValues[1];
    endKeyFrameIndex = 1;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ImVec2Anim::SetEndValue(const ImVec2& endVal) { keyValues[keyValues.size() - 1] = KeyValue(1.0, endVal); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ImVec2Anim::SetKeyValueAt(const double step, const ImVec2& vValue)
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

void imanim::ImVec2Anim::SetStartValue(const ImVec2& vStartValue) { keyValues[0] = KeyValue(0.0, vStartValue); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ImVec2Anim::UpdateValueForProgress(const double prog)
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
    ImVec2 v;
    v.x = currentEndKeyFrame.value.x - currentStartKeyFrame.value.x;
    v.y = currentEndKeyFrame.value.y - currentStartKeyFrame.value.y;
    const double localProgress = (prog - currentStartKeyFrame.step) / (currentEndKeyFrame.step - currentStartKeyFrame.step);
    v.x *= static_cast<float>(localProgress);
    v.y *= static_cast<float>(localProgress);
    vec2->x = currentStartKeyFrame.value.x + v.x;
    vec2->y = currentStartKeyFrame.value.y + v.y;
}
