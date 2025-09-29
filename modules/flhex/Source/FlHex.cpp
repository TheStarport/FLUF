#include "PCH.hpp"

#include <shellapi.h>
#include "Fluf.hpp"
#include "FlHex.hpp"

#include "Utils/MemUtils.hpp"
#include "Utils/StringUtils.hpp"

const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

void FlHex::OnDllLoaded(const std::string_view dllName, HMODULE dllPtr) { Patch(false, dllName); }

void FlHex::Patch(const bool restore, const std::string_view loadedModuleName)
{
    for (auto& module : hexEdits)
    {
        if (!loadedModuleName.empty() && _strcmpi(module.first.c_str(), loadedModuleName.data()) != 0)
        {
            continue;
        }

        const char* moduleName = _strcmpi(module.first.c_str(), "freelancer.exe") == 0 ? nullptr : module.first.c_str();
        const auto moduleHandle = reinterpret_cast<DWORD>(GetModuleHandleA(moduleName));
        if (!moduleHandle)
        {
            continue;
        }

        for (auto& [description, offset, bytes, originalBytes] : module.second)
        {
            Fluf::Log(LogLevel::Debug,
                      std::format("{} {}", restore ? "Reverting" : "Applying", description.has_value() ? description.value() : "(unknown patch)"));
            const auto relOfs = offset.value() + moduleHandle;

            if (!restore)
            {
                std::vector<byte> buffer;
                buffer.reserve(buffer.size());
                for (const auto& byte : bytes)
                {
                    buffer.emplace_back(static_cast<BYTE>(byte.value()));
                }

                auto& restorationBuffer = originalBytes.value();
                restorationBuffer.resize(buffer.size());

                MemUtils::ReadProcMem(relOfs, restorationBuffer.data(), buffer.size());
                MemUtils::WriteProcMem(relOfs, buffer.data(), buffer.size());
            }
            else
            {
                auto& restorationBuffer = originalBytes.value();
                restorationBuffer.reserve(restorationBuffer.size());
                MemUtils::WriteProcMem(relOfs, restorationBuffer.data(), restorationBuffer.size());
            }
        }
    }
}

FlHex::FlHex()
{
    const auto loadedHexEdits = ConfigHelper<decltype(hexEdits)>::Load(configPath);
    if (!loadedHexEdits.has_value())
    {
        return;
    }

    hexEdits = *loadedHexEdits;
    Patch(false);
}

FlHex::~FlHex() { Patch(true); };

std::string_view FlHex::GetModuleName() { return moduleName; }

SETUP_MODULE(FlHex);
