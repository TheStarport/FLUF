#include "PCH.hpp"

#include "GroupInfo.hpp"

#include "FLCore/Common/CEquip/CAttachedEquip/CEShield.hpp"
#include "FLCore/Common/CObjects/CSimple/CEqObj/CShip.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include "FLCore/Common/Globals.hpp"
#include "FLUF.UI.hpp"
#include "ImGui/IconFontAwesome6.hpp"
#include "ImGui/ImGuiInterface.hpp"
#include "Rml/RmlContext.hpp"
#include "Utils/StringUtils.hpp"

#include <imgui_internal.h>

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
            if (flufUi->GetConfig()->uiMode == UiMode::Rml)
            {
                memberDataModel.DirtyVariable("members");
            }
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

        const std::string name = StringUtils::wstos(std::wstring_view(reinterpret_cast<const wchar_t*>(ship->get_pilot_name())));
        const float distance = std::abs(glm::distance<3, float, glm::packed_highp>(ship->position, playerShip->position));
        const auto health = ship->hitPoints / ship->get_max_hit_pts();
        const auto shield = reinterpret_cast<CEShield*>(ship->equipManager.FindFirst(static_cast<uint>(EquipmentClass::Shield)));
        float shieldHealth = 0.f;
        if (shield && shield->maxShieldHitPoints > 0.f && shield->IsFunctioning())
        {
            // ReSharper disable once CppDFAUnusedValue
            shieldHealth = (shield->currentShieldHitPoints - shield->maxShieldHitPoints * shield->offlineThreshold) /
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

    if (flufUi->GetConfig()->uiMode == UiMode::Rml)
    {
        memberDataModel.DirtyVariable("members");
    }
}

void GroupInfo::OnGameLoad()
{
    if (flufUi->GetConfig()->uiMode == UiMode::ImGui)
    {
        flufUi->GetImGuiInterface()->RegisterImGuiModule(this);

        INI_Reader ini;
        ini.open("../DATA/SHIPS/shiparch.ini", false);
        while (ini.read_header())
        {
            if (!ini.is_header("ship"))
            {
                continue;
            }

            while (ini.read_value())
            {
                if (ini.is_value("nickname"))
                {
                    std::string nick = ini.get_value_string();
                    const auto path = std::format("../DATA/INTERFACE/IMAGES/SHIPS/{}.png", nick);
                    if (!std::filesystem::exists(path))
                    {
                        Fluf::Log(LogLevel::Warn, std::format("Ship image doesn't exist \"{}\"", nick));
                        continue;
                    }

                    shipImageMap[CreateID(nick.c_str())] = path;
                }
            }
        }

        return;
    }

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

void GroupInfo::RadialProgressBar(const std::string& label, const float progress, const ImVec2& size, const ImVec4& color, ImVec2 center)
{
    const ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return;
    }

    ImGui::BeginGroup();
    ImGui::PushID(label.c_str());

    const float circleRadius = ImMin(size.x, size.y) * 0.5f;
    if (center.x == 0.f && center.y == 0.f)
    {
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        center = ImVec2(pos.x + circleRadius, pos.y + circleRadius);
    }

    const float arcAngle = 2.0f * IM_PI * progress;

    if (progress > 0.0f)
    {
        window->DrawList->PathArcTo(center, circleRadius, -IM_PI * 0.5f, -IM_PI * 0.5f + arcAngle, 32);
        window->DrawList->PathStroke(ImGui::GetColorU32(color), false, 5.0f);
    }

    ImGui::PopID();
    ImGui::EndGroup();
}

