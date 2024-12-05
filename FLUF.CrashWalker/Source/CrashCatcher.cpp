#include "PCH.hpp"

#include "CrashCatcher.hpp"

#include "FLCore/Common/Archetype/Root.hpp"
#include "FLCore/Common/CObjects/CObject.hpp"
#include "FLCore/Common/CommonMethods.hpp"
#include "FLUF/Include/Fluf.hpp"
#include "Typedefs.hpp"
#include "Utils/Detour.hpp"
#include "Utils/MemUtils.hpp"
#include "Utils/StringUtils.hpp"

// Inline-ASM isn't read and it often thinks this file is full of 1-line statements
// ReSharper disable CppDFAUnreachableCode
// ReSharper disable CppDFAConstantFunctionResult
// ReSharper disable CppRedundantQualifier
// ReSharper disable CppCStyleCast

template <class... Args>
void LogInternal(const wchar_t* templateStr, Args&&... args)
{
    Fluf::Log(LogLevel::Error, StringUtils::wstos(std::vformat(templateStr, std::make_wformat_args(std::forward<Args&>(args)...))));
}

template <class... Args>
void LogInternal(const char* templateStr, Args&&... args)
{
    Fluf::Log(LogLevel::Error, std::vformat(templateStr, std::make_format_args(std::forward<Args&>(args)...)));
}

void __stdcall CrashCatcher::LogContent47bc4()
{
    Fluf::Log(LogLevel::Error, "Exception/Crash in content.dll:0xCb_47bc4 - probably missing formation in faction_props.ini/formations.ini - exiting");
    MessageBoxA(
        nullptr, "Exception/Crash in content.dll:0xCb_47bc4 - probably missing formation in faction_props.ini/formations.ini - exiting", "Exception", MB_OK);
    Sleep(1000);
    std::exit(-1);
}

CrashCatcher::FixContent47bc4::FixContent47bc4()
{
    test(eax, eax);
    jz("willCrash");
    mov(edi, eax);
    mov(edx, dword[edi]);
    mov(ecx, edi);
    ret();
    L("willCrash");
    xor_(ecx, ecx);
    ret();
}

DWORD __stdcall CrashCatcher::EngBase11A6DCatch(const BYTE* data)
{
    __try
    {
        return *reinterpret_cast<const DWORD*>(data + 0x28);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        LogInternal("Exception/Crash suppression engbase.dll:0x11A6D");
        return 0;
    }
}

const BYTE* __stdcall CrashCatcher::EngBase124BDCatch(const BYTE* data)
{
    static char cmp[256], part[256];

    __try
    {
        DWORD addr = *(DWORD*)(data + 12);
        if (addr)
        {
            addr = *(DWORD*)(addr + 4);
        }

        if (addr)
        {
            strncpy_s(cmp, (LPCSTR)addr, sizeof(cmp));
            cmp[sizeof(cmp) - 1] = '\0';
        }
        else
        {
            *cmp = '\0';
        }

        addr = *(DWORD*)(data + 8);
        if (addr)
        {
            addr = *(DWORD*)(addr + 4);
        }

        if (addr)
        {
            strncpy_s(part, (LPCSTR)addr, sizeof(part));
            part[sizeof(part) - 1] = '\0';
        }
        else
        {
            *part = '\0';
        }

        data = *(PBYTE*)(data + 16);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        LogInternal("Exception/Crash suppression engbase.dll:0x124BD");
        LogInternal("Cmp={} Part={}", cmp, part);
        data = nullptr;
    }

    return data;
}

CrashCatcher::FixEngBase11A6D::FixEngBase11A6D()
{
    push(eax);
    call(CrashCatcher::EngBase11A6DCatch);
    ret(8);
}

CrashCatcher::FixEngBase124BD::FixEngBase124BD()
{
    push(eax);
    call(CrashCatcher::EngBase124BDCatch);
    test(eax, eax);
    ret();
}

CrashCatcher::FixContent6F8B330::FixContent6F8B330(void* savePtr)
{
    mov(ptr[savePtr], ecx);
    jmp(CrashCatcher::FixContentF8B330Detour);
}

CrashCatcher::FixContent6F78DD0::FixContent6F78DD0(void* savePtr)
{
    mov(ptr[savePtr], ecx);
    jmp(CrashCatcher::FixContent6F78DD0Detour);
}

