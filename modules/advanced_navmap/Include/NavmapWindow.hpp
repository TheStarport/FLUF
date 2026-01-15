#pragma once
#include "advanced_navmap.hpp"
#include "ImGui/FlWindow.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <imgui.h>

class advanced_navmap;

class NavmapWindow final : public FlWindow
{
public:
    NavmapWindow(ImGuiInterface* imguiInterface, bool* openState, advanced_navmap* owner);
    ~NavmapWindow();
    void RenderWindowContents() override;

    struct VisitedSystem {
        uint32_t id;
        std::string name;
        float posX;
        float posY;
        float scale = 1.0f; // escala del sistema para el navmap
        std::string file;  // archivo del sistema
    };

    // Cached system data structures
    struct PlanetEntry {
        ImVec2 pos;           // world coords (x, y) donde y puede ser el Y o el Z según useZAsMapY
        std::string nickname; // opcional si quieres etiquetar
        int idsName;
        bool visit;           // si Visit=1 en el INI (siempre visible)
    };

    struct BaseEntry {
        ImVec2 pos;           // en unidades del mundo (x, mapY)
        std::string baseName; // valor de la clave "base = ..."
        std::string nickname; // nickname si quieres mostrarlo
        int idsName;
        bool visit;           // si Visit=1 en el INI
    };

    struct CachedSystemData {
        std::vector<PlanetEntry> planets;
        std::vector<BaseEntry> bases;
        bool isCached = false;
    };

    // miembro público (accesible desde advanced_navmap.cpp)
    std::vector<VisitedSystem> found_systems;

private:
    char systemNameBuf[128];

    ImGuiInterface* imguiInterface;
    bool* openState;
    advanced_navmap* owner;

    // Cache for parsed system data
    std::unordered_map<std::string, CachedSystemData> systemDataCache;

    // Helper methods for parsing and caching
    void ParseAndCacheSystemData(const std::string& systemFile, CachedSystemData& cache);
    CachedSystemData* GetCachedSystemData(const std::string& systemFile);
    void InvalidateSystemCache(const std::string& systemFile);
    void RefreshDiscoveredSystems();

    friend advanced_navmap;
};

// Puntero global a la instancia (declaración)
extern NavmapWindow* g_navmapWindow;