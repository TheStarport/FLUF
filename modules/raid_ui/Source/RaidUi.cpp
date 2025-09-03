#include "PCH.hpp"

#include "RaidUi.hpp"

#include "FLCore/Common/CEquip/CAttachedEquip/CEShield.hpp"
#include "FLCore/Common/CObjects/CSimple/CEqObj/CShip.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include "FLCore/Common/Archetype/Root/EqObj/Ship.hpp"
#include "FLCore/Common/Globals.hpp"
#include "FLCore/FLCoreServer.h"
#include "FLUF.UI.hpp"
#include "ImGui/IconFontAwesome6.hpp"
#include "ImGui/ImGuiHelper.hpp"
#include "ImGui/ImGuiInterface.hpp"
#include "KeyManager.hpp"
#include "Utils/StringUtils.hpp"

#include <Exceptions.hpp>
#include <imgui_internal.h>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void RaidUi::OnFixedUpdate(const double delta)
{
    timer -= delta;
    if (timer > 0)
    {
        return;
    }

    timer = customisationSettings->refreshRate;

    static bool lastDebugState = customisationSettings->debugMode;

    const auto playerShip = Fluf::GetPlayerCShip();
    if (!playerShip || (!customisationSettings->debugMode && (!playerShip->playerGroup || lastDebugState)))
    {
        lastDebugState = false;
        if (!members.empty())
        {
            members.clear();
        }

        return;
    }

    if (customisationSettings->debugMode)
    {
        if (!members.empty())
        {
            return;
        }

        lastDebugState = true;
        auto shipClassIter = shipClassImageMap.shipClassImageMap.begin();

        if (shipClassIter == shipClassImageMap.shipClassImageMap.end())
        {
            return;
        }

        for (char i = '0'; i < '0' + 32; ++i)
        {
            // clang-format off
            members[(int)i] = {
                .name = std::string(23,i),
                .shipClass = shipClassIter->first,
                .distance = 10.0f * (int)i,
                .healthPercent = 0.8f,
                .healthCurrent = 800.0f,
                .healthMax = 1000.0f,
                .shieldPercent = 0.5f,
                .shieldCurrent = 500.0f,
                .shieldMax = 1000.0f,
                .shieldRecharging = false,
                .shieldRechargeStart = 0.0f,
                .shieldRechargeEnd = 0.0f,
            };
            // clang-format on

            if (++shipClassIter == shipClassImageMap.shipClassImageMap.end())
            {
                shipClassIter = shipClassImageMap.shipClassImageMap.begin();
            }
        }
        return;
    }

    std::set<uint> ids;
    auto next = CObject::FindFirst(CObject::CSHIP_OBJECT);
    do
    {
        const auto ship = dynamic_cast<CShip*>(next);
        next = CObject::FindNext();

        if (!ship || ship == playerShip || playerShip->playerGroup != ship->playerGroup)
        {
            continue;
        }

        ids.emplace(ship->id);

        // We have found another ship that shares our group

        auto existingMember = members.find(ship->id);

        FmtStr name1, name2;
        std::array<wchar_t, 24> characterName{};
        auto namePtr = reinterpret_cast<const unsigned short*>(characterName.data());
        Reputation::Vibe::GetName(ship->repVibe, name1, name2, namePtr);

        if (!*namePtr)
        {
            continue;
        }

        const std::string name = StringUtils::wstos(std::wstring_view(reinterpret_cast<const wchar_t*>(namePtr)));
        const float distance = std::abs(glm::distance<3, float, glm::packed_highp>(ship->position, playerShip->position));
        const auto health = ship->hitPoints / ship->get_max_hit_pts();
        const auto shield = reinterpret_cast<CEShield*>(ship->equipManager.FindFirst(static_cast<uint>(EquipmentClass::Shield)));
        // ReSharper disable CppDFAUnreadVariable
        float shieldHealth = 0.f, shieldCurrent = 0.f, shieldMax = 0.f;
        bool shieldRecharging = false;
        float timeUntilRecharge = 0.0f, existingShieldRechargeStart = existingMember == members.end() ? 0.f : existingMember->second.shieldRechargeStart;
        if (shield && shield->maxShieldHitPoints > 0.f)
        {
            // ReSharper disable thrice CppDFAUnusedValue
            shieldHealth = (shield->currShieldHitPoints - shield->maxShieldHitPoints * shield->offlineThreshold) /
                           (shield->maxShieldHitPoints * (1.f - shield->offlineThreshold));
            shieldCurrent = shield->currShieldHitPoints;
            shieldMax = shield->maxShieldHitPoints;

            if (!shield->IsFunctioning())
            {
                shieldRecharging = true;
                timeUntilRecharge = static_cast<float>(shield->rebuildTimestamp);
            }
            else
            {
                existingShieldRechargeStart = static_cast<float>(Timing::GetGlobalTime());
            }
        }
        // ReSharper restore CppDFAUnreadVariable

        // clang-format off
        members[ship->id] = {
            .name = name,
            .shipClass = ship->shiparch()->shipClass,
            .distance = distance,
            .healthPercent = health,
            .healthCurrent = ship->hitPoints,
            .healthMax = ship->get_max_hit_pts(),
            .shieldPercent = shieldHealth,
            .shieldCurrent = shieldCurrent,
            .shieldMax = shieldMax,
            .shieldRecharging = shieldRecharging,
            .shieldRechargeStart = existingShieldRechargeStart,
            .shieldRechargeEnd = timeUntilRecharge,
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

void RaidUi::OnGameLoad()
{
    if (flufUi->GetConfig()->uiMode != UiMode::ImGui)
    {
        return;
    }

    flufUi->GetImGuiInterface()->RegisterImGuiModule(this);
    flufUi->GetImGuiInterface()->RegisterOptionsMenu(this, static_cast<RegisterOptionsFunc>(&RaidUi::RenderRaidUiOptions));
}

void RaidUi::RadialProgressBar(const std::string& label, const float progress, const ImVec2& size, const ImVec4& color, ImVec2 center)
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

void RaidUi::Render()
{
    if (!customisationSettings->enable || members.empty())
    {
        return;
    }

    const auto interface = flufUi->GetImGuiInterface();

    auto windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;
    if (imguiPanelLocked)
    {
        windowFlags |= ImGuiWindowFlags_NoMove;
    }

    Fluf::GetPlayerIObjRW();

    ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4());
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4());
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4());

    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding, ImVec2(1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20.f, 2.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);

    ImGui::SetNextWindowSize(ImVec2(400.f, 600.f), ImGuiCond_Always);
    ImGui::Begin("GroupInfo", nullptr, windowFlags);

    const auto font = interface->GetImGuiFont("Saira", 24);
    assert(font);

    ImGui::PushFont(font);

    const bool isWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_Stationary) && !customisationSettings->hideLockSymbol;
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

    ImGui::BeginTable("GroupInfoTableContainer", 2);
    ImGui::TableSetupColumn("GroupInfoTable1", ImGuiTableColumnFlags_WidthFixed, 200.f);
    ImGui::TableSetupColumn("GroupInfoTable2", ImGuiTableColumnFlags_WidthFixed, 200.f);
    ImGui::TableNextColumn();

    int column = 0;
    int row = 0;
    int totalMemberProcessed = 0;
    bool skip = false;
    for (auto& member : members)
    {
        ++totalMemberProcessed;
        if (!skip && (row == 0 || row == 6))
        {
            row = 0;
            ImGui::PushID(column++);
            ImGui::BeginTable("GroupInfoTable##", 2, ImGuiTableFlags_SizingFixedFit);
        }

        skip = false;

        ImTextureID texture = 0;
        if (auto cls = shipClassImageMap.shipClassImageMap.find(member.second.shipClass); cls != shipClassImageMap.shipClassImageMap.end())
        {
            uint height;
            uint width;
            texture = interface->LoadTexture(std::format("{}/{}", shipClassImageMap.folderName, cls->second), width, height);
            assert(texture);
            if (!texture)
            {
                MessageBoxA(nullptr, std::format("Unable to load texture: {}/{}", shipClassImageMap.folderName, cls->second).c_str(), "Texture Missing", MB_OK);
                std::exit(1);
            }
        }
        else
        {
            skip = true;
            continue;
        }

        ImGui::BeginGroup();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        constexpr ImVec2 imageSize = { 40.f, 40.f };
        ImGui::Dummy({ 0.f, 20.f });
        ImGui::Image(texture, imageSize);

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

        int popCount = 2;
        ImGui::PushStyleColor(ImGuiCol_Text, reinterpret_cast<ImVec4&>(customisationSettings->nameColor));
        ImGui::Text(member.second.name.c_str());
        ImGui::PushStyleColor(ImGuiCol_Text, reinterpret_cast<ImVec4&>(customisationSettings->progressBarTextColor));

        if (member.second.healthCurrent > 0.f)
        {
            popCount++;
            std::string healthFormat = std::format("{:.0f} / {:.0f}", member.second.healthCurrent, member.second.healthMax);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, reinterpret_cast<ImVec4&>(customisationSettings->healthBarColor));
            ImGui::ProgressBar(member.second.healthPercent, ImVec2(100.f, 0.f), healthFormat.c_str());
            if (member.second.shieldMax != 0)
            {
                popCount++;
                if (member.second.shieldRecharging)
                {
                    const auto timing = static_cast<float>(Timing::GetGlobalTime());
                    const auto timeUntilRebuild = member.second.shieldRechargeEnd - member.second.shieldRechargeStart;
                    const auto rebuildPercentage = (member.second.shieldRechargeEnd - timing) / timeUntilRebuild;

                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, reinterpret_cast<ImVec4&>(customisationSettings->shieldBarColor));
                    std::string shieldFormat = std::format("{:.0f}s", std::ceilf(member.second.shieldRechargeEnd - timing));
                    ImGui::ProgressBar(1 - rebuildPercentage, ImVec2(100.f, 0.f), shieldFormat.c_str());
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, reinterpret_cast<ImVec4&>(customisationSettings->shieldBarRechargingColor));
                    std::string shieldFormat = std::format("{:.0f} / {:.0f}", member.second.shieldCurrent, member.second.shieldMax);
                    ImGui::ProgressBar(member.second.shieldPercent, ImVec2(100.f, 0.f), shieldFormat.c_str());
                }
            }
        }

        ImGui::PopStyleColor(popCount);
        ImGui::EndGroup();

        if (ImGui::IsItemClicked())
        {
            const auto foundObject = dynamic_cast<CShip*>(CObject::Find(member.first, CObject::CSHIP_OBJECT));
            if (foundObject)
            {
                foundObject->Release();
            }

            if (auto* playerShip = Fluf::GetPlayerIObjRW(); playerShip && foundObject)
            {
                struct SetTargetData
                {
                        uint unk = 0x1C;
                        uint null1 = 0;
                        uint null2 = 0;
                        uint null3 = 0;
                        uint targetId = 0;
                        uint subId = 0;
                };
                using SetPlayerTarget = bool(__thiscall*)(IObjRW * player, SetTargetData * data);
                static auto setPlayerTarget = reinterpret_cast<SetPlayerTarget>(0x544D70);

                // Has Shift
                if (ImGui::GetIO().KeyShift)
                {
                    if (const auto target = foundObject->get_target())
                    {
                        const auto simple = dynamic_cast<CSimple*>(target->cobject());

                        SetTargetData data;
                        data.targetId = target->get_id();
                        setPlayerTarget(playerShip, &data);
                    }
                }
                else
                {
                    SetTargetData data;
                    data.targetId = foundObject->get_id();
                    setPlayerTarget(playerShip, &data);
                }
            }
        }

        if (row == 5 || totalMemberProcessed == members.size())
        {
            ImGui::EndTable();
            ImGui::PopID();

            if (row == 5)
            {
                ImGui::TableNextColumn();
            }
        }

        row++;
    }

    ImGui::EndTable();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(3);
    ImGui::PopFont();
    ImGui::End();
}

