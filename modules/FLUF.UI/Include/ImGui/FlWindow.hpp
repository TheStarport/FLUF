#pragma once

#include "FLUF.UI.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <string>

class FLUF_UI_API FlWindow
{
        std::string title;
        ImVec2 pivot;
        ImVec2 position;
        ImVec2 size{ 1280.f, 1024.f };
        bool centered = false;
        ImGuiWindowFlags windowFlags;
        ImGuiCond conditionFlag;
        bool drawScrollbars;
        RenderingBackend renderingBackend;
        void* dxDevice;
        bool isEscapeCloseable;
        bool isOpen = false;
        ImGuiWindow* imguiWindow = nullptr;

        void DrawScrollbars() const;
        static void DrawWindowDecorations(ImVec2 startingPos, ImVec2 windowSize);

    protected:
        inline static ImGuiInterface* imguiInterface;
        ~FlWindow();
        virtual void RenderWindowContents() = 0;

    public:
        ImGuiWindow* GetImGuiWindow() const;
        void SetOpenState(bool newState);
        bool IsOpen() { return isOpen; }
        bool IsEscapeCloseable() { return isEscapeCloseable; }
        void SetTitle(const std::string& title);
        void CenterWindow(bool center = true);
        void SetSize(ImVec2 size);
        /**
         * @brief
         * Set the size of the window to the desired width, with the height calculated dynamically
         * according the specified aspect ratio. If the width is greater than the current display size, it will be shrunk
         * incrementally by the specified shrink percentage.
         * @param width The maximum possible width, will be shrunk if too large
         * @param aspectRatio A ratio of width to height pixels. Common values are 16:9 and 4:3
         * @param shrinkPercentage A value between 0 and 0.8f, that will be the amount to reduce the width by each attempt to see
         * if it can fit on the screen
         * @note This should be called before each render function, it's suggested to override Render()
         * and call this before calling the original. This must be called frequently because the screensize can change while the application
         * is still open (if shifted to another monitor, or switching between windowed and fullscreen mode)
         */
        void SetSizeWithAspectRatio(int width, ImVec2 aspectRatio, float shrinkPercentage = 0.2f);

        /**
         * @brief
         * Set position using relative coordinates rather than absolute ones.
         * Providing a value between 0 and 1 will correlate to 'percent of dimension',
         * so an x value of 0.5 when the width of the screen is 1000, will set x at 500
         * @param pos A vector where X and Y are values between 0 and 1
         */
        void SetPositionRelative(ImVec2 pos);
        void SetPosition(ImVec2 position);
        void SetPivot(ImVec2 pivot);
        /**
         * @note If overwriting, ensure that the original function is called
         */
        virtual void Render();
        static void RenderImguiFromDisplayList(RenderDisplayList* rdl);
        explicit FlWindow(std::string windowName, ImGuiWindowFlags flags = ImGuiWindowFlags_None, ImGuiCond condition = ImGuiCond_Appearing,
                          bool isEscapeCloseable = true);
};
