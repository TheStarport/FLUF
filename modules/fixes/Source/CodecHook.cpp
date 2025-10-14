#include "PCH.hpp"

#include "Fixes.hpp"
#include "Utils/MemUtils.hpp"

// ReSharper disable once CppDeclaratorNeverUsed
static int priority;
static DWORD returnAddress;

int __fastcall CodecEnumCompare(const char* desc)
{
    static std::array<char, 460> name;
    static DWORD userCodec = -1;

    if (userCodec == -1)
    {
        userCodec = GetEnvironmentVariableA("FL_MP3", name.data(), name.size());
    }

    if (userCodec && memcmp(desc, name.data(), userCodec + 1) == 0)
    {
        return 0;
    }

    if (memcmp(desc, "Fraunhofer IIS MPEG Layer-3 Codec", 33) == 0)
    {
        // There's probably only one installed, but just in case.
        if (memcmp(desc + 33, " (professional)", 16) == 0)
        {
            return 1;
        }

        if (memcmp(desc + 33, " (advanced plus)", 17) == 0)
        {
            return 2;
        }

        if (memcmp(desc + 33, " (advanced)", 12) == 0)
        {
            return 3;
        }

        if (memcmp(desc + 33, " (basic)", 9) == 0)
        {
            return 4;
        }

        if (memcmp(desc + 33, " (lite)", 8) == 0)
        {
            return 5;
        }

        if (memcmp(desc + 33, " (decode only)", 15) == 0)
        {
            return 6;
        }
        return 7;
    }

    return 999;
}

void __declspec(naked) AudioCodecHook()
{
    __asm
        {
        cmp	dword ptr [edi+4], 0
        jne	e1
        inc	dword ptr [edi+4]
        mov	priority, 999
      e1:
        mov	ecx, esi
        call CodecEnumCompare
        cmp	eax, priority
        jae	done
        mov	priority, eax
        mov	[edi+4], ebp
      done:
        jmp	returnAddress
        align	16
        }
}

void Fixes::PatchAudioCodec()
{
    auto sm = reinterpret_cast<DWORD>(GetModuleHandleA("SoundManager.dll"));
    auto ss = reinterpret_cast<DWORD>(GetModuleHandleA("SoundStreamer.dll"));

    if (MemUtils::IsRunningOnWine())
    {
        MemUtils::NopAddress(ss + 0x1016, 33);
        MemUtils::NopAddress(sm + 0x8676, 33);
    }

    const auto soundManagerAddress = reinterpret_cast<DWORD>(GetModuleHandleA("SoundManager.dll")) + (0x6ee8ca0 - 0x6ee0000);
    const auto soundStreamerAddress = reinterpret_cast<DWORD>(GetModuleHandleA("SoundStreamer.dll")) + (0x6ef3f50 - 0x6ef0000);

    returnAddress = soundManagerAddress + 0x2d;

    MemUtils::PatchAssembly(soundManagerAddress, AudioCodecHook);
    MemUtils::PatchAssembly(soundStreamerAddress, AudioCodecHook);
}
