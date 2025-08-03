#include "PCH.hpp"

#include "QolPatcher.hpp"

QolPatcher::OptionPatch::OptionPatch(const std::string& moduleName, const DWORD offset, int* optionIndex, std::initializer_list<PatchOption> patches)
    : MemoryPatch(moduleName, offset, {}), patches(patches), selectedPatch(optionIndex)
{
    patchSize = 0;
    for (const auto& [label, patch] : this->patches)
    {
        if (patchSize == 0)
        {
            patchSize = patch.size();
        }

        assert(patchSize == patch.size(), "Option Patch size must be the same among all options");
    }

    const auto module = reinterpret_cast<DWORD>(GetModuleHandleA(moduleName.empty() ? nullptr : moduleName.c_str()));
    if (!module)
    {
        return;
    }

    const auto address = module + patchOffset;
    oldData.resize(patchSize);
    patchedData.resize(patchSize);
    MemUtils::ReadProcMem(address, oldData.data(), oldData.size());
}

void QolPatcher::OptionPatch::RenderComponent()
{
    int index = *selectedPatch;

    auto currentPatch = this->patches[index];

    if (ImGui::BeginCombo("##new-opt", currentPatch.label.c_str()))
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