bool RaidUi::OnTogglePanelKeyCommand()
{
    customisationSettings->enable = !customisationSettings->enable;
    return true;
}

void RaidUi::RenderRaidUiOptions(const bool saveRequested)
{
    static auto wipSettings = rfl::Box<CustomisationSettings>::make(*customisationSettings);

    ImGui::Checkbox("Enable", &wipSettings->enable);
    ImGui::Checkbox("Hide Lock Symbol", &wipSettings->hideLockSymbol);
    ImGui::SameLine();
    ImGuiHelper::HelpMarker("When the cursor is stationary over the raid ui for 0.25s a lock will appear.\n"
                            "Ticking this box will prevent it from appearing at all.");

    ImGui::Checkbox("Debug Mode", &wipSettings->debugMode);
    ImGui::SameLine();
    ImGuiHelper::HelpMarker("Creates a mock preview of a full group using all available ship classes");

    ImGui::SliderFloat("Refresh Rate", &wipSettings->refreshRate, 0.f, 2.f, "%.2f");
    ImGui::SameLine();
    ImGuiHelper::HelpMarker("The refresh rate determines how long to wait between updating the UI. "
                            "There may performance considerations with setting it to 0 on some machines.");

    int counter = 0;
    const auto itemSpacing = ImGui::GetStyle().ItemSpacing;
    constexpr auto resetText = "Reset##";
#define COLOR_PICKER(lbl, ptr)                                                                                                                   \
    ImGui::ColorEdit4(lbl, wipSettings->ptr.data());                                                                                             \
    ImGui::SameLine();                                                                                                                           \
    ImGui::PushID(counter++);                                                                                                                    \
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - (ImGui::CalcTextSize(resetText).x + itemSpacing.x * 2.0f)); \
    if (ImGui::Button(resetText))                                                                                                                \
    {                                                                                                                                            \
        wipSettings->ptr = customisationSettings->ptr;                                                                                           \
    }                                                                                                                                            \
    ImGui::PopID();

    COLOR_PICKER("Name Colour", nameColor);
    COLOR_PICKER("Progress Bar Text Colour", progressBarTextColor);
    COLOR_PICKER("Health Bar Colour", healthBarColor);
    COLOR_PICKER("Shield Bar Colour", shieldBarColor);
    COLOR_PICKER("Shield Bar Recharging Colour", shieldBarRechargingColor);
