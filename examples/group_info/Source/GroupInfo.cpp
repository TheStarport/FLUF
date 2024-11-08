#include "PCH.hpp"

#include "GroupInfo.hpp"

#include "FLCore/Common/CEquip/CAttachedEquip/CEShield.hpp"
#include "FLCore/Common/CObjects/CSimple/CEqObj/CShip.hpp"

#include "FLUF/Include/Fluf.hpp"
#include "FLUF/Include/FlufModule.hpp"

#include "FLUF.UI/Include/FLUF.UI.hpp"
#include "FLUF.UI/Include/Rml/RmlContext.hpp"
#include "Utils/StringUtils.hpp"

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void GroupInfo::OnFixedUpdate(const double delta)
{
    timer -= delta;
    if (timer > 0)
    {
        return;
    }

    timer = 1.;

    const auto playerShip = Fluf::GetCShip();
    if (!playerShip) // || !playerShip->playerGroup)
    {
        if (!members.empty())
        {
            members.clear();
            memberDataModel.DirtyVariable("members");
        }

        return;
    }

    const auto groupId = playerShip->groupId;

    std::set<uint> ids;

    auto next = CObject::FindFirst(CObject::CSHIP_OBJECT);
    do
    {
        const auto ship = dynamic_cast<CShip*>(next);
        next = CObject::FindNext();

        if (!ship || ship == playerShip) // || ship->groupId != groupId)
        {
            continue;
        }

        ids.emplace(ship->id);

        // We have found another ship that shares our group

        std::string name = StringUtils::wstos(std::wstring_view(reinterpret_cast<const wchar_t*>(ship->get_pilot_name())));
        const float distance = std::abs(glm::distance<3, float, glm::packed_highp>(ship->position, playerShip->position));
        const auto health = ship->hitPoints / ship->get_max_hit_pts();
        const auto shield = reinterpret_cast<CEShield*>(ship->equipManager.FindFirst(static_cast<uint>(EquipmentClass::Shield)));
        float shieldHealth = 0.f;
        if (shield && shield->maxShieldHitPoints > 0.f && shield->IsFunctioning())
        {
            shieldHealth = (shield->currentShieldHitPoints - (shield->maxShieldHitPoints * shield->offlineThreshold)) /
                           (shield->maxShieldHitPoints * (1.f - shield->offlineThreshold));
        }

        members[ship->id] = { name, ship->archetype->archId, distance, health, shieldHealth };
    }
    while (next);

    // Remove any ships that are no longer in play and where their timer has expired
    for (auto iter = members.begin(); iter != members.end();)
    {
        if (!ids.contains(iter->first))
        {
            iter->second.deathTimer -= delta;
            if (iter->second.deathTimer <= 0)
            {
                iter = members.erase(iter);
            }
        }
        else
        {
            ++iter;
        }
    }

    memberDataModel.DirtyVariable("members");
}

void GroupInfo::OnGameLoad()
{
    const auto context = flufUi->GetRmlContext();

    if (!context.has_value())
    {
        Fluf::Log(LogLevel::Error,
                  "Rml context was not found. Ensure that FLUF.UI is loaded, "
                  "has a higher priority than group_info, and is set to RML mode.");
        return;
    }

    auto ctor = context->CreateDataModel("members");

    if (auto memberHandle = ctor.RegisterStruct<GroupMember>())
    {
        memberHandle.RegisterMember("name", &GroupMember::name);
        memberHandle.RegisterMember("distance", &GroupMember::distance);
        memberHandle.RegisterMember("health", &GroupMember::health);
        memberHandle.RegisterMember("shield", &GroupMember::shield);
        memberHandle.RegisterMember("shipArch", &GroupMember::shipArch);
    }

    ctor.RegisterArray<decltype(members)>();
    ctor.Bind("members", &members);

    memberDataModel = ctor.GetModelHandle();

    document = context->LoadDocument("local://INTERFACE/RML/group_info.rml");
    if (!document)
    {
        Fluf::Log(LogLevel::Error, "INTERFACE/Rml/group_info.rml was not found. Crashes are likely.");
        return;
    }

    document->Show();
}

GroupInfo::GroupInfo()
{
    const auto weakPtr = Fluf::GetModule(FlufUi::moduleName);
    if (weakPtr.expired())
    {
        Fluf::Log(LogLevel::Error, "Group info was loaded, but FLUF UI was not loaded. Crashes are likely.");
        return;
    }

    const auto module = std::static_pointer_cast<FlufUi>(weakPtr.lock());
    Fluf::Log(LogLevel::Trace, std::format("{}", DWORD(module.get())));
    if (module->GetConfig()->uiMode != UiMode::Rml)
    {
        Fluf::Log(LogLevel::Error, "Group info was loaded, but FLUF UI's ui mode was not set to RML");
        return;
    }

    flufUi = module;
}

std::string_view GroupInfo::GetModuleName() { return moduleName; }

SETUP_MODULE(GroupInfo);
