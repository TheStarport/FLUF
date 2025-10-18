#pragma once

struct ContentStory
{
        void* vtable00;
        void* vtable04;
        DWORD dunno08;
        uint missionStage;
        DWORD dunno10;
        float dunno14;
        bool dunno18;
        const char name[32];
};

class Retold;
class RetoldHooks
{
        friend Retold;

        template <typename T>
        using CreateContentMessageHandler = T(__thiscall*)(T, void* contentInstance, DWORD* payload);
        inline static std::unique_ptr<FunctionDetour<CreateContentMessageHandler<ContentStory*>>> contentStoryCreateDetour = nullptr;

        using ConsumeFireResourcesType = void(__thiscall*)(CELauncher* launcher);
        inline static FunctionDetour<ConsumeFireResourcesType> consumeFireResourcesDetour{ reinterpret_cast<ConsumeFireResourcesType>(
            GetProcAddress(GetModuleHandleA("common.dll"), "?ConsumeFireResources@CELauncher@@UAEXXZ")) };

        using GunCanFireType = FireResult(__thiscall*)(CEGun* gun, Vector& target);
        inline static FunctionDetour<GunCanFireType> gunCanFireDetour{ reinterpret_cast<GunCanFireType>(
            GetProcAddress(GetModuleHandleA("common.dll"), "?CanFire@CEGun@@MBE?AW4FireResult@@ABVVector@@@Z")) };

        using ShieldSetHealth = void(__thiscall*)(CEShield* shield, float hitPts);
        inline static FunctionDetour<ShieldSetHealth> shieldSetHealthDetour{ reinterpret_cast<ShieldSetHealth>(
            GetProcAddress(GetModuleHandleA("common.dll"), "?SetHitPoints@CEShield@@UAEXM@Z")) };
};
