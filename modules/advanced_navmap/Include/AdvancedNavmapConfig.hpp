#pragma once

#include <vector>
#include <string>
#include <map>

struct AdvancedNavmapConfig
{
    static constexpr char path[] = "modules/config/advanced_navmap.yml";

    // Lista de sistemas que se pueden mostrar en el mapa
    std::vector<std::string> displayable_systems;

    // Configuración del mapa
    bool show_player_position = true;
    bool show_system_grid = true;
    bool show_coordinates = true;
    float map_scale = 1.0f;
};