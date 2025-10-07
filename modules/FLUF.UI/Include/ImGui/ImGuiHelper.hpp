#pragma once

#include <ImportFlufUi.hpp>
#include <imgui.h>

class FLUF_UI_API ImGuiHelper
{
    public:
        ImGuiHelper() = delete;
        static void HelpMarker(const char* desc, char character = '?', ImU32 color = 0);
        static void CenterNextWindow(ImVec2 size = { 1280.f, 1024 });

        /**
         * @brief Write text on a single line that is aligned centrally
         * @param text The text to write
         * @param widthOverride An optional number to use instead of the window width, useful when dealing with tables or child windows.
         */
        static void CenteredText(const char* text, std::optional<float> widthOverride = std::nullopt);

        /**
       * @brief Write text on a single line that is aligned to the right
       * @param text The text to write
       * @param widthOverride An optional number to use instead of the window width, useful when dealing with tables or child windows.
       */
        static void RightAlignedText(const char* text, std::optional<float> widthOverride = std::nullopt);
};
