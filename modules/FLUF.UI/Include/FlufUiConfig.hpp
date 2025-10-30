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
    OpenGL
};

struct LoadedFont
{
        std::string fontName;
        std::string fontPath;
        bool isDefault = false;
        rfl::Skip<ImFont*> defaultFont = nullptr;
        rfl::Skip<ImFont*> italicFont = nullptr;
        rfl::Skip<ImFont*> boldFont = nullptr;
        rfl::Skip<ImFont*> boldItalicFont = nullptr;
};

struct FlufUiConfig
{
        static constexpr char configPath[] = "modules/config/FLUF.UI.yml";
        static constexpr char configPathOverrides[] = "FLUF.UI-overrides.yml";

        UiMode uiMode = UiMode::None;
        float dpi = 1.0f;
        bool enforceUiMode;
        std::vector<LoadedFont> loadedFonts{
            { "Saira", "SairaCondensed-Light.otf", true }
        };
};
