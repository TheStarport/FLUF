#include "../Include/advanced_navmap.hpp"
#include "../Include/AdvancedNavmapConfig.hpp"
#include "vendor/FLHookSDK/include/FLCore/FLCoreServer.h"
#include "FLUF.UI/Include/FLUF.UI.hpp"                       /* defines FlufUi, UiMode, etc. */
#include "FLUF/Include/Fluf.hpp"
#include "FLUF/Include/KeyManager.hpp"
#include "FLUF.UI/Include/ImGui/ImGuiInterface.hpp"         /* to register ImGui module */
#include "../Include/NavmapWindow.hpp"
#include <stdexcept>
#include <windows.h> /* OutputDebugStringA for debugging */
#include <map>

#ifndef ModuleLoadException
using ModuleLoadException = std::runtime_error;
#endif
#include <PCH.hpp>

std::shared_ptr<AdvancedNavmapConfig> config;

advanced_navmap::advanced_navmap()
    : showMessage(false), flufUi(nullptr), navmapWindow(nullptr), currentSystemId(0)
{
    OutputDebugStringA("advanced_navmap::advanced_navmap() - Constructor called\n");
}

advanced_navmap::~advanced_navmap()
{
    if (flufUi && flufUi->GetConfig()->uiMode == UiMode::ImGui)
    {
        flufUi->GetImGuiInterface()->UnregisterImGuiModule(this);
        Fluf::Log(LogLevel::Trace, "advanced_navmap: Unregistered ImGui module on destruction.");
    }
}

std::string_view advanced_navmap::GetModuleName()
{
    return "advanced_navmap";
}

std::shared_ptr<AdvancedNavmapConfig> advanced_navmap::GetConfig() const
{
    return config;
}

void advanced_navmap::OnLogin(uint client, bool singlePlayer, FLPACKET_UNKNOWN* flpacket_unknown)
{
    /* implement if needed */
}

void advanced_navmap::OnGameLoad()
{
    OutputDebugStringA("advanced_navmap::OnGameLoad - Starting initialization\n");

    const auto weakPtr = Fluf::GetModule(FlufUi::moduleName);
    if (weakPtr.expired())
    {
        Fluf::Log(LogLevel::Error, "advanced_navmap::OnGameLoad - FlufUi not available.");
        OutputDebugStringA("advanced_navmap::OnGameLoad - FlufUi not available\n");
        return;
    }

    /* lock and cast to FlufUi (FlufUi should inherit FlufModule) */
    flufUi = std::static_pointer_cast<FlufUi>(weakPtr.lock());
    if (!flufUi)
    {
        OutputDebugStringA("advanced_navmap::OnGameLoad - static_pointer_cast failed\n");
        return;
    }

    if (flufUi->GetConfig()->uiMode != UiMode::ImGui)
    {
        Fluf::Log(LogLevel::Error, "advanced_navmap::OnGameLoad - UI mode not ImGui.");
        OutputDebugStringA("advanced_navmap::OnGameLoad - UI mode not ImGui\n");
        return;
    }

    /* Register the module with ImGuiInterface */
    flufUi->GetImGuiInterface()->RegisterImGuiModule(this);
    Fluf::Log(LogLevel::Trace, "advanced_navmap: Registered ImGui module.");
    OutputDebugStringA("advanced_navmap::OnGameLoad - Registered ImGui module\n");

    /* Create the NavmapWindow */
    navmapWindow = std::make_unique<NavmapWindow>(flufUi->GetImGuiInterface(), &showMessage, this);
    Fluf::Log(LogLevel::Trace, "advanced_navmap: NavmapWindow created.");
    OutputDebugStringA("advanced_navmap::OnGameLoad - NavmapWindow created\n");

    /* Register the key with lambda to capture 'this' correctly */
    auto keyCallback = [this]() -> bool {
        return this->OnToggleMessageKeyCommand();
    };

    try {
        Fluf::GetKeyManager()->RegisterKey(
            this,
            "USER_STATUS",
            Key::USER_STATUS, //change this to a different key
            static_cast<KeyFunc>(&advanced_navmap::OnToggleMessageKeyCommand),
            false
        );
        OutputDebugStringA("advanced_navmap::OnGameLoad - Key registered successfully\n");
    } catch (const std::exception& e) {
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg), "advanced_navmap::OnGameLoad - Key registration failed: %s\n", e.what());
        OutputDebugStringA(errorMsg);
    }

    char initMsg[128];
    snprintf(initMsg, sizeof(initMsg), "advanced_navmap::OnGameLoad - Initialization complete, this=%p\n", this);
    OutputDebugStringA(initMsg);
}

static std::string WideToUtf8(const std::wstring& w)
{
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    if (n <= 0) return {};
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}

