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

/*
std::pair<bool, float> GetEngineStateAndDrag(CShip* ship)
{
    auto& eqm = ship->equipManager;
    CEquipTraverser traverser{ static_cast<int>(EquipmentClass::Engine) };
    auto eq = static_cast<CEEngine*>(eqm.Traverse(traverser));
    float linearDrag = ship->shiparch()->linearDrag;
    bool inEngineKill = false;
    while (eq)
    {
        if (!eq->IsFunctioning())
        {
            inEngineKill = true;
        }

        linearDrag += eq->EngineArch()->linearDrag;
        eq = static_cast<CEEngine*>(eqm.Traverse(traverser));
    }

    return { inEngineKill, linearDrag };
}
*/

/*
bool SmoothStrafing::OnStrafeLeftKey(KeyState keyState)
{
    auto ship = Fluf::GetPlayerCShip();
    if (!ship)
    {
        return false;
    }

    if (keyState == KeyState::Pressed)
    {
        auto [inEngineKill, linearDrag] = GetEngineStateAndDrag(ship);
        wasInEngineKill = inEngineKill;
        currentStrafeDir = StrafeDir::Left;
    }
    else if (keyState == KeyState::Released && currentStrafeDir == StrafeDir::Left)
    {
        currentStrafeDir = StrafeDir::None;

        if (wasInEngineKill)
        {
            ship->get_behavior_interface()->update_current_behavior_engage_engine(false);
        }
    }

    return true;
}

bool SmoothStrafing::OnStrafeRightKey(KeyState keyState)
{
    auto ship = Fluf::GetPlayerCShip();
    if (!ship)
    {
        return false;
    }

    if (keyState == KeyState::Pressed)
    {
        auto [inEngineKill, linearDrag] = GetEngineStateAndDrag(ship);
        wasInEngineKill = inEngineKill;
        currentStrafeDir = StrafeDir::Right;
    }
    else if (keyState == KeyState::Released && currentStrafeDir == StrafeDir::Right)
    {
        currentStrafeDir = StrafeDir::None;

        if (wasInEngineKill)
        {
            ship->get_behavior_interface()->update_current_behavior_engage_engine(false);
        }
    }

    return true;
}
*/

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
    auto strafeForceAcceleration = shipStrafeForces.find(ship->shiparch()->archId);
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

void SmoothStrafing::BeforePhysicsUpdate(uint system, float delta) { deltaTime = delta; }

void SmoothStrafing::OnGameLoad()
{
    MemUtils::PatchAssembly(0x62BBA62, OnStrafeForceApply);

    /*auto manager = Fluf::GetKeyManager();

    manager->RegisterKey(
        this, "USER_MANEUVER_SLIDE_EVADE_LEFT", Key::USER_MANEUVER_SLIDE_EVADE_LEFT, static_cast<KeyFunc>(&SmoothStrafing::OnStrafeLeftKey), true);
    manager->RegisterKey(
        this, "USER_MANEUVER_SLIDE_EVADE_RIGHT", Key::USER_MANEUVER_SLIDE_EVADE_RIGHT, static_cast<KeyFunc>(&SmoothStrafing::OnStrafeRightKey), true);*/

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

void SmoothStrafing::OnLaunch(uint client, struct FLPACKET_LAUNCH& launch) { currentStrafePercentage = 0.f; }

/*void SmoothStrafing::OnPhysicsUpdate(uint system, float delta)
{
    const auto player = Fluf::GetPlayerCShip();

    if (!player)
    {
        currentStrafePercentage = 0.f;
        return;
    }

    const auto strafe = shipStrafeForces.find(player->archetype->archId);
    if (strafe == shipStrafeForces.end())
    {
        return;
    }

    auto orientation = player->get_orientation();
    auto [inEngineKill, linearDrag] = GetEngineStateAndDrag(player);
    if (const float strafeForce = strafe->second.second * delta; currentStrafeDir == StrafeDir::Left)
    {
        if (inEngineKill)
        {
            player->get_behavior_interface()->update_current_behavior_engage_engine(true);
        }

        if (currentStrafeForce > 0.f)
        {
            currentStrafeForce -= strafeForce;
        }

        currentStrafeForce -= strafeForce;
    }
    else if (currentStrafeDir == StrafeDir::Right)
    {
        if (inEngineKill)
        {
            player->get_behavior_interface()->update_current_behavior_engage_engine(true);
        }

        if (currentStrafeForce < 0.f)
        {
            currentStrafeForce += strafeForce;
        }

        currentStrafeForce += strafeForce;
    }
    else
    {
        if (inEngineKill)
        {
            return;
        }

        if (currentStrafeForce > 0.f)
        {
            currentStrafeForce -= strafe->second.second * delta;
        }
        else if (currentStrafeForce < 0.f)
        {
            currentStrafeForce += strafe->second.second * delta;
        }
        else
        {
            return;
        }
    }

    Vector dir = { orientation[0][0], orientation[1][0], orientation[2][0] };
    currentStrafeForce = std::clamp(currentStrafeForce, -strafe->second.first, strafe->second.first);

    printf("Applying Force: %f\n", currentStrafeForce / linearDrag);
    dir *= currentStrafeForce / linearDrag * delta;

    const Vector newVelocity = ShipManipulator::GetVelocity(player) + dir;
    ShipManipulator::SetVelocity(player, newVelocity);
}*/

SmoothStrafing::SmoothStrafing() { AssertRunningOnClient; }

std::string_view SmoothStrafing::GetModuleName() { return moduleName; }

SETUP_MODULE(SmoothStrafing);
