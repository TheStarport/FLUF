#pragma once

#include <ImportFlufUi.hpp>
#include <imgui.h>

class FLUF_UI_API ImGuiHelper
{
    public:
        ImGuiHelper() = delete;
        static void HelpMarker(const char* desc);
};
