#pragma once

#include "ImportFlufUi.hpp"

enum class UiMode
{
    None,
    Rml,
};

class FLUF_UI_API FlufUiConfig
{
        std::string_view GetSaveLocation();

    public:
        UiMode uiMode = UiMode::Rml;
        float dpi = 1.0f;

        bool Save();
        void Load();
};