#undef COLOR_PICKER

    if (saveRequested)
    {
        customisationSettings = rfl::Box<CustomisationSettings>::make(*wipSettings);
        ConfigHelper<CustomisationSettings, customisationFile>::Save(*customisationSettings);
    }
}

RaidUi::RaidUi()
{
    AssertRunningOnClient;

    const auto weakPtr = Fluf::GetModule(FlufUi::moduleName);
    if (weakPtr.expired())
    {
        throw ModuleLoadException("RaidUi was loaded, but FLUF UI was not loaded.");
    }

    const auto module = std::static_pointer_cast<FlufUi>(weakPtr.lock());
    flufUi = module;

    if (module->GetConfig()->uiMode != UiMode::ImGui)
    {
        throw ModuleLoadException("RaidUi was loaded, but FLUF UI's ui mode was not set to 'ImGui'");
    }

    static constexpr char configFile[] = "modules/config/raid_ui.yml";
    if (const auto classMap = ConfigHelper<ShipClassImageMap, configFile>::Load(); !classMap.has_value())
    {
        if (MessageBoxA(nullptr,
                        std::format("There were issues processing the config file at {}.\n\nPress 'OK' to generate a new config or cancel to "
                                    "terminate the application.",
                                    configFile)
                            .c_str(),
                        "Config not found",
                        MB_OKCANCEL) == IDCANCEL)
        {
            std::exit(1);
        }

        ConfigHelper<ShipClassImageMap, configFile>::Save(shipClassImageMap);
    }
    else
    {
        shipClassImageMap = classMap.value();
    }

    Fluf::GetKeyManager()->RegisterKey(
        this, "FLUF_TOGGLE_RAID_UI", Key::NN_TOGGLE_OPEN_OVERRIDE, static_cast<KeyFunc>(&RaidUi::OnTogglePanelKeyCommand), false);

    auto customisations = ConfigHelper<CustomisationSettings, customisationFile>::Load(true, false);
    customisationSettings = customisations.has_value() ? rfl::Box<CustomisationSettings>::make(*customisations) : rfl::Box<CustomisationSettings>::make();
}

RaidUi::~RaidUi()
{
    if (flufUi->GetConfig()->uiMode == UiMode::ImGui)
    {
        flufUi->GetImGuiInterface()->UnregisterImGuiModule(this);
    }
}

std::string_view RaidUi::GetModuleName() { return moduleName; }

SETUP_MODULE(RaidUi);
