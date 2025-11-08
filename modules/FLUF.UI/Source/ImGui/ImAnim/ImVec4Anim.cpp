#include <PCH.hpp>

#include "Internal/ImAnim/ImVec4Anim.hpp"

#include "Internal/ImAnim/ImVec2Anim.hpp"

imanim::ImVec4Anim::ImVec4Anim(ImVec4* pVec4) : vec4(pVec4)
{
    // Add dummy start and end key frames
    keyValues.emplace_back(0.0, ImVec4(1, 1, 1, 1));
    keyValues.emplace_back(1.0, ImVec4(1, 1, 1, 1));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

imanim::ImVec4Anim::~ImVec4Anim() {}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const ImVec4& imanim::ImVec4Anim::GetEndValue() const { return keyValues[keyValues.size() - 1].value; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const ImVec4& imanim::ImVec4Anim::getStartValue() const { return keyValues[0].value; }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ImVec4Anim::OnStartAnimation()
{
    currentStartKeyFrame = keyValues[0];
    currentEndKeyFrame = keyValues[1];
    endKeyFrameIndex = 1;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ImVec4Anim::SetEndValue(const ImVec4& vEndValue) { keyValues[keyValues.size() - 1] = KeyValue(1.0, vEndValue); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ImVec4Anim::SetKeyValueAt(const double step, const ImVec4& val)
{
    if ((step < 0.0) || (step > 1.0))
    {
        // Invalid step
        return;
    }

    // Set or insert the key frame
    for (size_t index = 0; index < keyValues.size(); ++index)
    {
        const double dCurStep = keyValues[index].step;
        if (NearlyEqual(dCurStep, step))
        {
            // Replace the current key frame
            keyValues[index] = KeyValue(step, val);
            break;
        }
        if (dCurStep > step)
        {
            // Insert the new key frame
            keyValues.insert(keyValues.begin() + index, KeyValue(step, val));
            break;
        }
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ImVec4Anim::SetStartValue(const ImVec4& vStartValue) { keyValues[0] = KeyValue(0.0, vStartValue); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::ImVec4Anim::UpdateValueForProgress(const double dProgress)
{
    // Make sure that the current start and end frames are valid
    if (dProgress > currentEndKeyFrame.step)
    {
        // Find the new start and end frames
        for (size_t index = (endKeyFrameIndex + 1); index < keyValues.size(); ++index)
        {
            const KeyValue keyValue = keyValues[index];
            if (keyValue.step >= dProgress)
            {
                currentEndKeyFrame = keyValue;
                currentStartKeyFrame = keyValues[index - 1];
                endKeyFrameIndex = index;
                break;
            }
        }
    }

    // Calculate the new value
    ImVec4 v;
    v.x = currentEndKeyFrame.value.x - currentStartKeyFrame.value.x;
    v.y = currentEndKeyFrame.value.y - currentStartKeyFrame.value.y;
    v.z = currentEndKeyFrame.value.z - currentStartKeyFrame.value.z;
    v.w = currentEndKeyFrame.value.w - currentStartKeyFrame.value.w;
    const double localProgress = (dProgress - currentStartKeyFrame.step) / (currentEndKeyFrame.step - currentStartKeyFrame.step);
    v.x *= static_cast<float>(localProgress);
    v.y *= static_cast<float>(localProgress);
    v.z *= static_cast<float>(localProgress);
    v.w *= static_cast<float>(localProgress);
    vec4->x = currentStartKeyFrame.value.x + v.x;
    vec4->y = currentStartKeyFrame.value.y + v.y;
    vec4->z = currentStartKeyFrame.value.z + v.z;
    vec4->w = currentStartKeyFrame.value.w + v.w;
}
