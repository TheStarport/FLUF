#pragma once
#include "FLUF/Include/FlufModule.hpp"
#include "FLUF/Include/KeyManager.hpp"
#include "FLUF.UI/Include/ImGui/ImGuiModule.hpp"
#include "AdvancedNavmapConfig.hpp"
#include <memory>
#include <string_view>

/* Forward-declare FlufUi para que podamos usar std::shared_ptr<FlufUi> en el header. */
/* No incluimos el header completo aquí para reducir dependencias; lo incluimos en el .cpp. */
class FlufUi;
class NavmapWindow;

class advanced_navmap : public FlufModule, public ImGuiModule
{
public:
    advanced_navmap();
    ~advanced_navmap() override;

    std::vector<uint32_t> visitedCache;

    /* Hooks */
    void OnGameLoad() override;
    void OnLogin(uint client, bool singlePlayer, FLPACKET_UNKNOWN* flpacket_unknown) override;

    /* ImGuiModule::Render */
    void Render() override;
    std::string_view GetModuleName() override;

    /* callback de tecla */
    bool OnToggleMessageKeyCommand();

    /* Get config */
    std::shared_ptr<AdvancedNavmapConfig> GetConfig() const;

    /* Discovered system list*/
    std::vector<std::string> discoveredSystems;
    //current system
    uint32_t currentSystemId;

private:
    bool showMessage = false;
    std::shared_ptr<FlufUi> flufUi;
    std::unique_ptr<NavmapWindow> navmapWindow;
};