void GroupInfo::Render()
{
    if (members.empty())
    {
        return;
    }

    auto interface = flufUi->GetImGuiInterface();

    auto windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar;
    if (imguiPanelLocked)
    {
        windowFlags |= ImGuiWindowFlags_NoMove;
    }

    ImGui::SetNextWindowSize(ImVec2(500.f, 800.f), ImGuiCond_Always);
    ImGui::Begin("GroupInfo", nullptr, windowFlags);

    const auto font = interface->GetImGuiFont("Saira", 24);
    assert(font);

    ImGui::PushFont(font);

    ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4());
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4());
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4());

    bool isWindowHovered = ImGui::IsWindowHovered();
    if (!isWindowHovered)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4());
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4());
    }

    const auto faFont = interface->GetImGuiFont("FA", 24);
    assert(faFont);

    ImGui::PushFont(faFont);
    if (ImGui::Button(imguiPanelLocked ? ICON_FA_LOCK : ICON_FA_LOCK_OPEN))
    {
        imguiPanelLocked = !imguiPanelLocked;
    }

    ImGui::PopFont();

    if (!isWindowHovered)
    {
        ImGui::PopStyleColor(6);
    }

    if (ImGui::BeginTable("GroupInfoTable", 4, ImGuiTableFlags_SizingStretchProp))
    {
        constexpr auto tableFlags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHeaderLabel;
        ImGui::TableSetupColumn("ship1", tableFlags, 105.0f);
        ImGui::TableSetupColumn("ship2", tableFlags, 105.0f);
        ImGui::TableSetupColumn("ship3", tableFlags, 105.0f);
        ImGui::TableSetupColumn("ship4", tableFlags, 105.0f);
        ImGui::TableHeadersRow();

        auto member = members.begin();
        for (int row = 0; row < 2; row++)
        {
            ImGui::TableNextRow();
            for (int column = 0; column < 4 && member != members.end(); column++)
            {
                ImGui::TableSetColumnIndex(column);
                ImGui::Dummy({ 5.f, 1.0f });

                constexpr ImVec2 imageSize = { 100.f, 100.f };

                const auto imageCenter = ImGui::GetCursorScreenPos() + imageSize / 2;
                uint width = 0, height = 0;
                if (auto shipImage = shipImageMap.find(member->second.shipArch); shipImage == shipImageMap.end())
                {
                    ImGui::Text("Image not found");
                }
                else if (const auto texture = interface->LoadTexture(shipImage->second, width, height); texture)
                {
                    ImGui::Image(reinterpret_cast<ImTextureID>(texture), imageSize);
                }
                else
                {
                    ImGui::Text("Failed to load texture");
                }

                RadialProgressBar("ShieldRadial#", member->second.shield, imageSize, ImVec4(.0f, .23f, 1.0f, 1.0f), imageCenter);
                RadialProgressBar("HealthRadial#", member->second.health, imageSize - ImVec2{ 7.f, 7.f }, ImVec4(0.7f, .0f, .0f, 1.0f), imageCenter);
                ImGui::TextWrapped(member->second.name.c_str());
                if (member->second.deathTimer > 0.0)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
                    ImGui::Text("DEAD");
                    ImGui::PopStyleColor();
                }
                else if (member->second.distance >= 100'000.f)
                {
                    ImGui::Text("FAR");
                }
                else if (member->second.distance >= 1'000.f)
                {
                    ImGui::Text("%.2fK", member->second.distance / 1'000.f);
                }
                else
                {
                    ImGui::Text("%.0fm", member->second.distance);
                }

                ++member;
            }
        }

        ImGui::EndTable();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopFont();
    ImGui::End();
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
    flufUi = module;

    if (module->GetConfig()->uiMode == UiMode::None)
    {
        Fluf::Log(LogLevel::Error, "Group info was loaded, but FLUF UI's ui mode was set to 'None'");
        return;
    }
}

GroupInfo::~GroupInfo()
{
    if (flufUi->GetConfig()->uiMode == UiMode::ImGui)
    {
        flufUi->GetImGuiInterface()->UnregisterImGuiModule(this);
    }
}

std::string_view GroupInfo::GetModuleName() { return moduleName; }

SETUP_MODULE(GroupInfo);
