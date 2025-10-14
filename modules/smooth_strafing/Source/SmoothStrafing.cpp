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
        float strafeVerticalMultiplier = 0.1f;
        uint leftFuse = 0;
        uint rightFuse = 0;
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
            else if (ini.is_value("strafe_vertical_multiplier"))
            {
                strafeVerticalMultiplier = ini.get_value_float(0);
            }
            else if (ini.is_value("strafe_fuse"))
            {
                leftFuse = CreateID(ini.get_value_string(0));
                rightFuse = CreateID(ini.get_value_string(1));
            }
        }

        if (nickname.empty() || strafeAcceleration == 0.f)
        {
            continue;
        }

        shipStrafeForces[CreateID(nickname.c_str())] = {
            .acceleration = strafeAcceleration,
            .leftFuse = leftFuse,
            .rightFuse = rightFuse,
            .verticalMultiplier = strafeVerticalMultiplier //
        };
    }
}

void ToggleShipFuse(const CShip* ship, const uint id, const bool on)
{
    const auto srv = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll"));

    if (!srv || !id)
    {
        return;
    }

    const auto inspect = reinterpret_cast<bool (*)(const uint&, Ship*&, StarSystem*& starSystem)>(srv + 0x206C0);
    Ship* iObj;
    StarSystem* starSystem;
    inspect(ship->id, iObj, starSystem);

    if (on)
    {
        iObj->light_fuse(0, id, 0, 0.f, 0.f);
    }
    else
    {
        iObj->unlight_fuse(id, 0, 0.f);
    }
}

float* __fastcall SmoothStrafing::GetStrafeForce(CShip* ship, CStrafeEngine* engine)
{
    static float retValue;

    const auto strafeData = shipStrafeForces.find(ship->shiparch()->archId);
    if (const auto player = Fluf::GetPlayerCShip(); player != ship || strafeData == shipStrafeForces.end())
    {
        retValue = ship->shiparch()->strafeForce;
        return &retValue;
    }

    static auto lastStrafeDirection = StrafeDir::None;
    auto dir = engine->GetStrafe();

    if (dir == StrafeDir::UpLeft || dir == StrafeDir::UpRight)
    {
        if (lastStrafeDirection == StrafeDir::Up)
        {
            dir = dir == StrafeDir::UpLeft ? StrafeDir::Left : StrafeDir::Right;
        }
        else
        {
            dir = StrafeDir::Up;
        }

        engine->SetStrafe(dir);
        ship->strafeDir = dir;
    }
    else if (dir == StrafeDir::DownLeft || dir == StrafeDir::DownRight)
    {
        if (lastStrafeDirection == StrafeDir::Down)
        {
            dir = dir == StrafeDir::DownLeft ? StrafeDir::Left : StrafeDir::Right;
        }
        else
        {
            dir = StrafeDir::Down;
        }

        engine->SetStrafe(dir);
        ship->strafeDir = dir;
    }

    if (dir != lastStrafeDirection)
    {
        if (fuseIsLit)
        {
            if (lastStrafeDirection == StrafeDir::Right)
            {
                ToggleShipFuse(ship, strafeData->second.rightFuse, false);
            }
            else if (lastStrafeDirection == StrafeDir::Left)
            {
                ToggleShipFuse(ship, strafeData->second.leftFuse, false);
            }

            fuseIsLit = false;
        }

        lastStrafeDirection = dir;
        totalTimeBeenStrafing = 0.f;
    }

    if (dir == StrafeDir::None)
    {
        retValue = 0.f;
        return &retValue;
    }

    totalTimeBeenStrafing += deltaTime;
    retValue = std::clamp(strafeData->second.acceleration * totalTimeBeenStrafing, 0.f, ship->shiparch()->strafeForce);

    if (dir == StrafeDir::Right || dir == StrafeDir::Left)
    {
        currentHorizontalStrafeForce = retValue;
        if (std::abs(currentVerticalStrafeForce) > 1.f)
        {
            currentVerticalStrafeForce *= 0.97f;
        }

        if (!fuseIsLit)
        {
            fuseIsLit = true;
            lastFuse = ship->strafeDir == StrafeDir::Right ? strafeData->second.rightFuse : strafeData->second.leftFuse;
            ToggleShipFuse(ship, lastFuse, true);
        }
    }
    else if (dir == StrafeDir::Up || dir == StrafeDir::Down)
    {
        retValue *= strafeData->second.verticalMultiplier;
        currentVerticalStrafeForce = retValue;
        if (std::abs(currentHorizontalStrafeForce) > 1.f)
        {
            currentHorizontalStrafeForce *= 0.97f;
        }
    }
    else
    {
        retValue = 0.f;
    }

    return &retValue;
}

void __declspec(naked) SmoothStrafing::OnStrafeForceApply()
{
    static constexpr DWORD retAddress = 0x62BBA6E;
    __asm
    {
        push edx
        push eax
        mov edx, esi
        call GetStrafeForce
        fld [eax]
        pop eax
        pop edx
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

    retValue = std::clamp(archetype->maxForce - currentHorizontalStrafeForce, 0.f, archetype->maxForce);
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

    const auto player = Fluf::GetPlayerCShip();
    if (!player)
    {
        return;
    }

    if (player->strafeDir == StrafeDir::None)
    {
        // If no longer strafing, slowly reduce to 0
        currentHorizontalStrafeForce *= 0.97f;
        currentVerticalStrafeForce *= 0.97f;
        totalTimeBeenStrafing = std::clamp(totalTimeBeenStrafing - delta, 0.f, 1.f);

        if (fuseIsLit)
        {
            ToggleShipFuse(player, lastFuse, false);
            fuseIsLit = false;
        }

        return;
    }

    // If the player has changed direction in strafe, but is still strafing
    // We want to lower the amount of force, but not feel like they are being
    // forcefully stopped. So we decrease by ~6% a second, vs the normal ~18%
    if (player->strafeDir != StrafeDir::Left && player->strafeDir != StrafeDir::Right)
    {
        currentHorizontalStrafeForce *= 0.99f;
    }
    else if (player->strafeDir != StrafeDir::Down && player->strafeDir != StrafeDir::Up)
    {
        currentVerticalStrafeForce *= 0.99f;
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
    currentHorizontalStrafeForce = 0.f;
    currentVerticalStrafeForce = 0.f;
    totalTimeBeenStrafing = 0.f;
    deltaTime = 0.f;
}

SmoothStrafing::SmoothStrafing() { AssertRunningOnClient; }

std::string_view SmoothStrafing::GetModuleName() { return moduleName; }

SETUP_MODULE(SmoothStrafing);
