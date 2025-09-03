#include "PCH.hpp"
#include "../Include/advanced_navmap.hpp"
#include "../Include/NavmapWindow.hpp"
#include "FLUF/Include/Fluf.hpp"
#include "FLUF.UI/Include/ImGui/ImGuiInterface.hpp"
#include "imgui.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cstdio> // sscanf
#include <cmath>
#include <algorithm>
#include <cstring> // memset
#include "vendor/FLHookSDK/include/FLCore/Common/Globals.hpp" // for Players

// Forward declarations for helper functions
std::string loadIniFile(const std::string& relativePath);
static std::string WideToUtf8(const std::wstring& w);

NavmapWindow* g_navmapWindow = nullptr;

NavmapWindow::NavmapWindow(ImGuiInterface* imguiInterface, bool* openState, advanced_navmap* owner)
    : FlWindow("Advanced Navigation Map", ImGuiWindowFlags_NoMove), imguiInterface(imguiInterface), openState(openState), owner(owner)
{
    g_navmapWindow = this;
    CenterWindow();
    SetSize(ImVec2(620, 700));
    systemNameBuf[0] = '\0';
}

void NavmapWindow::ParseAndCacheSystemData(const std::string& systemFile, CachedSystemData& cache)
{
    if (cache.isCached) return; // Already cached

    std::string iniContent = loadIniFile(systemFile);
    if (iniContent.empty()) {
        // Don't mark as cached if we couldn't load the file
        // This will allow retrying on next access
        return;
    }

    // Clear existing data
    cache.planets.clear();
    cache.bases.clear();

    // Helper: trim (removes spaces at beginning/end)
    auto trim = [](const std::string& s) -> std::string
    {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        return s.substr(start, end - start + 1);
    };

    // --- Settings: use Z as map-Y and flip Y on screen (works with typical INIs) ---
    const bool useZAsMapY = true;         // if true: map currentPos.y = z; if false: currentPos.y = y

    // Parse INI and collect planets and bases
    std::istringstream iniStream(iniContent);
    std::string ln;
    bool inObject = false;
    bool isPlanet = false;
    bool hasPos = false;

    // temporaries per object
    PlanetEntry currentPlanet;
    BaseEntry currentBase;
    ImVec2 currentPos;
    std::string currentRawNickPlanet;
    std::string currentRawNickBase;

    currentPlanet.pos = ImVec2(0.0f, 0.0f);
    currentPlanet.idsName = 0;
    currentPlanet.nickname.clear();
    currentPlanet.visit = false;

    currentBase.pos = ImVec2(0.0f, 0.0f);
    currentBase.baseName.clear();
    currentBase.nickname.clear();
    currentBase.idsName = 0;
    currentBase.visit = false;
    currentRawNickPlanet.clear();
    currentRawNickBase.clear();

    auto finalize_current_object = [&]()
    {
        // Planet nickname: prefer idsName -> infocard, else raw nickname, else empty
        if (currentPlanet.idsName > 0)
        {
            std::wstring w = Fluf::GetInfocardName(currentPlanet.idsName);
            if (!w.empty())
            {
                currentPlanet.nickname.assign(WideToUtf8(w));
            }
            else if (!currentRawNickPlanet.empty())
            {
                currentPlanet.nickname = currentRawNickPlanet;
            }
            else
            {
                currentPlanet.nickname.clear();
            }
        }
        else if (!currentRawNickPlanet.empty())
        {
            currentPlanet.nickname = currentRawNickPlanet;
        }
        else
        {
            currentPlanet.nickname.clear();
        }

        // Base nickname: prefer idsName -> infocard, else raw nickname, else empty
        if (currentBase.idsName > 0)
        {
            std::wstring w = Fluf::GetInfocardName(currentBase.idsName);
            if (!w.empty())
            {
                currentBase.nickname.assign(WideToUtf8(w));
            }
            else if (!currentRawNickBase.empty())
            {
                currentBase.nickname = currentRawNickBase;
            }
            else
            {
                currentBase.nickname.clear();
            }
        }
        else if (!currentRawNickBase.empty())
        {
            currentBase.nickname = currentRawNickBase;
        }
        else
        {
            currentBase.nickname.clear();
        }

        // push if applicable - planets will be filtered by discovery status later
        if (isPlanet && hasPos)
        {
            cache.planets.push_back(currentPlanet);
        }
        if (!currentBase.baseName.empty() && hasPos && !currentRawNickBase.empty())
        {
            uint32_t baseId = CreateID(currentRawNickBase.c_str());
            uint clientId = Fluf::GetPlayerClientId();
            PlayerData& playerData = Players[clientId];
            auto it = playerData.visitEntries.find(baseId);
            uint32_t value = (it != playerData.visitEntries.end()) ? it->second : 0;
            if (value || currentBase.visit) // if it has been visited or has Visit=1
            {
                cache.bases.push_back(currentBase);
            }
        }

        // clear temporaries
        currentPlanet = PlanetEntry();
        currentPlanet.idsName = 0;
        currentPlanet.nickname.clear();
        currentPlanet.visit = false;

        currentBase = BaseEntry();
        currentBase.idsName = 0;
        currentBase.baseName.clear();
        currentBase.nickname.clear();
        currentBase.visit = false;

        currentRawNickPlanet.clear();
        currentRawNickBase.clear();
        isPlanet = false;
        hasPos = false;
    };

    while (std::getline(iniStream, ln))
    {
        std::string L = trim(ln);
        if (L.empty()) continue;
        if (L[0] == ';') continue;

        if (L == "[Object]")
        {
            if (inObject)
            {
                // close previous
                finalize_current_object();
            }
            // start new object
            inObject = true;
            isPlanet = false;
            hasPos = false;
            currentPos = ImVec2(0.0f, 0.0f);
            // leaves currentPlanet/currentBase reset (finalize already does the cleanup)
            continue;
        }

        if (!inObject) continue;

        // nickname (we only save the raw; we'll resolve when closing the object)
        if (L.rfind("nickname", 0) == 0)
        {
            size_t eq = L.find('=');
            if (eq != std::string::npos)
            {
                std::string nm = trim(L.substr(eq + 1));
                currentRawNickPlanet = nm;
                currentRawNickBase = nm;
            }
            continue;
        }

        // ids_name = 12345  -> parsear y guardar entero
        if (L.rfind("ids_name", 0) == 0)
        {
            size_t eq = L.find('=');
            if (eq != std::string::npos)
            {
                std::string val = trim(L.substr(eq + 1));
                int id = 0;
                if (sscanf(val.c_str(), "%d", &id) == 1)
                {
                    currentPlanet.idsName = id;
                    currentBase.idsName = id;
                }
            }
            continue;
        }

        // pos = x, y, z
        if (L.rfind("pos", 0) == 0)
        {
            size_t eq = L.find('=');
            if (eq != std::string::npos)
            {
                std::string vals = trim(L.substr(eq + 1));
                float x = 0.0f, y = 0.0f, z = 0.0f;
                if (sscanf(vals.c_str(), "%f , %f , %f", &x, &y, &z) == 3)
                {
                    ImVec2 mapped;
                    mapped.x = x;
                    mapped.y = (useZAsMapY ? z : y);
                    currentPos = mapped;
                    currentPlanet.pos = mapped;
                    currentBase.pos = mapped;
                    hasPos = true;
                }
            }
            continue;
        }

        // atmosphere_range indicates it's a planet
        if (L.rfind("atmosphere_range", 0) == 0)
        {
            isPlanet = true;
            continue;
        }

        // visit = 1  -> if 1, show even if not visited by player
        if (L.rfind("visit", 0) == 0)
        {
            size_t eq = L.find('=');
            if (eq != std::string::npos)
            {
                std::string val = trim(L.substr(eq + 1));
                int v = 0;
                if (sscanf(val.c_str(), "%d", &v) == 1)
                {
                    currentPlanet.visit = (v == 1);
                    currentBase.visit = (v == 1);
                }
            }
            continue;
        }

        // base = some_base_name  -> mark as base (we'll save baseName)
        if (L.rfind("base", 0) == 0)
        {
            size_t eq = L.find('=');
            if (eq != std::string::npos)
            {
                std::string val = trim(L.substr(eq + 1));
                if (!val.empty())
                {
                    currentBase.baseName = val;
                }
            }
            continue;
        }
    }

    // close last object
    if (inObject)
    {
        finalize_current_object();
    }

    cache.isCached = true;
}

