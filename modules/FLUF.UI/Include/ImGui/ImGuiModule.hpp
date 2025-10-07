#pragma once

#include <imgui.h>

class ImGuiModule
{
    protected:
        static constexpr ImGuiWindowFlags defaultHeadlessWindowsFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration |
                                                                        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoFocusOnAppearing |
                                                                        ImGuiWindowFlags_NoBringToFrontOnFocus;

    public:
        virtual ~ImGuiModule() = default;
        virtual void Render() = 0;
};
