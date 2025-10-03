#include "PCH.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include "SmoothStrafing.hpp"

#include <KeyManager.hpp>
#include <ShipManipulator.hpp>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

bool SmoothStrafing::OnStrafeLeftKey(KeyState keyState)
{
    if (keyState == KeyState::Pressed && currentStrafeDir == StrafeDir::None)
    {
        currentStrafeDir = StrafeDir::Left;
    }

    if (keyState == KeyState::Released)
    {
        currentStrafeDir = StrafeDir::None;
    }

    return true;
}

bool SmoothStrafing::OnStrafeRightKey(KeyState keyState)
{
    if (keyState == KeyState::Pressed && currentStrafeDir == StrafeDir::None)
    {
        currentStrafeDir = StrafeDir::Right;
    }

    if (keyState == KeyState::Released)
    {
        currentStrafeDir = StrafeDir::None;
    }

    return true;
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
        float maxStrafeForce = 0.f;
        float strafeAcceleration = 0.f;
        while (ini.read_value())
        {
            if (ini.is_value("nickname"))
            {
                nickname = ini.get_value_string();
            }
            else if (ini.is_value("strafe_force_max"))
            {
                maxStrafeForce = ini.get_value_float(0);
            }
            else if (ini.is_value("strafe_force_acceleration"))
            {
                // Divide force by 60 to create a per second value
                strafeAcceleration = ini.get_value_float(0) / 60;
            }
        }

        if (nickname.empty() || !maxStrafeForce || !strafeAcceleration)
        {
            continue;
        }

        shipStrafeForces[CreateID(nickname.c_str())] = { maxStrafeForce, strafeAcceleration };
    }
}

void SmoothStrafing::OnGameLoad()
{
    auto manager = Fluf::GetKeyManager();

    manager->RegisterKey(
        this, "USER_MANEUVER_SLIDE_EVADE_LEFT", Key::USER_MANEUVER_SLIDE_EVADE_LEFT, static_cast<KeyFunc>(&SmoothStrafing::OnStrafeLeftKey), true);
    manager->RegisterKey(
        this, "USER_MANEUVER_SLIDE_EVADE_RIGHT", Key::USER_MANEUVER_SLIDE_EVADE_RIGHT, static_cast<KeyFunc>(&SmoothStrafing::OnStrafeRightKey), true);

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

    for (auto file : files)
    {
        if (!ini.open(file.c_str(), false))
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
    currentStrafeDir = StrafeDir::None;
}

void SmoothStrafing::OnPhysicsUpdate(uint system, float delta)
{
    const auto player = Fluf::GetPlayerCShip();

    if (!player)
    {
        currentStrafeForce = 0.f;
        return;
    }

    if (currentStrafeDir == StrafeDir::None)
    {
        return;
    }

    const auto strafe = shipStrafeForces.find(player->archetype->archId);
    if (strafe == shipStrafeForces.end())
    {
        return;
    }

    Vector dir{};
    auto orientation = player->get_orientation();
    if (currentStrafeDir == StrafeDir::Left)
    {
        currentStrafeForce -= strafe->second.second;
        dir = { orientation[0][1], orientation[1][1], orientation[2][1] };
    }
    else if (currentStrafeDir == StrafeDir::Right)
    {
        currentStrafeForce += strafe->second.second;
        dir = { orientation[0][2], orientation[1][2], orientation[2][2] };
    }

    currentStrafeForce = std::clamp(currentStrafeForce, -strafe->second.first, strafe->second.first);

    dir *= currentStrafeForce;

    const Vector newVelocity = ShipManipulator::GetVelocity(player) + dir;
    ShipManipulator::SetVelocity(player, newVelocity);
}

SmoothStrafing::SmoothStrafing() { AssertRunningOnClient; }

std::string_view SmoothStrafing::GetModuleName() { return moduleName; }

SETUP_MODULE(SmoothStrafing);
