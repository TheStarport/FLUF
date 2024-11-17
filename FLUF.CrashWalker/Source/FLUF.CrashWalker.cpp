#include "PCH.hpp"

#include "FLUF.CrashWalker.hpp"

#include "FLCore/Common/CommonMethods.hpp"
#include "FLUF/Include/Fluf.hpp"
#include "Utils/MemUtils.hpp"

#include <curl/curl.h>
#include <minidumpapiset.h>
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

long __stdcall FlufCrashWalker::CrashHandlerExceptionFilter(EXCEPTION_POINTERS* exceptionPointers)
{
    if (exceptionPointers->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
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

    sw.ShowCallstack(GetCurrentThread(), exceptionPointers->ContextRecord);
    std::stringstream str;

    auto exOffset = reinterpret_cast<size_t>(exceptionPointers->ExceptionRecord->ExceptionAddress);

    // Extract dll name
    std::string dllName;
    HMODULE dll;
    MODULEINFO mod;
    if (RtlPcToFileHeader(reinterpret_cast<void*>(exOffset), reinterpret_cast<void**>(&dll)))
    {
        if (CHAR maxPath[MAX_PATH]; GetModuleFileNameA(dll, maxPath, MAX_PATH))
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
    size_t relativeAddress = reinterpret_cast<size_t>(mod.lpBaseOfDll) + mod.SizeOfImage - exOffset;
    auto error = module->FindError(dllName, relativeAddress);
    if (!error)
    {
        std::array<char, MAX_PATH> totalPath{};
        GetUserDataPath(totalPath.data());

        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        auto dumpPath =
            config->useOnlySingleDumpFile ? std::format("{}/crash.dmp") : std::format("{}/{:%Y-%m-%d %H.%M.%S}.dmp", std::string(totalPath.data()), time);

        if (HANDLE file = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            file != INVALID_HANDLE_VALUE)
        {
            MINIDUMP_EXCEPTION_INFORMATION dumpInformation;
            dumpInformation.ThreadId = GetCurrentThreadId();
            dumpInformation.ExceptionPointers = exceptionPointers;
            dumpInformation.ClientPointers = 0;

            auto flags =
                config->miniDumpFlags > 0 ? config->miniDumpFlags : MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithModuleHeaders;
            if (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, static_cast<MINIDUMP_TYPE>(flags), &dumpInformation, nullptr, nullptr) != 0)
            {
                CloseHandle(file);
            }
            else
            {
                CloseHandle(file);
                DeleteFileA(dumpPath.c_str());
            }
        }

        str << "\tUnhandled Exception!\n\t-- Important Information --\n"
            << std::endl
            << "\tSource: " << dllName << std::endl
            << "\tRelExpAddr: 0x" << std::hex << relativeAddress << std::endl
            << "\tExpCode: 0x" << exceptionPointers->ExceptionRecord->ExceptionCode << std::endl
            << "\tExpFlags: " << exceptionPointers->ExceptionRecord->ExceptionFlags << std::endl
            << "\tExpAddress: 0x" << exOffset << std::endl
            << "\tIf you discover how to reproduce this and the reason, report this to Starport!" << std::endl;

        MessageBoxA(nullptr, str.str().c_str(), "Fatal Crash. Unknown Error Code", MB_ICONWARNING | MB_OK);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // clang-format off
    str << "Source: " << dllName <<
        "\nFaulting Offset: 0x" << std::hex << error->offset.value() <<
        "\nFound by: " << error->author <<
        "\nSuspected Reason: " << error->description << std::endl;
    // clang-format on

    MessageBoxA(nullptr, str.str().c_str(), "Fatal Crash. Known Error Code", MB_ICONWARNING | MB_OK);
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

struct MemoryBuffer
{
        char* data;
        size_t size;
};

size_t DownloadCallback(void* ptr, size_t size, size_t nmemb, void* data)
{
    const size_t totalSize = size * nmemb;
    auto* mem = static_cast<MemoryBuffer*>(data);

    mem->data = static_cast<char*>(realloc(mem->data, mem->size + totalSize + 1));
    if (mem->data == nullptr)
    {
        // Memory allocation failed
        Fluf::Log(LogLevel::Error, "Not enough memory (realloc failed)");
        return 0;
    }

    memcpy(&mem->data[mem->size], ptr, totalSize);
    mem->size += totalSize;
    mem->data[mem->size] = 0; // Null-terminate the string

    return totalSize;
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
    MemoryBuffer buffer{};
    const auto url = "https://raw.githubusercontent.com/TheStarport/KnowledgeBase/refs/heads/master/static/payloads/crash-offsets.json";
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
    const CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        free(buffer.data);
        Fluf::Log(LogLevel::Error, std::format("Curl error: {}", curl_easy_strerror(res)));
        return;
    }

    auto result = rfl::json::read<std::vector<ErrorPayload>>(std::string(buffer.data, buffer.size));
    free(buffer.data);
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
    module = this;
    loadLibraryDetour.Detour(LoadLibraryDetour);
    config = std::make_unique<FlufCrashWalkerConfig>();
    config->Load();

    if (const auto server = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll")); server && config->applyRestartFlPatch)
    {
        // Server.dll should always be loaded, but let's make sure. Additionally, this offset should only be relevant on startup.
        // These offsets will patch server.dll to regenerate restart.fl on every launch.
        // Also ensure restart.fl only gets loaded after being regenerated.

        constexpr std::array<byte, 35> patch1{ 0x8D, 0x8C, 0x24, 0x5C, 0x01, 0x00, 0x00, 0x51, 0x8D, 0x54, 0x24, 0x5C, 0x52, 0xEB, 0x13, 0xFF, 0x11, 0x83,
                                               0xC4, 0x08, 0x85, 0xC0, 0x74, 0x11, 0x8B, 0xCD, 0xE8, 0x22, 0xFD, 0xFF, 0xFF, 0xEB, 0x0F, 0x90, 0xB9 };
        constexpr std::array<byte, 6> patch2{ 0xEB, 0xE6, 0x83, 0xC4, 0x08, 0xEB };

        MemUtils::WriteProcMem(server + 0x06900F, patch1.data(), patch1.size());
        MemUtils::WriteProcMem(server + 0x069036, patch2.data(), patch2.size());
    }
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
