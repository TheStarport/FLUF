#include "PCH.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "KeyManager.hpp"

#include "Utils/MemUtils.hpp"

#include <magic_enum.hpp>

static KeyState currentKeyState;

bool KeyManager::HandleKey(const Key key)
{
    if (const auto overriddenKey = overriddenKeys.find(key); overriddenKey != overriddenKeys.end())
    {
        const auto module = Fluf::GetModule(overriddenKey->second.moduleName);
        if (module.expired())
        {
            return false;
        }

        return (module.lock().get()->*overriddenKey->second.function)(currentKeyState);
    }

    return false;
}

void __declspec(naked) KeyManager::HandleKeyNaked()
{
    __asm
    {
        push ecx
        mov ecx, KeyManager::instance
        push[esp + 8]
        call KeyManager::HandleKey
        pop ecx
        test al, al
        jz   cmd_not_processed
        ret

        cmd_not_processed:
            sub esp, 80h
            push ebx
            push 0x00576417
            ret

    }
}

using MessagePumpKeyHandler = bool (*)(int keyState, int a2, int a3);
FunctionDetour messagePumpKeyHandlerDetour{ reinterpret_cast<MessagePumpKeyHandler>(0x577850) };
bool MessagePumpKeyHandlerDetour(const int keyState, const int a2, const int a3)
{
    currentKeyState = keyState == 0x100 ? KeyState::Pressed : KeyState::Released;

    messagePumpKeyHandlerDetour.UnDetour();
    const auto res = messagePumpKeyHandlerDetour.GetOriginalFunc()(keyState, a2, a3);
    messagePumpKeyHandlerDetour.Detour(MessagePumpKeyHandlerDetour);

    return res;
}

KeyManager::KeyManager()
{
    Fluf::Log(LogLevel::Debug, "Setting up KeyManager hooks");
    instance = this;

    std::array<byte, 7> patch = { 0xB9, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE1 };
    const auto address = reinterpret_cast<PDWORD>(reinterpret_cast<char*>(&patch) + 1);
    *address = reinterpret_cast<DWORD>(HandleKeyNaked);
    MemUtils::WriteProcMem(0x576410, patch.data(), patch.size());

    messagePumpKeyHandlerDetour.Detour(MessagePumpKeyHandlerDetour);
}

KeyManager::~KeyManager() = default;

void KeyManager::RegisterKey(FlufModule* module, std::string_view newName, Key key, const KeyFunc function, const bool suppressWarnings)
{
    if (const auto overriddenKey = overriddenKeys.find(key); overriddenKey != overriddenKeys.end())
    {
        if (const auto moduleName = module->GetModuleName(); overriddenKey->second.moduleName != moduleName)
        {
            const std::string message = std::format("Attempting to register key '{}' which has registered by another module under the name {}\n"
                                                    "Either modify this module or the previous one, or remove one of them from the module load order.",
                                                    magic_enum::enum_name(key),
                                                    overriddenKey->second.moduleName);
            MessageBoxA(nullptr, message.c_str(), "Key Already Registered!", MB_OK);
            std::exit(1);
        }
    }

    if (!suppressWarnings && !overrideableKeys.contains(key))
    {
        Fluf::Log(LogLevel::Warn, std::format("Key '{}' is not an overridable key. Some existing functionality may be lost!", magic_enum::enum_name(key)));
    }

    auto moduleName = module->GetModuleName();
    Fluf::Log(LogLevel::Info, std::format("({}) Registering key '{}' under new name '{}'", moduleName, magic_enum::enum_name(key), newName));

    // Replace the INI name mapping (e.g. USER_BUTTON -> FLUF_OPEN_UI)
    const auto newNameAddress = 0x614DD8 + static_cast<ulong>(key) * 4;
    *reinterpret_cast<const char**>(newNameAddress) = newName.data();

    overriddenKeys[key] = { std::string(moduleName), function };
}

bool KeyManager::UnregisterKey(const Key key) { return overriddenKeys.erase(key) == 1; }
