#include "PCH.hpp"

#include "ClientServerCommunicator.hpp"

#include "FlufModule.hpp"
#include "Internal/Hooks/InfocardOverrides.hpp"
#include "Utils/MemUtils.hpp"

void ClientServerCommunicator::OnReceiveChatMessageClient(uint sourceClientId, const uint destClientId, const size_t size, char* data)
{
    auto payload = FlufPayload::FromPayload(data, size);
    if (!payload)
    {
        oldSendChat(sourceClientId, destClientId, size, data);
        return;
    }

    if (InfocardOverrides::HandlePayload(*payload))
    {
        return;
    }

    Fluf::instance->CallModuleEvent(&FlufModule::OnPayloadReceived, sourceClientId, *payload);
}

void ClientServerCommunicator::OnReceiveChatMessageServer(uint sourceClientId, size_t size, char* data, uint destTargetId, int unknown)
{
    const auto payload = FlufPayload::FromPayload(data, size);
    if (!payload)
    {
        oldSubmitChat(sourceClientId, size, data, destTargetId, unknown);
        return;
    }

    if (InfocardOverrides::HandlePayload(*payload))
    {
        return;
    }

    Fluf::instance->CallModuleEvent(&FlufModule::OnPayloadReceived, sourceClientId, *payload);
}

ClientServerCommunicator::ClientServerCommunicator()
{
    if (Fluf::IsRunningOnClient())
    {
        MemUtils::ReadProcMem(clientReceivePayloadAddress, &oldSendChat, sizeof oldSendChat);
        const auto proc = reinterpret_cast<FARPROC>(&ClientServerCommunicator::OnReceiveChatMessageClient);
        MemUtils::WriteProcMem(clientReceivePayloadAddress, &proc, sizeof(proc));
    }
    else
    {
        const auto server = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandleA("server.dll"), "Server"));
        const auto submitChat = server - 0x08;

        MemUtils::ReadProcMem(submitChat, &oldSubmitChat, sizeof(oldSubmitChat));
        const auto newSubmitChat = &ClientServerCommunicator::OnReceiveChatMessageServer;
        MemUtils::WriteProcMem(submitChat, &newSubmitChat, sizeof(newSubmitChat));
    }
}

ClientServerCommunicator::~ClientServerCommunicator() = default;
