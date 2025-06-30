#pragma once

#include "ImportFlufUi.hpp"

struct ImFont;
enum class UiMode
{
    None,
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
        std::set<int> fontSizes;
        rfl::Skip<std::unordered_map<int, ImFont*>> fontSizesInternal;
};

struct FlufUiConfig
{
        static constexpr char configPath[] = "modules/config/FLUF.UI.yml";
        static constexpr char configPathOverrides[] = "FLUF.UI-overrides.yml";

        UiMode uiMode = UiMode::None;
        float dpi = 1.0f;
        bool enforceUiMode;
        std::vector<LoadedFont> loadedFonts{
            { "Saira", "SairaCondensed-Light.ttf",  true, { 24, 36, 46, 48 } },
            {    "FA",                         "", false,     { 24, 36, 48 } },
        };
};
