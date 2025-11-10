#include "PCH.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include "Debugger.hpp"

#include <FLUF.UI.hpp>
#include <imgui_internal.h>
#include <magic_enum.hpp>
#include <ImGui/ImGuiInterface.hpp>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

#define ImKvp(key, ...)       \
    ImGui::TableNextColumn(); \
    ImGui::Text(key);         \
    ImGui::TableNextColumn(); \
    ImGui::Text(__VA_ARGS__)

void Debugger::RenderDebugSpaceWindow(Ship* player)
{
    const auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({ viewport->Size.x * 0.05f, viewport->Size.y * 0.05f }, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize({ 500.f, 800.f }, ImGuiCond_Appearing);
    if (!ImGui::Begin("Debug Window", &renderDebugSpaceWindow, ImGuiWindowFlags_NoBackground))
    {
        ImGui::End();
    }

    ImGui::PushStyleVarY(ImGuiStyleVar_CellPadding, 1.f);
    ImGui::PushStyleColor(ImGuiCol_Text, 0xFFFFFFFF);
    ImGui::SeparatorText("Target Info");

    EqObj* target = nullptr;
    player->get_target(reinterpret_cast<IObjRW*&>(target));
    if (target && !target->is_dying() && target->cobj->archetype->archId != 0xb740ccc5) // waypoint check
    {
        auto eq = target->ceqobj();

        const auto shield = reinterpret_cast<CEShield*>(eq->equipManager.FindFirst(static_cast<uint>(EquipmentClass::Shield)));
        float shieldCurrent = 0.f;
        bool shieldRecharging = false;

        if (shield && shield->maxShieldHitPoints > 0.f)
        {
            shieldCurrent = shield->currShieldHitPoints;

            if (!shield->IsFunctioning())
            {
                shieldRecharging = true;
            }
        }

        float power = 0.f;
        target->get_power(power);

        std::string_view behaviourName = "";
        auto ai = eq->get_behavior_interface();

        if (ai)
        {
            behaviourName = magic_enum::enum_name(ai->get_current_directive()->op);
        }
        else
        {
            behaviourName = "No AI";
        }

        auto vibe = eq->repVibe;
        std::string affiliationStr = "Unknown";
        uint affiliation = 0;

        FmtStr name1(0, nullptr);
        FmtStr name2(0, nullptr);
        const ushort* str = nullptr;
        Reputation::Vibe::GetName(vibe, name1, name2, str);
        Reputation::Vibe::GetAffiliation(vibe, affiliation, true);

        if (affiliation)
        {
            affiliationStr = StringUtils::wstos(Fluf::GetInfocardName(Reputation::get_short_name(affiliation)));
        }

        std::string archStr = StringUtils::wstos(Fluf::GetInfocardName(eq->archetype->idsName));

        ImGui::BeginTable("##dbg-target-table", 2);

        ImKvp("ID", "%u", target->get_id());
        if (eq->objectClass == CObject::CSOLAR_OBJECT)
        {
            ImKvp("Name", "%s", StringUtils::wstos(Fluf::GetInfocardName(name1.strid)).c_str());
        }
        else
        {
            ImKvp("Name", "%s", StringUtils::wstos(reinterpret_cast<const wchar_t*>(str)).c_str());
        }
        ImKvp("Affiliation", "%s", affiliationStr.c_str());
        ImKvp("AI State", "%s", behaviourName.empty() ? "Unknown" : behaviourName.data());
        ImKvp("Velocity", "%.0f", target->get_velocity().Magnitude());
        if (eq->objectClass == CObject::CSOLAR_OBJECT && !reinterpret_cast<CSolar*>(eq)->isDestructible)
        {
            ImKvp("Health", "Indestructible");
        }
        else
        {
            ImKvp("Health", "%.0f (%.0f%%)", eq->hitPoints, eq->get_relative_health() * 100.f);
        }
        ImKvp("Shield", "%.0f (%s)", shieldCurrent, shieldRecharging ? "Offline" : "Online");
        ImKvp("Power", "%.0f", power);

        bool thrustersActive = false;
        target->are_thrusters_active(thrustersActive);
        ImKvp("Thruster State", thrustersActive ? "Active" : "Inactive");

        const auto& [orient, pos] = target->get_transform();
        ImKvp("Position", "%.0f, %.0f, %.0f", pos.x, pos.y, pos.z);
        const auto rotDeg = orient.ToEuler(true);
        const auto rotRad = orient.ToEuler(false);
        ImKvp("Rotation (deg)", "%.2f, %.2f, %.2f", rotDeg.x, rotDeg.y, rotDeg.z);
        ImKvp("Rotation (rad)", "%.5f, %.5f, %.5f", rotRad.x, rotRad.y, rotRad.z);

        EqObj* nextTarget = nullptr;
        target->get_target(reinterpret_cast<IObjRW*&>(nextTarget));

        ImGui::SeparatorText("Target's Target");

        if (nextTarget && !nextTarget->is_dying())
        {
            vibe = nextTarget->ceqobj()->repVibe;
            Reputation::Vibe::GetName(vibe, name1, name2, str);

            affiliation = 0;
            if (_wcsicmp(reinterpret_cast<const wchar_t*>(str), L"(null)") == 0)
            {
                Reputation::Vibe::GetAffiliation(vibe, affiliation, true);

                if (affiliation)
                {
                    affiliationStr = StringUtils::wstos(Fluf::GetInfocardName(Reputation::get_short_name(affiliation)));
                }
            }

            ai = nextTarget->ceqobj()->get_behavior_interface();
            behaviourName = magic_enum::enum_name(ai->get_current_directive()->op);

            ImKvp("ID", "%u", nextTarget->get_id());
            ImKvp("Name", "%s", StringUtils::wstos(reinterpret_cast<const wchar_t*>(str)));
            ImKvp("Affiliation", "%s", affiliationStr);
            ImKvp("AI State", "%s", behaviourName.empty() ? "Unknown" : behaviourName.data());
        }
        else
        {
            ImKvp("ID", "---");
            ImKvp("Name", "---");
            ImKvp("Affiliation", "---");
            ImKvp("AI State", "---");
        }

        ImGui::EndTable();
    }
    else
    {
        ImGui::Text("No target selected");
    }

  earlyExit:
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::End();
}

void Debugger::RenderDebugCommandWindow() {}

void Debugger::OnGameLoad()
{
    const auto weakPtr = Fluf::GetModule(FlufUi::moduleName);
    if (weakPtr.expired())
    {
        throw ModuleLoadException("Debugger was loaded, but FLUF UI was not loaded.");
    }

    const auto module = std::static_pointer_cast<FlufUi>(weakPtr.lock());
    if (module->GetConfig()->uiMode != UiMode::ImGui)
    {
        throw ModuleLoadException("Debugger was loaded, but FLUF.UI's ui mode was not set to ImGui.");
    }

    const auto imgui = module->GetImGuiInterface();
    if (!imgui)
    {
        throw ModuleLoadException("Debugger was loaded, but FLUF.UI's failed to provide an ImGui interface.");
    }

    imgui->RegisterImGuiModule(this);
}

void Debugger::Render()
{
    if (!SinglePlayer())
    {
        return;
    }

    if (renderDebugCommandWindow)
    {
        RenderDebugCommandWindow();
    }

    if (const auto player = Fluf::GetPlayerIObj(); player && !player->is_dying() && renderDebugSpaceWindow)
    {
        RenderDebugSpaceWindow(player);
    }
}

Debugger::Debugger()
{
    instance = this;

    AssertRunningOnClient;
}

Debugger::~Debugger() = default;

std::string_view Debugger::GetModuleName() { return moduleName; }

SETUP_MODULE(Debugger);
