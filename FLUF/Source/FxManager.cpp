#include "PCH.hpp"

#include "FxManager.hpp"
#include "FLCore/Server/IObject/GameObject.hpp"

void FxManager::FixedUpdate(float deltaTime)
{
    for (auto effectIter = fxList.begin(); effectIter != fxList.end();)
    {
        if (effectIter->remainingLifetime == 0.f)
        {
            effectIter++;
            continue;
        }

        effectIter->remainingLifetime -= deltaTime;
        if (effectIter->remainingLifetime <= 0.f)
        {
            effectIter = fxList.erase(effectIter);
            continue;
        }
        effectIter++;
    }
}

FxManager::AleEffect* FxManager::AllocateEffect(Id hash, uint allocValue)
{
    using alloc1 = AleEffect*(__cdecl*)(int);
    alloc1 allocFunc = (alloc1)(0x4F79A0);

    using impactConstructor = void*(__thiscall*)(AleEffect*, uint);
    impactConstructor effectConstructor = (impactConstructor)(0x4F8060);

    AleEffect* effect = allocFunc(132);
    effectConstructor(effect, allocValue);

    auto effectData = particleMap->find(hash);
    effect->Initialize(effectData->second);

    return effect;
}

FxManager::AleEffect* FxManager::CreateNewEngineObjectAttachedEffect(const GameObject* gameObj, const Transform& offset, Id hash, float lifetime)
{

    using playParentedEffect = void(__thiscall*)(AleEffect*, const GameObject*, void*, const Transform&);
    playParentedEffect playAle = (playParentedEffect)(0x4F7D20);

    auto effect = AllocateEffect(hash);

    playAle(effect, gameObj, gameObj->cobj->index, offset);

    fxList.emplace_back(gameObj, lifetime, effect);

    return effect;
}

void FxManager::CleanFromGameObject(GameObject* gameObj)
{
    if (!gameObj)
    {
        return;
    }

    for (auto effectIter = fxList.begin(); effectIter != fxList.end();)
    {
        if (effectIter->attachedObject == gameObj)
        {
            effectIter = fxList.erase(effectIter);
            continue;
        }
        effectIter++;
    }
}

void FxManager::CleanAll(bool skipPlayerShip)
{ //TODO: Add handling for the bool flag
    fxList.clear();
}

FxManager::FxInstance::~FxInstance()
{
    static auto AleCleanup1 = reinterpret_cast<void(__thiscall*)(void*)>(0x4F8110);
    static auto AleCleanup2 = reinterpret_cast<void(__thiscall*)(void*)>(0x4F7A90);
    AleCleanup1(effect);
    effect->Cleanup();
    AleCleanup2(effect);
}
