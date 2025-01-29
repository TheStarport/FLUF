#include "PCH.hpp"

#include "KillNotifier.hpp"

#include "Fluf.hpp"
#include "FlufModule.hpp"

#include <rfl/msgpack.hpp>

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

FlufModule::ModuleProcessCode KillNotifier::OnPayloadReceived(uint sourceClientId, std::array<char, 4> header, void* data, size_t size)
{
    if(!Fluf::IsRunningOnClient() || header != killMessageHeader)
    {
        return ModuleProcessCode::ContinueUnhandled;
    }

    auto result = rfl::msgpack::read<KillMessage>((const char*)data, size);
    if(result.error().has_value())
    {
        Fluf::Error("Invalid kill message payload");
        return ModuleProcessCode::Continue;
    }

    MessageBoxA(nullptr, result.value().textMessage.c_str(), "OH NO THEY KILLED KENNY!", MB_OK);

    return ModuleProcessCode::Handled;
}

KillNotifier::KillNotifier()
{
}

std::string_view KillNotifier::GetModuleName() { return moduleName; }

SETUP_MODULE(KillNotifier);
