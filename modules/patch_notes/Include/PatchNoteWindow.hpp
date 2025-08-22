#pragma once
#include "PatchNote.hpp"
#include "ImGui/FlWindow.hpp"

class PatchNotes;
class PatchNoteWindow final : public FlWindow
{
        friend PatchNotes;
        void RenderWindowContents() override;
        std::vector<PatchNote>* patches;
        ImGuiInterface* interface;
        bool* openState;

    public:
        PatchNoteWindow(ImGuiInterface* interface, std::vector<PatchNote>* patches, bool* openState);
};
