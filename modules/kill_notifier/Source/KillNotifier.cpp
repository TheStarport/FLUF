#include "PCH.hpp"

#include "KillNotifier.hpp"

#include "ClientServerCommunicator.hpp"
#include "Fluf.hpp"
#include "FlufModule.hpp"

#include <rfl/msgpack.hpp>

BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

FlufModule::ModuleProcessCode KillNotifier::OnPayloadReceived(uint sourceClientId, const FlufPayload& payload)
{
    if (!Fluf::IsRunningOnClient() || payload.header != killMessageHeader)
    {
        return ModuleProcessCode::ContinueUnhandled;
    }

    auto result = payload.Convert<KillMessage>();
    if (result.error().has_value())
    {
        Fluf::Error("Invalid kill message payload");
        return ModuleProcessCode::Continue;
    }

    MessageBoxA(nullptr, result.value().textMessage.c_str(), "OH NO THEY KILLED KENNY!", MB_OK);
    return ModuleProcessCode::Handled;
}

KillNotifier::KillNotifier() {}

std::string_view KillNotifier::GetModuleName() { return moduleName; }

SETUP_MODULE(KillNotifier);