CrashCatcher::FixCommon6329B78::FixCommon6329B78()
{
    test(eax, eax);
    jz("earlyExit");
    mov(esi, ptr[eax]);
    cmp(esi, eax);
    jz("earlyExit");
    jmp(reinterpret_cast<const void*>(0x6329B7E));
    L("earlyExit");
    jmp(reinterpret_cast<const void*>(0x6329B94));
}

void CrashCatcher::CrashProc6F671A0(int arg1)
{
    try
    {
        __asm {
            pushad
            push arg1
            call [crashProc6F671A0Old]
            add esp, 4
            popad
        }
    }
    catch (...)
    {
        Fluf::Log(LogLevel::Error, std::format("Crash suppression in CrashProc6F671A0(arg1={:#X})", arg1));
    }
}

CObject* CrashCatcher::GetRoot(CObject* child)
{
    try
    {
        return ::GetRoot(child);
    }
    catch (...)
    {
        Fluf::Log(LogLevel::Error, std::format("Crash suppression in GetRoot(child={})", child->get_archetype()->archId));
        return child;
    }
}

DWORD __stdcall CrashCatcher::C4800HookNaked() { return contentModule; }

int CrashCatcher::C4800Hook(int* a1, int* a2, int* zone, double* a4, int a5, int a6)
{
    __try
    {
        int res = 0;

        __asm {
            pushad
            push a6
            push a5
            push a4
            push zone
            push a2
            push a1
            call C4800HookNaked
            add eax, 0xC4800
            call eax
            add esp, 24
            mov [res], eax
            popad
        }

        return res;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        LogInternal(L"Exception/Crash suppression content.dll:0xC608D(zone={:#8X})", zone ? *zone : 0);
        return 0;
    }
}

char __stdcall CrashCatcher::FixContentF8B330Detour(int arg1)
{
    int res = 0;
    try
    {
        __asm
        {
            pushad
            push arg1
            mov ecx, savedEcx
            call [crashProc6F8B330Old]
            mov [res], eax
            popad
        }
    }
    catch (...)
    {
        Fluf::Log(LogLevel::Error, std::format("Crash suppression in CrashProc6F8B330(arg1={:#X})", arg1));
    }

    return static_cast<char>(res);
}

void __stdcall CrashCatcher::FixContent6F78DD0Detour(int arg1, int arg2)
{
    try
    {
        __asm
        {
            pushad
            push arg2
            push arg1
            mov ecx, savedEcx
            call [crashProc6F78DD0Old]
            popad
        }
    }
    catch (...)
    {
        Fluf::Log(LogLevel::Error, std::format("Crash suppression in CrashProc6F78DD0(arg1={:#X}, arg2={:#X})", arg1, arg2));
    }
}

void CrashCatcher::PatchAlchemy()
{
    if (const auto alchemyModule = reinterpret_cast<DWORD>(GetModuleHandleA("alchemy.dll")); alchemyModule)
    {
        {
            // check for an invalid pointer (0xFFFFFFFF) passed into the method, skip processing if so. (confirmed)
            const std::array<BYTE, 16> patch = { 0x83, 0xF9, 0xFF, 0x74, 0x05, 0xE8, 0x33, 0xE7, 0xFF, 0xFF, 0x89, 0xf0, 0x5e, 0xc2, 0x04, 0x00 };
            MemUtils::WriteProcMem(alchemyModule + 0xA1D3, patch.data(), patch.size());
        }
    }
}

void CrashCatcher::UnpatchAlchemy()
{
    if (const auto alchemyModule = reinterpret_cast<DWORD>(GetModuleHandleA("alchemy.dll")); alchemyModule)
    {
        {
            // check for an invalid pointer (0xFFFFFFFF) passed into the method, skip processing if so. (confirmed)
            const std::array<BYTE, 16> patch = { 0xE8, 0x38, 0xE7, 0xFF, 0xFF, 0xF6, 0x44, 0x24, 0x08, 0x01, 0x74, 0x09, 0x56, 0xE8, 0xFB, 0xBE };
            MemUtils::WriteProcMem(alchemyModule + 0xA1D3, patch.data(), patch.size());
        }
    }
}

void CrashCatcher::PatchCommon()
{
    if (const auto commonModule = reinterpret_cast<DWORD>(GetModuleHandleA("common.dll")))
    {
        fixCommon6329B78 = std::make_unique<FixCommon6329B78>();

        {
            constexpr uchar patch[] = { 0xe9 };
            MemUtils::WriteProcMem(commonModule + 0xC9B78, patch, 1);
            MemUtils::PatchCallAddr(commonModule, 0xC9B78, (void*)fixCommon6329B78->getCode());
        }
    }
}