bool advanced_navmap::OnToggleMessageKeyCommand()
{
    // Debug BEFORE the change
    OutputDebugStringA("=== BEFORE THE CHANGE ===\n");
    char before[128];
    snprintf(before, sizeof(before), "BEFORE: showMessage=%d, address=%p\n",
             showMessage ? 1 : 0, &showMessage);
    OutputDebugStringA(before);

    uint clientId = Fluf::GetPlayerClientId();
    PlayerData& playerData = Players[clientId];

    // Read systems from YAML
    config = std::make_shared<AdvancedNavmapConfig>(
        *ConfigHelper<AdvancedNavmapConfig, AdvancedNavmapConfig::path>::Load()
    );

    // Populate found_systems with systems that have visited bases
    if (navmapWindow) {
        for (uint32_t systemId = 1; systemId < 1000; ++systemId) {
            auto sys = Universe::get_system(systemId);
            if (sys) {
                // Parse the system data to get bases
                NavmapWindow::CachedSystemData cache;
                navmapWindow->ParseAndCacheSystemData(sys->file, cache);
                if (cache.isCached && !cache.bases.empty()) {
                    // Check if any base has been visited
                    bool hasVisitedBase = false;
                    for (const NavmapWindow::BaseEntry& base : cache.bases) {
                        uint32_t baseId = CreateID(base.baseName.c_str());
                        auto it = playerData.visitEntries.find(baseId);
                        if (it != playerData.visitEntries.end() && it->second > 0) {
                            hasVisitedBase = true;
                            break;
                        }
                    }
                    if (hasVisitedBase) {
                        auto nameW = Fluf::GetInfocardName(sys->idsName);
                        std::string name = WideToUtf8(nameW);
                        navmapWindow->found_systems.emplace_back(
                            NavmapWindow::VisitedSystem{
                                sys->id,
                                std::move(name),
                                sys->navMapPos.x,
                                sys->navMapPos.y,
                                sys->navMapScale,
                                sys->file
                            }
                        );
                    }
                }
            }
        }
    }

    // Add current system if not already in the list
    uint32_t currentSystemId = playerData.systemId;
    bool currentSystemAdded = false;

    // Check if current system is already in found_systems
    for (const auto& systemId_check : navmapWindow ? navmapWindow->found_systems : std::vector<NavmapWindow::VisitedSystem>{}) {
        if (systemId_check.id == currentSystemId) {
            currentSystemAdded = true;
            break;
        }
    }

    // If current system is not in the list, add it manually
    if (!currentSystemAdded && navmapWindow) {
        auto currentSys = Universe::get_system(currentSystemId);
        if (currentSys) {
            auto nameW = Fluf::GetInfocardName(currentSys->idsName);
            std::string name = WideToUtf8(nameW);

            navmapWindow->found_systems.emplace_back(
                NavmapWindow::VisitedSystem{
                    currentSys->id,
                    std::move(name),
                    currentSys->navMapPos.x,
                    currentSys->navMapPos.y,
                    currentSys->navMapScale,
                    currentSys->file
                }
            );
        }
    }

    // Change the state of showMessage
    showMessage = !showMessage;

    // If showMessage is true and the window exists, set isOpen to true
    if (showMessage && navmapWindow) {
        navmapWindow->isOpen = true;
        OutputDebugStringA("Set navmapWindow->isOpen = true\n");
    }

    // Get player current system
    this->currentSystemId = playerData.systemId;

    // Debug AFTER the change
    OutputDebugStringA("=== AFTER THE CHANGE ===\n");
    char after[128];
    snprintf(after, sizeof(after), "AFTER: showMessage=%d, address=%p\n",
             showMessage ? 1 : 0, &showMessage);
    OutputDebugStringA(after);

    return true;
}


void advanced_navmap::Render()
{
    // Debug at the start of Render
    char debug[64];
    snprintf(debug, sizeof(debug), "RENDER: showMessage=%d, address=%p\n",
             showMessage ? 1 : 0, &showMessage);
    OutputDebugStringA(debug);

    if (!showMessage) {
        OutputDebugStringA("RENDER: Exiting because showMessage is FALSE\n");
        // If showMessage is false, ensure isOpen is also false
        if (navmapWindow) {
            navmapWindow->isOpen = false;
        }
        return;
    }

    OutputDebugStringA("RENDER: Continuing because showMessage is TRUE\n");

    // Render the NavmapWindow if it exists
    if (navmapWindow) {
        OutputDebugStringA("RENDER: navmapWindow exists, calling Render()\n");
        // Synchronize isOpen with showMessage before rendering
        navmapWindow->isOpen = true;
        char isOpenDebug[64];
        snprintf(isOpenDebug, sizeof(isOpenDebug), "RENDER: navmapWindow->isOpen=%d\n", navmapWindow->isOpen ? 1 : 0);
        OutputDebugStringA(isOpenDebug);
        navmapWindow->Render();
        OutputDebugStringA("RENDER: navmapWindow->Render() called\n");
    } else {
        OutputDebugStringA("RENDER: navmapWindow is NULL\n");
    }
}

SETUP_MODULE(advanced_navmap)