#pragma once

#include "FlufUiConfig.hpp"
#include "ImGuiModule.hpp"

#include <unordered_set>

class FlufUi;
class ImGuiInterface
{
        static constexpr int DefaultFontSize = 36;
        inline static bool showStyleWindow = false;
        inline static bool showDemoWindow = false;
        std::unordered_map<std::string, void*> loadedTextures;
        void* dxDevice;
        std::unordered_set<ImGuiModule*> imguiModules;

        struct MouseState
        {
                bool leftDown;
                bool rightDown;
                bool middleDown;
                bool mouse4Down;
                bool mouse5Down;
        };

        friend FlufUi;

        std::shared_ptr<FlufUiConfig> config;
        RenderingBackend backend;
        static ImGuiStyle& GenerateDefaultStyle();

        void Render();
        static void PollInput();
        static MouseState ConvertState(DWORD state);
        static bool WndProc(FlufUiConfig* config, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    public:
        ~ImGuiInterface();
        explicit ImGuiInterface(FlufUi* flufUi, RenderingBackend backend, void* device);

        /**
         * @brief Loads an image from the specified path using the currently running render backend.
         * @param path The path of the image file. Can be relative to the EXE folder or absolute.
         * Supported formats are: .bmp, .dds, .dib, .hdr, .jpg, .pfm, .png, .ppm, and .tga.
         * @param width If image loading is successful, width will be populated with the horizontal size of the image.
         * @param height If image loading is successful, height will be populated with the vertical size of the image.
         * @returns A void* of whose type will depend on the currently running render backend.\n
         * In the event it is DX9, it will be a PDIRECT3DTEXTURE9.
         */
        FLUF_UI_API void* LoadTexture(const std::string& path, uint& width, uint& height);
        /**
         * @brief Register a plugin to receive the render event from ImGui.
         */
        FLUF_UI_API bool RegisterImGuiModule(ImGuiModule*);

        /**
         * @brief Remove a previously registered plugin.
         */
        FLUF_UI_API bool UnregisterImGuiModule(ImGuiModule*);

        /**
         * @brief Gets a loaded font with the provided name and size. If the font has been loaded,
         * but this font size has not been used previously, it will reload the font in the desired size.
         */
        FLUF_UI_API ImFont* GetImGuiFont(const std::string& fontName, int fontSize) const;
};