void CrashCatcher::PatchServer()
{
    if (const auto serverModule = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll")))
    {
        const auto hook = reinterpret_cast<FARPROC>(CrashCatcher::GetRoot);
        MemUtils::ReadProcMem(serverModule + 0x84018, &oldGetRootProc, 4);
        MemUtils::WriteProcMem(serverModule + 0x84018, &hook, 4);
    }
}

void CrashCatcher::PatchContent()
{
    if (contentModule = reinterpret_cast<DWORD>(GetModuleHandleA("content.dll")); contentModule)
    {
        fixContent47Bc4 = std::make_unique<FixContent47bc4>();
        fixContent6F8B330 = std::make_unique<FixContent6F8B330>(&savedEcx);
        fixContent6F78DD0 = std::make_unique<FixContent6F78DD0>(&savedEcx);

        // Patch for crash at content.dll + blarg
        MemUtils::PatchCallAddr(contentModule, 0xC608D, C4800Hook);

        // Patch for crash at content.dll + c458f ~ adoxa (thanks man)
        // Crash if solar has wrong destructible archetype (NewArk for
        // example is fuchu_core with hit_pts = 0 - different from
        // client and server in my case) and player taken off from
        // nearest base of this archetype (Manhattan) This is caused by
        // multiple players dying in the same planet death zone. Also
        // 000c458f error arises when nearby stations within a zone )
        // are reputed not coinciding with reputation on the
        // client-side.
        {
            // alternative: 0C458F, 8B0482->33C090
            constexpr uchar patch[] = { 0x74, 0x11, 0xeb, 0x05 };
            MemUtils::WriteProcMem(contentModule + 0xC457F, patch, 4);
        }

        // Patch for crash at content.dll + Cb_47bc4
        // This appears to be related to NPCs and/or their chatter.
        // What's missing contains the from, to and cargo entries
        // (amongst other stuff). Original Bytes: 8B F8 8B 17 8B CF
        {
            constexpr uchar patch[] = { 0x90, 0xe8 }; // nop call
            MemUtils::WriteProcMem(contentModule + 0x47bc2, patch, 2);
            MemUtils::PatchCallAddr(contentModule, 0x47bc2 + 1, (void*)fixContent47Bc4->getCode());
        }

        // Hook for crash at 0xEB4B5 (confirmed)
        const auto hook = (FARPROC)fixContent6F8B330->getCode();
        MemUtils::ReadProcMem(contentModule + 0x11C970, &crashProc6F8B330Old, 4);
        MemUtils::WriteProcMem(contentModule + 0x11C970, &hook, 4);
        MemUtils::WriteProcMem(contentModule + 0x11CA00, &hook, 4);

        // Hook for crash at 0xD8E14 (confirmed)
        crashProc6F78DD0Old = MemUtils::PatchCallAddr(contentModule, 0x5ED4B, (void*)fixContent6F78DD0->getCode());
        MemUtils::PatchCallAddr(contentModule, 0xBD96A, (void*)fixContent6F78DD0->getCode());

        // Hook for crash at 0xC71AE (confirmed)
        crashProc6F671A0Old = MemUtils::PatchCallAddr(contentModule, 0xBDC80, CrashProc6F671A0);
        MemUtils::PatchCallAddr(contentModule, 0xBDCF9, CrashProc6F671A0);
        MemUtils::PatchCallAddr(contentModule, 0xBE41C, CrashProc6F671A0);
        MemUtils::PatchCallAddr(contentModule, 0xC67E2, CrashProc6F671A0);
        MemUtils::PatchCallAddr(contentModule, 0xC6AA5, CrashProc6F671A0);
        MemUtils::PatchCallAddr(contentModule, 0xC6BE8, CrashProc6F671A0);
        MemUtils::PatchCallAddr(contentModule, 0xC6F71, CrashProc6F671A0);
        MemUtils::PatchCallAddr(contentModule, 0xC702A, CrashProc6F671A0);
        MemUtils::PatchCallAddr(contentModule, 0xC713B, CrashProc6F671A0);
        MemUtils::PatchCallAddr(contentModule, 0xC7180, CrashProc6F671A0);
    }
}

void CrashCatcher::UnpatchServer()
{
    if (const auto serverModule = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll")))
    {
        MemUtils::WriteProcMem(serverModule + 0x84018, &oldGetRootProc, 4);
    }
}
void CrashCatcher::UnpatchCommon()
{
    if (const auto commonModule = reinterpret_cast<DWORD>(GetModuleHandleA("common.dll")))
    {
        {
            constexpr uchar patch[] = { 0x8B, 0x30, 0x3B, 0xF0, 0x74 };
            MemUtils::WriteProcMem(commonModule + 0xC9B78, patch, sizeof(patch));
        }
    }
}

void CrashCatcher::UnpatchContent()
{
    if (contentModule = reinterpret_cast<DWORD>(GetModuleHandleA("content.dll")); contentModule)
    {
        {
            const uchar patch[] = { 0xe8, 0x6e, 0xe7, 0xff, 0xff };
            MemUtils::WriteProcMem(contentModule + 0xC608D, patch, 5);
        }

        {
            const uchar patch[] = { 0x8B, 0xF8, 0x8B, 0x17, 0x8B, 0xCF };
            MemUtils::WriteProcMem(contentModule + 0x47bc2, patch, 6);
        }

        MemUtils::WriteProcMem(contentModule + 0x11C970, &crashProc6F8B330Old, 4);
        MemUtils::WriteProcMem(contentModule + 0x11CA00, &crashProc6F8B330Old, 4);

        MemUtils::PatchCallAddr(contentModule, 0x5ED4B, crashProc6F78DD0Old);
        MemUtils::PatchCallAddr(contentModule, 0xBD96A, crashProc6F78DD0Old);

        MemUtils::PatchCallAddr(contentModule, 0xBDC80, crashProc6F671A0Old);
        MemUtils::PatchCallAddr(contentModule, 0xBDCF9, crashProc6F671A0Old);
        MemUtils::PatchCallAddr(contentModule, 0xBE41C, crashProc6F671A0Old);
        MemUtils::PatchCallAddr(contentModule, 0xC67E2, crashProc6F671A0Old);
        MemUtils::PatchCallAddr(contentModule, 0xC6AA5, crashProc6F671A0Old);
        MemUtils::PatchCallAddr(contentModule, 0xC6BE8, crashProc6F671A0Old);
        MemUtils::PatchCallAddr(contentModule, 0xC6F71, crashProc6F671A0Old);
        MemUtils::PatchCallAddr(contentModule, 0xC702A, crashProc6F671A0Old);
        MemUtils::PatchCallAddr(contentModule, 0xC713B, crashProc6F671A0Old);
        MemUtils::PatchCallAddr(contentModule, 0xC7180, crashProc6F671A0Old);
    }
}

CrashCatcher::CrashCatcher()
{
    PatchContent();
    PatchServer();
    PatchCommon();
    PatchAlchemy();

    static FixEngBase11A6D fixEngBase11A6D;
    static FixEngBase124BD fixEngBase124BD;

    const auto engBaseModule = reinterpret_cast<DWORD>(GetModuleHandleA("engbase.dll"));

    // Patch for crash at engbase.dll + 0x0124BD ~ adoxa (thanks
    // man) This is caused by a bad cmp.
    constexpr uchar patch[] = { 0xe8 };
    MemUtils::WriteProcMem(engBaseModule + 0x0124BD, patch, 1);
    MemUtils::PatchCallAddr(engBaseModule, 0x0124BD, (void*)fixEngBase124BD.getCode());

    // Patch for crash at engbase.dll + 0x011a6d
    // This is caused by a bad cmp I suspect
    constexpr uchar patch2[] = { 0x90, 0xe9 }; // nop jmpr
    MemUtils::WriteProcMem(engBaseModule + 0x011a6d, patch2, 2);
    MemUtils::PatchCallAddr(engBaseModule, 0x011a6d + 1, (void*)fixEngBase11A6D.getCode());
}

CrashCatcher::~CrashCatcher()
{
    const auto engBaseModule = reinterpret_cast<DWORD>(GetModuleHandleA("engbase.dll"));

    if (engBaseModule)
    {
        const uchar patch[] = { 0x8B, 0x40, 0x10, 0x85, 0xc0 };
        MemUtils::WriteProcMem(engBaseModule + 0x0124BD, patch, 5);

        const uchar patch2[] = { 0x8B, 0x40, 0x28, 0xC2, 0x08, 0x00 };
        MemUtils::WriteProcMem(engBaseModule + 0x011a6d, patch2, 6);
    }

    UnpatchCommon();
    UnpatchAlchemy();
}
