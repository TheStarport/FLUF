#include "PCH.hpp"

#include "FLUF.CrashWalker.hpp"
#include "FLUF/Include/Fluf.hpp"

#include <curl/curl.h>
#include <rfl/Result.hpp>
#include <rfl/json.hpp>
#include <tchar.h>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

FlufCrashWalker* module;

static BOOL PreventSetUnhandledExceptionFilter()
{
    const auto hKernel32 = LoadLibraryA("kernel32.dll");
    if (hKernel32 == nullptr)
    {
        return 0;
    }

    void* pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
    if (pOrgEntry == nullptr)
    {
        return 0;
    }

    // Code for x86:
    // 33 C0                xor         eax,eax
    // C2 04 00             ret         4
    unsigned char executePatch[] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };

    DWORD dwOldProtect = 0;
    BOOL bProt = VirtualProtect(pOrgEntry, sizeof executePatch, PAGE_EXECUTE_READWRITE, &dwOldProtect);

    SIZE_T bytesWritten = 0;
    const BOOL ret = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, executePatch, sizeof executePatch, &bytesWritten);

    if (bProt != false && dwOldProtect != PAGE_EXECUTE_READWRITE)
    {
        DWORD dwBuf;
        VirtualProtect(pOrgEntry, sizeof executePatch, dwOldProtect, &dwBuf);
    }

    return ret;
}

static long __stdcall CrashHandlerExceptionFilter(EXCEPTION_POINTERS* pExPtrs)
{
    if (pExPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
    {
        static char MyStack[1024 * 128]; // be sure that we have enough space...
        // it assumes that DS and SS are the same!!! (this is the case for Win32)
        // change the stack only if the selectors are the same (this is the case for Win32)
        //__asm push offset MyStack[1024*128];
        //__asm pop esp;
        __asm mov eax, offset MyStack[1024 * 128];
        __asm mov esp, eax;
    }

    FlufWalker sw;

    sw.ShowCallstack(GetCurrentThread(), pExPtrs->ContextRecord);
    std::stringstream str;
    LPCSTR caption = "Fatal Crash. Unknown Error Code";

    auto exOffset = reinterpret_cast<size_t>(pExPtrs->ExceptionRecord->ExceptionAddress);

    // Extract dll name
    std::string dllName;
    HMODULE dll;
    MODULEINFO mod;
    if (RtlPcToFileHeader(reinterpret_cast<void*>(exOffset), reinterpret_cast<void**>(&dll)))
    {
        CHAR maxPath[MAX_PATH];
        if (GetModuleFileNameA(dll, maxPath, MAX_PATH))
        {
            const std::string path = maxPath;
            dllName = path.substr(path.find_last_of('\\') + 1);
            GetModuleInformation(GetCurrentProcess(), dll, &mod, sizeof(MODULEINFO));
        }
    }

    // Couldn't even get the DLL name, what now?
    if (dllName.empty())
    {
        Fluf::Log(LogLevel::Trace, "Unable to extract dll name during exception filtering. Likely a datasection? " + std::to_string(exOffset));
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Lookup the error!
    auto error = module->FindError(dllName, exOffset);
    if (!error)
    {
        str << "\tUnhandled Exception!\n\t-- Important Information --\n"
            << std::endl
            << "\tSource: " << dllName << std::endl
            << "\tRelExpAddr: 0x" << reinterpret_cast<size_t>(mod.lpBaseOfDll) + mod.SizeOfImage - exOffset << std::endl
            << "\tExpCode: 0x" << pExPtrs->ExceptionRecord->ExceptionCode << std::endl
            << "\tExpFlags: " << pExPtrs->ExceptionRecord->ExceptionFlags << std::endl
            << "\tExpAddress: 0x" << exOffset << std::endl
            << "\tIf you discover how to reproduce this and the reason, report this to Starport!" << std::endl;

        MessageBoxA(nullptr, str.str().c_str(), caption, MB_ICONWARNING | MB_OK);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // clang-format off
    str << "Source: " << dllName <<
        "\nFaulting Offset: " << std::hex << error->offset.value() <<
        "\nSuspected Reason: " << error->description <<
        "\nFound by: " << error->author << std::endl;
    // clang-format on

    MessageBoxA(nullptr, str.str().c_str(), caption, MB_ICONWARNING | MB_OK);
    return EXCEPTION_CONTINUE_SEARCH;
}

FunctionDetour loadLibraryDetour(LoadLibraryA);
BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);
    return TRUE;
}

HINSTANCE __stdcall FlufCrashWalker::LoadLibraryDetour(const char* libName)
{

    loadLibraryDetour.UnDetour();
    const auto res = LoadLibraryA(libName);
    loadLibraryDetour.Detour(LoadLibraryDetour);

    // Successfully loaded, lets check what the str was
    if (res)
    {
        if (_strcmpi(libName, "server.dll") == 0 && crashCatcher)
        {
            CrashCatcher::PatchServer();
        }
        else if (_strcmpi(libName, "content.dll") == 0 && crashCatcher)
        {
            CrashCatcher::PatchContent();
        }
    }

    return res;
}

void FlufCrashWalker::OnGameLoad()
{
    crashCatcher = std::make_unique<CrashCatcher>();

    // Download latest crash payload
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        return;
    }

    // Reserve 100KB of space for our json payload. It's 33KB at the time of writing, so 100KB should always be enough.
    std::array<char, 1024 * 100> buffer{};
    const auto url = "https://raw.githubusercontent.com/TheStarport/KnowledgeBase/refs/heads/master/static/payloads/crash-offsets.json";
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer.data());
    const CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        Fluf::Log(LogLevel::Error, std::format("Curl error: {}", curl_easy_strerror(res)));
        return;
    }

    auto result = rfl::json::read<std::vector<ErrorPayload>>(std::string(buffer.data(), buffer.size()));
    if (result.error().has_value())
    {
        Fluf::Log(LogLevel::Error, std::format("Error reading payload: {}", result.error().value().what()));
        return;
    }

    possibleErrors = result.value();

    // Now we have our payload we setup our exception handler
    SetUnhandledExceptionFilter(CrashHandlerExceptionFilter);
    PreventSetUnhandledExceptionFilter();
}

FlufCrashWalker::FlufCrashWalker()
{
    loadLibraryDetour.Detour(LoadLibraryDetour);
    module = this;
}

FlufCrashWalker::~FlufCrashWalker() = default;

std::string_view FlufCrashWalker::GetModuleName() { return moduleName; }

ErrorPayload* FlufCrashWalker::FindError(std::string_view module, size_t offset)
{
    for (auto& error : possibleErrors)
    {
        if (_strcmpi(error.moduleName.c_str(), module.data()) == 0 && error.offset.value() == offset)
        {
            return &error;
        }
    }

    return nullptr;
}

SETUP_MODULE(FlufCrashWalker);
