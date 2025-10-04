#include "PCH.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include "SmoothStrafing.hpp"

#include <KeyManager.hpp>
#include <ShipManipulator.hpp>
#include <Utils/MemUtils.hpp>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void SmoothStrafing::ReadIniFile(INI_Reader& ini)
{
    while (ini.read_header())
    {
        if (!ini.is_header("Ship"))
        {
            continue;
        }

        std::string nickname;
        float strafeAcceleration = 0.f;
        while (ini.read_value())
        {
            if (ini.is_value("nickname"))
            {
                nickname = ini.get_value_string();
            }
            else if (ini.is_value("strafe_force_acceleration"))
            {
                strafeAcceleration = ini.get_value_float(0);
            }
        }

        if (nickname.empty() || !strafeAcceleration)
        {
            continue;
        }

        shipStrafeForces[CreateID(nickname.c_str())] = strafeAcceleration;
    }
}

float* __fastcall SmoothStrafing::GetStrafeForce(const CShip* ship)
{
    static float retValue;

    const auto strafeForceAcceleration = shipStrafeForces.find(ship->shiparch()->archId);
    if (const auto player = Fluf::GetPlayerCShip(); player != ship || strafeForceAcceleration == shipStrafeForces.end())
    {
        retValue = ship->shiparch()->strafeForce;
        return &retValue;
    }

    static StrafeDir lastStrafeDirection = StrafeDir::None;
    if (const auto dir = ship->strafeDir; dir != lastStrafeDirection)
    {
        lastStrafeDirection = dir;
        totalTimeBeenStrafing = 0.f;
    }

    totalTimeBeenStrafing += deltaTime;

    retValue = std::clamp(strafeForceAcceleration->second * totalTimeBeenStrafing, 0.f, ship->shiparch()->strafeForce);
    currentStrafeForce = retValue;
    return &retValue;
}

void __declspec(naked) SmoothStrafing::OnStrafeForceApply()
{
    static constexpr DWORD retAddress = 0x62BBA6E;
    __asm
    {
        push eax
        call GetStrafeForce
        fld [eax]
        pop eax
        jmp retAddress
    }
}

float* __fastcall SmoothStrafing::GetThrusterForce(const Archetype::Thruster* archetype, const CEqObj* owner)
{
    static float retValue;
    if (const auto player = Fluf::GetPlayerCShip(); player != owner)
    {
        retValue = archetype->maxForce;
        return &retValue;
    }

    retValue = std::clamp(archetype->maxForce - currentStrafeForce, 0.f, archetype->maxForce);
    return &retValue;
}

void __declspec(naked) SmoothStrafing::OnThrusterForceApply()
{
    static constexpr DWORD retAddress = 0x629D341;
    __asm
    {
        push eax
        push edx
        mov edx, [esi-0x48] // CEqObj, owner, ecx already contains the archetype
        call GetThrusterForce
        fld [eax]
        pop edx
        pop eax
        jmp retAddress
    }
}

void SmoothStrafing::BeforePhysicsUpdate(uint system, const float delta)
{
    deltaTime = delta;

    auto player = Fluf::GetPlayerCShip();
    if (!player)
    {
        return;
    }

    const auto dir = player->strafeDir;
    if (dir == StrafeDir::None)
    {
        // If no longer strafing, slowly reduce to 0
        currentStrafeForce *= 0.96f;
    }
}

void SmoothStrafing::OnGameLoad()
{
    MemUtils::PatchAssembly(0x62BBA62, OnStrafeForceApply);
    MemUtils::PatchAssembly(0x629D33B, OnThrusterForceApply);

    INI_Reader ini;
    if (!ini.open("freelancer.ini", false) || !ini.find_header("Data"))
    {
        throw ModuleLoadException("Could not load freelancer.ini or it was malformed");
    }

    std::list<std::string> files;
    while (ini.read_value())
    {
        if (ini.is_value("ships"))
        {
            files.emplace_back(ini.get_value_string());
        }
    }

    ini.close();

    for (const auto& file : files)
    {
        if (!ini.open((std::string("../DATA/") + file).c_str(), false))
        {
            continue;
        }

        ReadIniFile(ini);
        ini.close();
    }
}

void SmoothStrafing::OnLaunch(uint client, struct FLPACKET_LAUNCH& launch)
{
    currentStrafeForce = 0.f;
    totalTimeBeenStrafing = 0.f;
    deltaTime = 0.f;
}

SmoothStrafing::SmoothStrafing() { AssertRunningOnClient; }

std::string_view SmoothStrafing::GetModuleName() { return moduleName; }

SETUP_MODULE(SmoothStrafing);
