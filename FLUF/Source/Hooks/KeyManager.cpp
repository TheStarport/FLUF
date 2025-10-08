#include "PCH.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"
#include "KeyManager.hpp"

#include "Utils/MemUtils.hpp"

#include <magic_enum.hpp>
#include <Utils/StringUtils.hpp>

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

void KeyManager::GenerateKeyMap()
{
    userKeyMap.clear();

    struct InternalKeyMap
    {
            DWORD a3;
            DWORD a4;
            DWORD a5;
            DWORD a6;
            DWORD a7;
            DWORD a8;
            DWORD a9;
    };

    auto* itemList = reinterpret_cast<st6::list<InternalKeyMap>*>(0x67C254);
    const auto userKeyStringMap = reinterpret_cast<const char**>(0x614DD8);

    for (const auto& key : *itemList)
    {
        if (key.a8 && (key.a9 - key.a8) >> 3)
        {
            auto nickname = key.a3 >= 0xCC ? "USER_NONE" : userKeyStringMap[key.a3];

            if (key.a3)
            {
                for (auto i = reinterpret_cast<DWORD*>(key.a8); i != reinterpret_cast<DWORD*>(key.a9); i += 2)
                {
                    KeyMapping::KeyMod mod;
                    switch (const auto existingMod = static_cast<KeyMapping::KeyMod>(i[1]))
                    {
                        case KeyMapping::KeyMod::SHIFT:
                        case KeyMapping::KeyMod::CTRL:
                        case KeyMapping::KeyMod::ALT:
                            {
                                mod = existingMod;
                                break;
                            }
                        default:;
                    }

                    Fluf::Info(std::format("nickname: {}, mod: {}", nickname, (int)mod));
                    userKeyMap.emplace_back(nickname, mod, i[0]);
                }
            }
        }
    }
}

void __declspec(naked) KeyManager::OnKeyMapLoad()
{
    static constexpr DWORD returnAddress = 0x576B60;
    __asm
    {
        push ecx
        mov ecx, KeyManager::instance
        call KeyManager::GenerateKeyMap
        pop ecx
        jmp returnAddress
    }
}

void __declspec(naked) KeyManager::OnKeyMapSave()
{
    static constexpr DWORD returnAddress = 0x5772D0;
    __asm
    {
        push ecx
        mov ecx, KeyManager::instance
        call KeyManager::GenerateKeyMap
        pop ecx
        jmp returnAddress
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

    // When key map is loaded/saved in Freelancer, populate our internal keymap
    MemUtils::PatchCallAddr(GetModuleHandleA(nullptr), 0x17750D, OnKeyMapLoad);
    MemUtils::PatchCallAddr(GetModuleHandleA(nullptr), 0xAC230, OnKeyMapSave);
}

KeyManager::~KeyManager() = default;

const std::vector<KeyMapping>& KeyManager::GetKeyMap() const { return userKeyMap; }

std::string KeyManager::TranslateKeyMapping(const KeyMapping& mapping)
{
    using TranslateKeyInternal = void (*)(int* block, wchar_t* buffer, int a256);
    static auto translateKeyInternal = reinterpret_cast<TranslateKeyInternal>(0x577F50);

    int a[3]{ 0, mapping.virtualKey, static_cast<int>(mapping.mod) };
    wchar_t buffer[256];
    translateKeyInternal(a, buffer, sizeof(buffer));
    const std::string output = StringUtils::wstos(buffer);

    using namespace std::string_view_literals;
    return StringUtils::ReplaceStr(output, "KP_"sv, "Num "sv);
}

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
