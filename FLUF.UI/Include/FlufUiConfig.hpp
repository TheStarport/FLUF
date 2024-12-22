#pragma once

#include "ImportFlufUi.hpp"

struct ImFont;
enum class UiMode
{
    None,
    Rml,
    ImGui
};

enum class RenderingBackend
{
    Dx8,
    Dx9,
};

struct LoadedFont
{
        std::string fontName;
        std::string fontPath;
        bool isDefault = false;
        rfl::Skip<std::unordered_map<int, ImFont*>> fontSizes;
};

struct FlufUiConfig
{
        static constexpr char configPath[] = "FLUF.UI.yml";
        static constexpr char configPathOverrides[] = "FLUF.UI-overrides.yml";

        UiMode uiMode = UiMode::None;
        float dpi = 1.0f;
        std::vector<LoadedFont> loadedFonts{
            { "Saira", "SairaCondensed-Light.ttf", true }
        };
};
