#pragma once

namespace RmlWin32
{
    int GetKeyModifierState();
    Rml::Input::KeyIdentifier ConvertKey(int win32_key_code);

    // @return True if the event is still propagating, false if it was handled here.
    bool ProcessKeyDownShortcuts(Rml::Context* context, Rml::Input::KeyIdentifier key, int key_modifier, float native_dp_ratio, bool priority);
} // namespace RmlWin32
