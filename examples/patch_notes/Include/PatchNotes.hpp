#pragma once

#include "FlufModule.hpp"
#include "ImGui/ImGuiModule.hpp"

#include <memory>

class FlufUi;

class PatchNotes final : public FlufModule, public ImGuiModule
{
        std::shared_ptr<FlufUi> flufUi;
        std::vector<PatchNote> patches;
        std::shared_ptr<PatchNoteConfig> config;

        bool showFullNotes = false;
        bool gamePaused = false;

        void LoadPatchNotesFromCache(std::string_view path);
        void RenderFullNotes();

        void OnGameLoad() override;
        void Render() override;

    public:
        static constexpr std::string_view moduleName = "Patch Notes";

        PatchNotes();
        ~PatchNotes() override;
        std::string_view GetModuleName() override;
};
