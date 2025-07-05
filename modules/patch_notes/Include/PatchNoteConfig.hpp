#pragma once

struct PatchNoteConfig
{
        static constexpr char path[] = "modules/config/patch_notes.yml";
        std::string url = "http://localhost:8000/notes.yml";
        bool sortPatchNotes = true;
};
