#pragma once
#include "ImportFluf.hpp"

class Fluf;
class FxManager
{
        friend Fluf;
        inline static FxManager* instance;

        struct AleEffect
        {
                virtual void Initialize(void* effectData);
                virtual void Cleanup();
                uint dunno[20];
        };

        struct FxInstance
        {
                ~FxInstance();

                const GameObject* attachedObject;
                float remainingLifetime = 0.f;
                AleEffect* effect;
        };

        std::list<FxInstance> fxList;

        void FixedUpdate(float deltaTime);
        AleEffect* AllocateEffect(Id hash, uint allocValue = 0);

        inline static st6::map<uint, void*>* particleMap = (st6::map<uint, void*>*)(0x6164B0);

     public:
        FLUF_API AleEffect* CreateNewEngineObjectAttachedEffect(const GameObject* gameObj, const Transform& transform, Id hash, float lifetime = 0.f);
        //void CreateNewEquipAttachedEffect(GameObject* gameObj, Id hash, float lifetime = 1.f);
        //void CreateNewFreefloatingEffect(const Vector&, Id hash, float lifetime = 1.f);

        FLUF_API void CleanFromGameObject(GameObject* gameObj);
        FLUF_API void CleanAll(bool skipPlayerShip);
};