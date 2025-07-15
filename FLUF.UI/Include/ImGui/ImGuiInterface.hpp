#pragma once

#include "FlufUiConfig.hpp"
#include "ImGuiModule.hpp"

#include <unordered_set>

class FlufModule;
class FlufUi;
using ModuleCall = void (FlufModule::*)();
using OnRenderStatsMenu = ModuleCall;
using RegisterMenuFunc = ModuleCall;
using RegisterOptionsFunc = void (FlufModule::*)(bool saveRequested);

enum class FontSize
{
    VerySmall = 12,
    Small = 24,
    Default = 36,
    Big = 48,
    VeryBig = 60
};

// Conditional fwd decs
#ifdef FLUF_UI
class PlayerStatusWindow;
class CustomHud;
#endif
class ImGuiInterface
{
        inline static bool showDemoWindow = false;
        void* renderingContext;
        std::unordered_map<std::string, void*> loadedTextures;

        bool showOptionsWindow = false;
        std::unordered_set<ImGuiModule*> imguiModules;
        std::unordered_map<FlufModule*, RegisterOptionsFunc> registeredOptionMenus;
        std::unordered_map<std::string, std::unordered_map<FlufModule*, OnRenderStatsMenu>> statMenus;
        std::string iniPath;

        struct MouseState
        {
                bool leftDown;
                bool rightDown;
                bool middleDown;
                bool mouse4Down;
                bool mouse5Down;
        };

        friend FlufUi;

#ifdef FLUF_UI
        std::unique_ptr<PlayerStatusWindow> playerStatusWindow;
        std::unique_ptr<CustomHud> customHud;
#endif
        FlufUi* flufUi;
        std::shared_ptr<FlufUiConfig> config;
        RenderingBackend backend;
        static ImGuiStyle& GenerateDefaultStyle();

        void InitSubmenus();
        void Render();
        void RenderOptionsMenu();
        static void PollInput();
        static MouseState ConvertState(DWORD state);
        static bool WndProc(FlufUiConfig* config, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    public:
        void* GetRenderingContext() const;
        ~ImGuiInterface();
        explicit ImGuiInterface(FlufUi* flufUi, RenderingBackend backend, void* context);

        /**
         * @brief Loads an image from the specified path using the currently running render backend.\n
         * Textures are automatically cached until restart.
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
        [[nodiscard]]
        FLUF_UI_API ImFont* GetImGuiFont(const std::string& fontName, int fontSize) const;

        [[nodiscard]]
        FLUF_UI_API ImFont* GetImGuiFont(const std::string& fontName, FontSize fontSize) const;

        [[nodiscard]]
        FLUF_UI_API ImFont* GetDefaultFont(int fontSize = static_cast<int>(FontSize::Default)) const;

        /**
         * @brief Register a callback that will be called when the custom options menu is visible for the module that calls it.
         * Every registered module gets its own tab within the options window, and the callback will be called when that tab is selected.
         * No ImGui state management is required beyond ensuring the ImGui stack is correct when leaving the callback.
         * @param module The module that called the register function
         * @param function A pointer to a class member function, static_cast to a RegisterOptionsFunc.
         * The function takes a boolean parameter indicating the 'save changes' button was pressed.
         * Any adjustments should not be real time and only applied when this is true.
         * @return A bool indicated successful registration. There is a limit of one menu per module.
         */
        FLUF_UI_API bool RegisterOptionsMenu(FlufModule* module, RegisterOptionsFunc function);

        /**
         * @brief Register a callback that will be called when the player status menu is visible for the module that calls it.
         * Every registered module gets its own tab within the status window, and the callback will be called when that tab is selected.
         * No ImGui state management is required beyond ensuring the ImGui stack is correct when leaving the callback.
         * @param module The module that called the register function
         * @param function A pointer to a class member function, static_cast to a RegisterStatusMenu.
         * @return A bool indicated successful registration. There is a limit of one menu per module.
         */
        FLUF_UI_API bool RegisterStatusMenu(FlufModule* module, RegisterMenuFunc function) const;

        /**
         * @brief Register a callback that will be called when the player stats menu is visible for the module that calls it.
         * Every registered module gets its own tab within the status window, and the callback will be called when that tab is selected.
         * Shared categories shall be merged together. For example, if two different modules contain the "Exploration" category, then both functions
         * will be called under the same subheading.
         * No ImGui state management is required beyond ensuring the ImGui stack is correct when leaving the callback.
         * @param module The module that called the register function
         * @param category The category of stats that the content should be rendered under
         * @param function A pointer to a class member function, static_cast to a RegisterStatusMenu.
         * @return A bool indicated successful registration. There is a limit of one menu per category per module.
         */
        FLUF_UI_API bool RegisterStatsMenu(FlufModule* module, const std::string& category, OnRenderStatsMenu function);
};