NavmapWindow::CachedSystemData* NavmapWindow::GetCachedSystemData(const std::string& systemFile)
{
    auto it = systemDataCache.find(systemFile);
    if (it == systemDataCache.end())
    {
        // Create new cache entry
        CachedSystemData& newCache = systemDataCache[systemFile];
        ParseAndCacheSystemData(systemFile, newCache);
        return &newCache;
    }
    return &it->second;
}

void NavmapWindow::InvalidateSystemCache(const std::string& systemFile)
{
    auto it = systemDataCache.find(systemFile);
    if (it != systemDataCache.end())
    {
        it->second.isCached = false;
        OutputDebugStringA("[NavMap] Invalidated cache for system\n");
    }
}

void NavmapWindow::RefreshDiscoveredSystems()
{
    // Check for newly discovered systems
    uint clientId = Fluf::GetPlayerClientId();
    if (clientId == 0) return;

    PlayerData& playerData = Players[clientId];

    // Check each system in our found_systems list
    for (const auto& sys : found_systems)
    {
        // Always refresh the cache for systems that have bases - this ensures visit data is up to date
        CachedSystemData* cache = GetCachedSystemData(sys.file);
        if (cache && cache->isCached && !cache->bases.empty())
        {
            // Check if the visit data has changed by comparing cached visit status with current player data
            bool visitDataChanged = false;

            for (const auto& base : cache->bases)
            {
                uint32_t baseId = CreateID(base.baseName.c_str());
                auto visitIt = playerData.visitEntries.find(baseId);
                uint32_t currentVisitValue = (visitIt != playerData.visitEntries.end()) ? visitIt->second : 0;

                // If the current visit status doesn't match what we have cached, refresh
                if ((currentVisitValue > 0) != base.visit)
                {
                    visitDataChanged = true;
                    break;
                }
            }

            if (visitDataChanged)
            {
                InvalidateSystemCache(sys.file);
            }
        }
    }
}

