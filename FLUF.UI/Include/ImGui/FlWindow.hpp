#pragma once

#include "FLUF.UI.hpp"

#include <imgui.h>
#include <string>

class FlWindow
{
        std::string title;
        ImVec2 pivot;
        ImVec2 position;
        ImVec2 size{ 1280.f, 1024.f };
        bool centered = false;
        ImGuiWindowFlags windowFlags;
        ImGuiCond conditionFlag;
        bool drawScrollbars;
        inline static RenderingBackend renderingBackend;
        inline static void* dxDevice;
        inline static ImTextureID backgroundTexture;
        inline static ImVec2 imageSize;

        void DrawScrollbars();
        static void DrawWindowDecorations(ImVec2 startingPos, ImVec2 windowSize);

    protected:
        bool isOpen = false;
        ~FlWindow() = default;
        virtual void RenderWindowContents() = 0;

    public:
        void SetTitle(const std::string& title);
        void CenterWindow(bool center = true);
        void SetSize(ImVec2 size);
        void SetPosition(ImVec2 position);
        void SetPivot(ImVec2 pivot);
        void Render();
        explicit FlWindow(std::string windowName, ImGuiWindowFlags flags = ImGuiWindowFlags_None, ImGuiCond condition = ImGuiCond_Appearing);
};
