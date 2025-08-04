#include "PCH.hpp"

#include "QolPatcher.hpp"

QolPatcher::OptionPatch::OptionPatch(const std::string& moduleName, const std::initializer_list<DWORD> offsets, const size_t patchSize, int* optionIndex,
                                     const std::initializer_list<PatchOption> patches)
    : MemoryPatch(moduleName, offsets, patchSize, {}), patches(patches), selectedPatch(optionIndex), patchSize(patchSize)
{
    for (const auto& [label, patch] : this->patches)
    {
        assert(patchSize == patch.size(), "Option Patch size must be the same among all options");
    }

    const auto& patch = this->patches[*selectedPatch];
    memcpy(patchedData.data(), patch.data.data(), patch.data.size());
}

void QolPatcher::OptionPatch::RenderComponent()
{
    int index = *selectedPatch;

    const auto [label, data] = this->patches[index];

    if (ImGui::BeginCombo("##new-opt", label.c_str()))
    {
        for (int i = 0; i < patches.size(); ++i)
        {
            auto& [label, data] = patches[i];
            const bool isSelected = index == i;
            if (ImGui::Selectable(std::format("{}##opt", label).c_str(), isSelected))
            {
                index = i;
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (index != *selectedPatch)
    {
        *selectedPatch = index;
        auto& [label, data] = patches[index];
        memcpy(patchedData.data(), data.data(), patchSize);
    }
}