NavmapWindow::~NavmapWindow()
{
    if (g_navmapWindow == this)
    {
        g_navmapWindow = nullptr;
    }
}

std::string loadIniFile(const std::string& relativePath)
{
    // If your paths are relative to "DATA/universe/", you can build the full path here
    std::string basePath = "../DATA/universe/";
    std::string fullPath = basePath + relativePath;

    std::ifstream file(fullPath);
    if (!file.is_open())
    {
        std::cerr << "Could not open INI file: " << fullPath << std::endl;
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
static std::string WideToUtf8(const std::wstring& w)
{
    if (w.empty())
    {
        return {};
    }
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    if (n <= 0)
    {
        return {};
    }
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}

void NavmapWindow::RenderWindowContents()
{
    if (!openState)
    {
        return;
    }
    if (!*openState)
    {
        return;
    }

    auto& io = ImGui::GetIO();

    // Get player data to check base visits
    uint clientId = Fluf::GetPlayerClientId();
    PlayerData& playerData = Players[clientId];

    // Refresh discovered systems when window is opened and periodically
    static bool firstTime = true;
    static int frameCounter = 0;
    frameCounter++;

    if (firstTime || frameCounter % 300 == 0) { // Refresh every ~5 seconds (assuming 60 FPS)
        RefreshDiscoveredSystems();
        firstTime = false;
    }

    ImGui::Text("Advanced Navigation Map");
    ImGui::Separator();

    // Canvas size
    ImVec2 canvasSize(540, 540);
    ImGui::InvisibleButton("canvas", canvasSize);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetItemRectMin();
    ImVec2 canvasMax = ImGui::GetItemRectMax();
    ImVec2 mouse = io.MousePos;
    bool canvasHovered = ImGui::IsItemHovered();

    // Persistent states for submap mode and map selection
    static bool inSubMap = false;               // if we are inside the system map
    static int activeSystemIndex = -1;          // system currently open in submap
    static ImVec2 selectedMapPos(-1.0f, -1.0f); // selection coordinates
    static bool mapPosSelected = false;         // if the user has confirmed a coordinate

    // Fuel calculation variables (need to be accessible throughout function)
    const bool useZAsMapY = true;  // Use Z as map Y coordinate
    const bool invertYOnScreen = false;  // screen has Y going down → we invert so "up" in world is up on screen

    // Fondo sutil del canvas (cambia color si estamos en submap para dar feedback)
    if (inSubMap)
    {
        // color ligeramente distinto para indicar que estamos "dentro" de un mapa
        drawList->AddRectFilled(origin, canvasMax, IM_COL32(18, 14, 40, 220), 0.0f);
    }
    else
    {
        drawList->AddRectFilled(origin, canvasMax, IM_COL32(10, 10, 20, 200), 0.0f);
    }

    // Draw the 16x16 grid (same resolution for both modes)
    const ImU32 gridColor = IM_COL32(90, 100, 110, 200);
    for (int i = 0; i <= 16; ++i)
    {
        float x = origin.x + i * canvasSize.x / 16.0f;
        float y = origin.y + i * canvasSize.y / 16.0f;
        drawList->AddLine(ImVec2(x, origin.y), ImVec2(x, origin.y + canvasSize.y), gridColor, 1.0f);
        drawList->AddLine(ImVec2(origin.x, y), ImVec2(origin.x + canvasSize.x, y), gridColor, 1.0f);
    }

    // If we are in overview we show systems; if in submap, we show submap UI
    // Interaction: detect hovered / clicked in overview
    const float pickRadius = 8.0f; // px to detect hover/click
    int hoveredIndex = -1;
    static int selectedIndex = -1; // still exists as global selection

    // Calculate screen positions of systems (only relevant in overview)
    std::vector<ImVec2> screenPos;
    screenPos.reserve(found_systems.size());
    for (const auto& sys : found_systems)
    {
        ImVec2 p(origin.x + sys.posX * canvasSize.x / 16.0f, origin.y + sys.posY * canvasSize.y / 16.0f);
        screenPos.push_back(p);
    }

    if (!inSubMap)
    {
        // Detect hover by distance to mouse
        for (size_t i = 0; i < screenPos.size(); ++i)
        {
            const ImVec2& p = screenPos[i];
            float dx = mouse.x - p.x;
            float dy = mouse.y - p.y;
            float dist2 = dx * dx + dy * dy;
            if (dist2 <= pickRadius * pickRadius)
            {
                hoveredIndex = (int)i;
                break;
            }
        }

        // Selection with click on canvas: when selecting a system, we enter submap
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
        {
            if (hoveredIndex >= 0)
            {
                selectedIndex = hoveredIndex;
                // enter the submap of the selected system
                activeSystemIndex = selectedIndex;
                inSubMap = true;
                // reset selection within submap
                selectedMapPos = ImVec2(-1.0f, -1.0f);
                mapPosSelected = false;
            }
            else
            {
                selectedIndex = -1;
            }
        }

        // Draw points (only points, no badges)
        const ImU32 pointColor = IM_COL32(255, 200, 60, 255);
        const ImU32 selectedColor = IM_COL32(180, 240, 140, 255);
        for (size_t i = 0; i < screenPos.size(); ++i)
        {
            const ImVec2& p = screenPos[i];
            float radius = ((int)i == selectedIndex) ? 6.0f : 4.0f;
            ImU32 col = ((int)i == selectedIndex) ? selectedColor : pointColor;
            drawList->AddCircleFilled(p, radius, col);
        }

        // If hover, show tooltip with name and coords
        if (hoveredIndex >= 0 && hoveredIndex < (int)found_systems.size())
        {
            const auto& s = found_systems[hoveredIndex];
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(s.name.c_str());
            ImGui::Separator();
            ImGui::Text("Coords: %.3f, %.3f", s.posX, s.posY);
            ImGui::EndTooltip();
        }
    }
    else
    {
        // === SUBMAP: system map===
        // Feedback UI superior
        const char* sysName = "(unknown)";
        const float navmapScale = found_systems[activeSystemIndex].scale;
        const float baseMin = -140000.0f;
        const float baseMax = 140000.0f;
        const float scaledRange = (baseMax - baseMin) * navmapScale;
        const float scaledMin = baseMin * navmapScale;
        const float scaledMax = scaledMin + scaledRange;

        // Get cached system data instead of parsing every frame
        CachedSystemData* cachedData = GetCachedSystemData(found_systems[activeSystemIndex].file);
        if (!cachedData) {
            // Cache doesn't exist at all - this shouldn't happen
            ImGui::Text("Error: System cache not found");
            return;
        }

        if (!cachedData->isCached) {
            // Try to parse the data
            ParseAndCacheSystemData(found_systems[activeSystemIndex].file, *cachedData);

            // If still not cached after parsing attempt, show error
            if (!cachedData->isCached) {
                ImGui::Text("Failed to load system data");
                return;
            }
        }

        // At this point we have the system data cache (may be empty if INI loading failed)
        // We can still show basic system information even without detailed planet/base data

        // Filter planets based on discovery status
        std::vector<PlanetEntry> discoveredPlanets;
        for (const auto& planet : cachedData->planets)
        {
            bool planetDiscovered = false;

            if (!planet.nickname.empty())
            {
                uint32_t planetId = CreateID(planet.nickname.c_str());
                auto it = playerData.visitEntries.find(planetId);
                uint32_t value = (it != playerData.visitEntries.end()) ? it->second : 0;

                // Planet is discovered if visited by player OR has Visit=1 in INI
                if (value > 0)
                {
                    planetDiscovered = true;
                }
            }

            // Also check if planet has Visit=1 in INI (for planets that should always be shown)
            if (!planetDiscovered && planet.visit)
            {
                planetDiscovered = true;
            }

            if (planetDiscovered)
            {
                discoveredPlanets.push_back(planet);
            }
        }

        // Use filtered data
        const auto& planets = discoveredPlanets;
        const auto& bases = cachedData->bases;

        const unsigned char circleAlpha = 51; // ~20% opacity

        // --- Draw planets + bases ---
        if (scaledRange > 0.0f)
        {
            const float planetRadiusScreen = 6.0f;
            // We use min(canvasSize.x, canvasSize.y) for pixel scale calculation (to avoid circle distortion)
            const float pixelBase = std::min(canvasSize.x, canvasSize.y);

            // Show planets if we have them
            if (!planets.empty())
            {
                for (const PlanetEntry& pl : planets)
                {
                    float nx = (pl.pos.x - scaledMin) / scaledRange; // 0..1
                    float ny = (pl.pos.y - scaledMin) / scaledRange; // 0..1

                    // clamp
                    nx = (nx < 0.0f) ? 0.0f : ((nx > 1.0f) ? 1.0f : nx);
                    ny = (ny < 0.0f) ? 0.0f : ((ny > 1.0f) ? 1.0f : ny);

                    float screenX = origin.x + nx * canvasSize.x;
                    float screenY = invertYOnScreen ? origin.y + (1.0f - ny) * canvasSize.y : origin.y + ny * canvasSize.y;
                    ImVec2 screenPos(screenX, screenY);

                    // white dot for the planet
                    drawList->AddCircleFilled(screenPos, planetRadiusScreen, IM_COL32(255, 255, 255, 255));

                    // optional nickname
                    if (!pl.nickname.empty())
                    {
                        ImVec2 textPos(screenPos.x + planetRadiusScreen + 4.0f, screenPos.y - ImGui::GetFontSize() * 0.5f);
                        drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.75f, textPos, IM_COL32(220, 220, 220, 220), pl.nickname.c_str());
                    }
                }
            }

            // Show bases if we have them
            if (!bases.empty())
            {
                // Now we draw the bases (blue square + text)
                const float baseHalf = 6.0f;                           // size in px (half the side of the square)
                const ImU32 baseFill = IM_COL32(70, 150, 220, 255);    // blue
                const ImU32 baseBorder = IM_COL32(220, 240, 255, 200); // light border

                for (const BaseEntry& b : bases)
                {
                    float nx = (b.pos.x - scaledMin) / scaledRange;
                    float ny = (b.pos.y - scaledMin) / scaledRange;

                    nx = (nx < 0.0f) ? 0.0f : ((nx > 1.0f) ? 1.0f : nx);
                    ny = (ny < 0.0f) ? 0.0f : ((ny > 1.0f) ? 1.0f : ny);

                    float screenX = origin.x + nx * canvasSize.x;
                    float screenY = invertYOnScreen ? origin.y + (1.0f - ny) * canvasSize.y : origin.y + ny * canvasSize.y;
                    ImVec2 center(screenX, screenY);

                    ImVec2 rectMin(center.x - baseHalf, center.y - baseHalf);
                    ImVec2 rectMax(center.x + baseHalf, center.y + baseHalf);
                    drawList->AddRectFilled(rectMin, rectMax, baseFill, 3.0f);
                    drawList->AddRect(rectMin, rectMax, baseBorder, 3.0f, 0, 1.0f);

                    // Show base name (nickname if it exists, or baseName)
                    std::string label = !b.nickname.empty() ? b.nickname : b.baseName;
                    if (!label.empty())
                    {
                        ImVec2 textPos(center.x + baseHalf + 4.0f, center.y - ImGui::GetFontSize() * 0.5f);
                        drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.75f, textPos, IM_COL32(220, 220, 220, 220), label.c_str());
                    }
                }
            }
        }

        // Draw player position if we are in the same system
        if (activeSystemIndex >= 0 && activeSystemIndex < (int)found_systems.size() && found_systems[activeSystemIndex].id == playerData.systemId)
        {
            auto playerShip = Fluf::GetPlayerCShip();
            if (playerShip)
            {
                Vector playerPos3D = playerShip->get_position();
                ImVec2 playerPos2D;
                playerPos2D.x = playerPos3D.x;
                playerPos2D.y = (useZAsMapY ? playerPos3D.z : playerPos3D.y);

                // Convert to screen coordinates
                float nx = (playerPos2D.x - scaledMin) / scaledRange;
                float ny = (playerPos2D.y - scaledMin) / scaledRange;
                nx = (nx < 0.0f) ? 0.0f : ((nx > 1.0f) ? 1.0f : nx);
                ny = (ny < 0.0f) ? 0.0f : ((ny > 1.0f) ? 1.0f : ny);

                float screenX = origin.x + nx * canvasSize.x;
                float screenY = invertYOnScreen ? origin.y + (1.0f - ny) * canvasSize.y : origin.y + ny * canvasSize.y;
                ImVec2 playerScreenPos(screenX, screenY);

                // Draw player diamond
                const float diamondSize = 8.0f;
                const ImU32 playerColor = IM_COL32(0, 255, 0, 255);
                const float lineThickness = 2.0f;

                ImVec2 top(playerScreenPos.x, playerScreenPos.y - diamondSize);
                ImVec2 right(playerScreenPos.x + diamondSize, playerScreenPos.y);
                ImVec2 bottom(playerScreenPos.x, playerScreenPos.y + diamondSize);
                ImVec2 left(playerScreenPos.x - diamondSize, playerScreenPos.y);

                drawList->AddLine(top, right, playerColor, lineThickness);
                drawList->AddLine(right, bottom, playerColor, lineThickness);
                drawList->AddLine(bottom, left, playerColor, lineThickness);
                drawList->AddLine(left, top, playerColor, lineThickness);

                // Etiqueta "Player"
                ImVec2 textPos(playerScreenPos.x + diamondSize + 4.0f, playerScreenPos.y - ImGui::GetFontSize() * 0.5f);
                drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.75f, textPos, IM_COL32(0, 255, 0, 255), "Ship");
            }
        }

        if (activeSystemIndex >= 0 && activeSystemIndex < (int)found_systems.size())
        {
            sysName = found_systems[activeSystemIndex].name.c_str();
        }

        // Small label above the canvas to indicate mode
        ImVec2 badgePos = ImVec2(origin.x + 6.0f, origin.y + 6.0f);
        drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.9f, ImVec2(badgePos.x + 8.0f, badgePos.y + 4.0f), IM_COL32(240, 240, 240, 255), sysName);

        // Instruction indicator in the bottom canvas
        ImVec2 instrPos = ImVec2(origin.x + 8.0f, origin.y + canvasSize.y - 24.0f);
        drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.8f, instrPos, IM_COL32(220, 220, 220, 200), "Right-click to return.");

        // If the mouse is inside and we click, calculate system coords
        bool mouseInside = canvasHovered;
        if (mouseInside)
        {
            // Normalize mouse to 0..1 in canvas
            float normX = (mouse.x - origin.x) / canvasSize.x;
            float normY = (mouse.y - origin.y) / canvasSize.y;
            // clamp
            if (normX < 0.0f)
            {
                normX = 0.0f;
            }
            if (normX > 1.0f)
            {
                normX = 1.0f;
            }
            if (normY < 0.0f)
            {
                normY = 0.0f;
            }
            if (normY > 1.0f)
            {
                normY = 1.0f;
            }

            // World coordinates
            float mappedNormY = invertYOnScreen ? (1.0f - normY) : normY;
            float worldX = scaledMin + normX * scaledRange;
            float worldY = scaledMin + mappedNormY * scaledRange;

            // Draw crosshair
            float hoverX = origin.x + (worldX - scaledMin) / scaledRange * canvasSize.x;
            float hoverY = origin.y + ((invertYOnScreen ? (1.0f - (worldY - scaledMin) / scaledRange) : ((worldY - scaledMin) / scaledRange))) * canvasSize.y;
            ImVec2 hoverScreenPos(hoverX, hoverY);

            // Text with floating coords next to the cursor
            char coordBuf[64];
            snprintf(coordBuf, sizeof(coordBuf), "%.2f, %.2f", worldX, worldY);
            ImVec2 txtSz = ImGui::CalcTextSize(coordBuf);
            ImVec2 txtPos = ImVec2(hoverScreenPos.x + 10.0f, hoverScreenPos.y - txtSz.y * 0.5f);
            drawList->AddRectFilled(
                ImVec2(txtPos.x - 4.0f, txtPos.y - 4.0f), ImVec2(txtPos.x + txtSz.x + 4.0f, txtPos.y + txtSz.y + 4.0f), IM_COL32(12, 12, 18, 200), 4.0f);
            drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.8f, txtPos, IM_COL32(240, 240, 240, 255), coordBuf);

            // left click: select coords
            if (ImGui::IsMouseClicked(0))
            {
                selectedMapPos = ImVec2(worldX, worldY);
                mapPosSelected = true;
            }

            // right click: return to overview (shortcut)
            if (ImGui::IsMouseClicked(1))
            {
                inSubMap = false;
                activeSystemIndex = -1;
                mapPosSelected = false;
            }
        }

        // If there is a selection within the map, draw marker and label
        if (mapPosSelected)
        {
            float selX = origin.x + (selectedMapPos.x - scaledMin) / scaledRange * canvasSize.x;
            float selY = origin.y + ((invertYOnScreen ? (1.0f - (selectedMapPos.y - scaledMin) / scaledRange) : ((selectedMapPos.y - scaledMin) / scaledRange))) * canvasSize.y;
            ImVec2 selScreenPos(selX, selY);
            const ImU32 selColor = IM_COL32(180, 240, 140, 255);
            drawList->AddCircleFilled(selScreenPos, 5.0f, selColor);

            char selBuf[64];
            ImVec2 txtSz = ImGui::CalcTextSize(selBuf);
            ImVec2 lblMin = ImVec2(selScreenPos.x + 8.0f, selScreenPos.y - txtSz.y * 0.5f - 6.0f);
            ImVec2 lblMax = ImVec2(lblMin.x + txtSz.x + 12.0f, lblMin.y + txtSz.y + 12.0f);
            // clamp dentro canvas
            if (lblMax.x > origin.x + canvasSize.x)
            {
                float shift = lblMax.x - (origin.x + canvasSize.x);
                lblMin.x -= shift;
                lblMax.x -= shift;
            }
            if (lblMin.y < origin.y)
            {
                float shift = origin.y - lblMin.y;
                lblMin.y += shift;
                lblMax.y += shift;
            }
            drawList->AddRectFilled(lblMin, lblMax, IM_COL32(12, 16, 22, 230), 6.0f);
            drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.85f, ImVec2(lblMin.x + 6.0f, lblMin.y + 6.0f), IM_COL32(240, 240, 240, 255), selBuf);
        }

        // Botón Back (volver a vista general)
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + canvasSize.y + 6.0f - 24.0f);
        ImGui::SameLine();
        ImGui::NewLine();
    }

    // --- Common Drawing ---

    ImVec2 playerPos(-1, -1); // invalid by default
    bool playerPosValid = false;

    if (owner && owner->currentSystemId >= 0)
    {
        // Search for the player's current system in found_systems
        for (const auto& sys : found_systems)
        {
            if (sys.id == owner->currentSystemId)
            {
                playerPos.x = origin.x + sys.posX * canvasSize.x / 16.0f;
                playerPos.y = origin.y + sys.posY * canvasSize.y / 16.0f;
                playerPosValid = true;
                break;
            }
        }

        // if found_systems does not contain the player system, use Universe::get_system()
        if (!playerPosValid)
        {
            auto playerSystem = Universe::get_system(owner->currentSystemId);
            if (playerSystem->navMapPos.x >= 0 && playerSystem->navMapPos.y >= 0)
            {
                playerPos.x = origin.x + playerSystem->navMapPos.x * canvasSize.x / 16.0f;
                playerPos.y = origin.y + playerSystem->navMapPos.y * canvasSize.y / 16.0f;
                playerPosValid = true;
            }
        }
    }

    // if there is a selection, draw compact label with leader line
    if (!inSubMap && selectedIndex >= 0 && selectedIndex < (int)found_systems.size())
    {
        const auto& s = found_systems[selectedIndex];
        ImVec2 p = screenPos[selectedIndex];

        // Label coords
        char labelBuf[256];
        snprintf(labelBuf, sizeof(labelBuf), "%s (%.3f, %.3f)", s.name.c_str(), s.posX, s.posY);
        float fontScale = 0.75f; 
        ImVec2 txtSz = ImGui::CalcTextSize(labelBuf);
        ImVec2 pad(6.0f, 4.0f);

        ImVec2 rectMin = ImVec2(p.x + 12.0f, p.y - txtSz.y / 2.0f - pad.y);
        ImVec2 rectMax = ImVec2(rectMin.x + txtSz.x + pad.x * 2.0f, rectMin.y + txtSz.y + pad.y * 2.0f);

        // clamp dentro del canvas
        if (rectMax.x > origin.x + canvasSize.x)
        {
            float shift = rectMax.x - (origin.x + canvasSize.x);
            rectMin.x -= shift;
            rectMax.x -= shift;
        }
        if (rectMin.y < origin.y)
        {
            float shift = origin.y - rectMin.y;
            rectMin.y += shift;
            rectMax.y += shift;
        }
        if (rectMax.y > origin.y + canvasSize.y)
        {
            float shift = rectMax.y - (origin.y + canvasSize.y);
            rectMin.y -= shift;
            rectMax.y -= shift;
        }

        // fondo y texto
        drawList->AddRectFilled(rectMin, rectMax, IM_COL32(12, 16, 22, 230), 6.0f);
        drawList->AddText(
            ImGui::GetFont(), ImGui::GetFontSize() * fontScale, ImVec2(rectMin.x + pad.x, rectMin.y + pad.y), IM_COL32(240, 240, 240, 255), labelBuf);

        // leader line
        ImVec2 rectCenter((rectMin.x + rectMax.x) * 0.5f, (rectMin.y + rectMax.y) * 0.5f);
        drawList->AddLine(p, rectCenter, IM_COL32(160, 160, 160, 200), 1.0f);
    }

    // Player diamond
    if (playerPosValid && !inSubMap)
    {
        const float diamondSize = 10.0f;                     // Diamond size
        const ImU32 diamondColor = IM_COL32(0, 255, 0, 255); // Bright green
        const float diamondLineThickness = 2.0f;

        ImVec2 top(playerPos.x, playerPos.y - diamondSize);
        ImVec2 right(playerPos.x + diamondSize, playerPos.y);
        ImVec2 bottom(playerPos.x, playerPos.y + diamondSize);
        ImVec2 left(playerPos.x - diamondSize, playerPos.y);

        drawList->AddLine(top, right, diamondColor, diamondLineThickness);
        drawList->AddLine(right, bottom, diamondColor, diamondLineThickness);
        drawList->AddLine(bottom, left, diamondColor, diamondLineThickness);
        drawList->AddLine(left, top, diamondColor, diamondLineThickness);
    }

    ImGui::Separator();

    ImGui::SameLine();
    if (ImGui::Button("Close"))
    {
        *openState = false;
    }
}