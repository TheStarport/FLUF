#pragma once

#include "FLCore/FLCoreServer.h"
#include "Fluf.hpp"

#include <rfl/msgpack.hpp>
#include <zstd.h>

struct IChatServer
{
        virtual void SubmitChat(uint sourceClientId, size_t payloadSize, void* buffer, uint clientId, uint unknown);
};

constexpr USHORT flufHeader = 0xF10F;
struct FlufPayload
{
        bool compressed{};
        std::string header;
        std::vector<char> data;

        template <typename T>
        rfl::Result<T> Convert() const
        {
            if (!compressed)
            {
                return rfl::msgpack::read<T>(data.data(), data.size());
            }

            const auto compressedSize = ZSTD_getFrameContentSize(data.data(), data.size());
            std::vector<char> uncompressedData(compressedSize);
            ZSTD_decompress(uncompressedData.data(), compressedSize, data.data(), data.size());

            return rfl::msgpack::read<T>(uncompressedData.data(), uncompressedData.size());
        }

        [[nodiscard]]
        std::vector<char> ToBytes() const
        {
            std::vector<char> bytes;
            bytes.resize(sizeof(flufHeader) + 1 + header.size() + sizeof(compressed) + data.size());

            auto ptr = bytes.data();
            const auto size = sizeof(flufHeader);
            memcpy_s(ptr, bytes.size(), &flufHeader, size);

            // Write our string header
            ptr += sizeof(flufHeader);
            auto newSize = bytes.size() - size - 1;

            *ptr = static_cast<byte>(header.size());
            ++ptr;
            memcpy_s(ptr, newSize, header.data(), header.size());

            // Write compressed boolean
            newSize -= header.size();
            ptr += header.size();
            memcpy_s(ptr, newSize, &compressed, sizeof(bool));

            --newSize;
            ++ptr;
            memcpy_s(ptr, newSize, data.data(), data.size());

            return bytes;
        }

        static std::optional<FlufPayload> FromPayload(char* data, size_t size)
        {
            // Check if enough room for the fluf header and the body, and that the header matches
            if (size < sizeof(flufHeader) + sizeof(compressed) + 2 || *reinterpret_cast<ushort*>(data) != flufHeader)
            {
                return std::nullopt;
            }

            const char* ptr = data + sizeof(flufHeader);
            const auto headerSize = *ptr;
            ptr++;

            if (sizeof(flufHeader) + sizeof(compressed) + 1 + headerSize > size)
            {
                return std::nullopt;
            }

            FlufPayload payload;
            payload.header.resize(headerSize);

            memcpy_s(payload.header.data(), headerSize, ptr, headerSize);
            ptr += headerSize;

            memcpy_s(&payload.compressed, sizeof(payload.compressed), ptr, sizeof(payload.compressed));
            ptr += sizeof(payload.compressed);

            const size_t newSize = size - (ptr - data);
            payload.data.resize(newSize);
            memcpy_s(payload.data.data(), newSize, ptr, newSize);

            return payload;
        }

        template <typename T>
        static FlufPayload ToPayload(const T& data, const std::string_view header)
        {
            if (header.size() > 255)
            {
                throw std::runtime_error("Header size cannot be bigger than one byte.");
            }

            FlufPayload payload;
            payload.header.resize(header.size());
            memcpy_s(payload.header.data(), payload.header.size(), header.data(), header.size());

            auto msgPack = rfl::msgpack::write<T>(data);

            const size_t maxPossibleSize = ZSTD_compressBound(msgPack.size());
            payload.data.resize(maxPossibleSize);
            const size_t newSize = ZSTD_compress(payload.data.data(), maxPossibleSize, msgPack.data(), msgPack.size(), 6);

            if (newSize > msgPack.size())
            {
                payload.data = msgPack;
                payload.compressed = false;
                return payload;
            }

            payload.compressed = true;
            payload.data.resize(newSize); // Cut down to exact size
            return payload;
        }

    private:
        FlufPayload() = default;
};

class Fluf;
class InfocardOverrides;
class ClientServerCommunicator
{
        using ServerSendChat = void(__stdcall*)(uint sourceId, uint targetId, size_t size, const char* data);
        using ClientSendChat = void(__stdcall*)(uint sourceClientId, size_t size, const char* data, uint destTargetId, int unknown);

        friend Fluf;
        static constexpr DWORD clientReceivePayloadAddress = 0x5E6668;
        inline static ClientSendChat oldSubmitChat;
        inline static ServerSendChat oldSendChat;

        IChatServer* clientChatServer = nullptr;

    public:
        static void __stdcall OnReceiveChatMessageClient(uint sourceClientId, uint destClientId, size_t size, char* data);
        static void __stdcall OnReceiveChatMessageServer(uint sourceClientId, size_t size, char* data, uint destTargetId, int unknown);
        template <typename T>
        bool SendPayloadFromClient(std::string_view header, const T& payload, const uint targetId = 0x10000)
        {
            if (!clientChatServer)
            {
                return false;
            }

            auto flufData = FlufPayload::ToPayload(payload, header);
            auto data = flufData.ToBytes();
            clientChatServer->SubmitChat(*reinterpret_cast<uint*>(0x673344), data.size(), data.data(), targetId, -1);

            return true;
        }

        template <typename T>
        bool SendPayloadFromServer(const uint targetId, std::string_view header, const T& payload)
        {
            if (Fluf::IsRunningOnClient())
            {
                return false;
            }

            auto flufData = FlufPayload::ToPayload(payload, header);
            auto data = flufData.ToBytes();
            static auto sendChat = reinterpret_cast<ServerSendChat>(reinterpret_cast<DWORD>(GetModuleHandleA("remoteclient.dll")) + 0x3BB80);
            sendChat(*reinterpret_cast<uint*>(0x673344), targetId, data.size(), data.data());

            return true;
        }

        ClientServerCommunicator();
        ~ClientServerCommunicator();
};
