#include <PCH.hpp>

#include "Internal/ImAnim/AnimationGroup.hpp"
#include <algorithm>

void imanim::AnimationGroup::AddAnimation(AbstractAnimation* pAnimation)
{
    if (pAnimation != nullptr)
    {
        animationList.push_back(pAnimation);
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::AnimationGroup::Clear() { animationList.clear(); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

imanim::AbstractAnimation* imanim::AnimationGroup::GetAnimationAt(const int index) const
{
    AbstractAnimation* anim = nullptr;

    if (index >= 0 && index < animationList.size())
    {
        anim = animationList[index];
    }

    return anim;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int imanim::AnimationGroup::GetAnimationCount() const { return static_cast<int>(animationList.size()); }

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void imanim::AnimationGroup::InsertAnimation(int index, AbstractAnimation* pAnimation)
{
    if (pAnimation == nullptr)
    {
        return;
    }

    if (index < 0)
    {
        index = 0;
    }

    if (index >= animationList.size())
    {
        animationList.push_back(pAnimation);
    }
    else
    {
        animationList.insert(animationList.begin() + index, pAnimation);
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int imanim::AnimationGroup::RemoveAnimation(AbstractAnimation* anim)
{
    int nRemoveCount = 0;

    for (auto iter = animationList.begin(); iter != animationList.end();)
    {
        if (*iter == anim)
        {
            iter = animationList.erase(iter);
            nRemoveCount++;
        }
        else
        {
            ++iter;
        }
    }

    return nRemoveCount;
}
