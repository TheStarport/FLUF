#pragma once

struct PatchNoteConfig
{
    static constexpr char path[] = "patch_notes.yml";
    std::string url = "http://localhost:8000/notes.yml";
    bool sortPatchNotes = true;
};