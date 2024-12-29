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
        float shieldHealth = 0.f, shieldCurrent = 0.f, shieldMax = 0.f;
        if (shield && shield->maxShieldHitPoints > 0.f)
        {
            // ReSharper disable thrice CppDFAUnusedValue
            shieldHealth = (shield->currentShieldHitPoints - shield->maxShieldHitPoints * shield->offlineThreshold) /
                           (shield->maxShieldHitPoints * (1.f - shield->offlineThreshold));
            shieldCurrent = shield->currentShieldHitPoints;
            shieldMax = shield->maxShieldHitPoints;
        }

        // clang-format off
        members[ship->id] = {
            .name = name,
            .shipArch = ship->archetype->archId,
            .distance = distance,
            .healthPercent = health,
            .healthCurrent = ship->hitPoints,
            .healthMax = ship->get_max_hit_pts(),
            .shieldPercent = shieldHealth,
            .shieldCurrent = shieldCurrent,
            .shieldMax = shieldMax
        };
        // clang-format on
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
}

void GroupInfo::OnGameLoad()
{
    if (flufUi->GetConfig()->uiMode != UiMode::ImGui)
    {
        return;
    }

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

    auto windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;
    if (imguiPanelLocked)
    {
        windowFlags |= ImGuiWindowFlags_NoMove;
    }

    ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4());
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4());
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4());

    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20.f, 2.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);

    ImGui::SetNextWindowSize(ImVec2(200.f, 600.f), ImGuiCond_Always);
    ImGui::Begin("GroupInfo", nullptr, windowFlags);

    const auto font = interface->GetImGuiFont("Saira", 24);
    assert(font);

    ImGui::PushFont(font);

    const bool isWindowHovered = ImGui::IsWindowHovered();
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

    ImGui::BeginTable("GroupInfoTable", 4, ImGuiTableFlags_SizingFixedFit);

    constexpr ImVec2 imageSize = { 40.f, 40.f };

    for (auto& member : members)
    {
        ImGui::BeginGroup();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Dummy({ 0.f, 20.f });

        uint width, height;
        auto texture1 = interface->LoadTexture("../DATA/INTERFACE/IMAGES/SYMBOLS/Circle.png", width, height);
        assert(texture1);
        ImGui::Image(reinterpret_cast<ImTextureID>(texture1), imageSize);

        constexpr auto writeDistance = [imageSize](const char* text)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (imageSize.x - ImGui::CalcTextSize(text).x) * .5f);
            ImGui::Text(text);
        };

        if (member.second.deathTimer > 0.0)
        {
            const char* text = "DEAD";
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
            writeDistance(text);
            ImGui::PopStyleColor();
        }
        else if (member.second.distance >= 100'000.f)
        {
            const char* text = "FAR";
            writeDistance(text);
        }
        else if (member.second.distance >= 1'000.f)
        {
            std::string distance = std::format("{:.2f}K", member.second.distance / 1'000.f);
            writeDistance(distance.c_str());
        }
        else
        {
            std::string distance = std::format("{:.0f}m", member.second.distance);
            writeDistance(distance.c_str());
        }

        ImGui::TableNextColumn();

        int popCount = 1;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.35f, 1.f, 1.f));
        ImGui::Text(member.second.name.c_str());

        if (member.second.healthCurrent > 0.f)
        {
            popCount += 2;
            std::string healthFormat = std::format("{:.0f} / {:.0f}", member.second.healthCurrent, member.second.healthMax);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.47f, 0.16f, 0.2f, 1.f));
            ImGui::ProgressBar(member.second.healthPercent, ImVec2(100.f, 0.f), healthFormat.c_str());
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.243f, 0.24f, 0.7f, 1.f));
            if (member.second.shieldMax != 0)
            {
                std::string shieldFormat = std::format("{:.0f} / {:.0f}", member.second.shieldCurrent, member.second.shieldMax);
                ImGui::ProgressBar(member.second.shieldPercent, ImVec2(100.f, 0.f), shieldFormat.c_str());
            }
        }

        ImGui::PopStyleColor(popCount);
        ImGui::EndGroup();
    }

    ImGui::EndTable();

    ImGui::PopStyleVar(3);
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
