#include "FLUF.CrashWalker.hpp"

#include "PCH.hpp"

#include "FLCore/Common/CommonMethods.hpp"
#include "FLUF/Include/Fluf.hpp"
#include "Utils/MemUtils.hpp"

#include <curl/curl.h>
#include <minidumpapiset.h>
#include <rfl/Result.hpp>
#include <rfl/json.hpp>
#include <tchar.h>
#include <winapifamily.h>

// ReSharper disable twice CppUseAuto
const st6_malloc_t st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

FlufCrashWalker* module;

int FlufCrashWalker::GlobalExceptionHandler(EXCEPTION_POINTERS* exceptionPointers)
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

    size_t relativeAddress;
    // Couldn't even get the DLL name, what now?
    if (dllName.empty())
    {
        dllName = "Unknown DLL";
        relativeAddress = 0;
    }
    else
    {
        relativeAddress = reinterpret_cast<size_t>(mod.lpBaseOfDll) + mod.SizeOfImage - exOffset;
    }

    // Lookup the error!
    auto error = module->FindError(dllName, relativeAddress);
    if (!error)
    {
        std::array<char, MAX_PATH> totalPath{};
        GetUserDataPath(totalPath.data());

        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        auto dumpPath = config->useOnlySingleDumpFile ? std::format("{}\\crash.dmp", std::string(totalPath.data()))
                                                      : std::format("{}/{:%Y-%m-%d %H.%M.%S}.dmp", std::string(totalPath.data()), time);

        bool createdDump = false;
        if (HANDLE file = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            file != INVALID_HANDLE_VALUE)
        {
            MINIDUMP_EXCEPTION_INFORMATION dumpInformation;
            dumpInformation.ThreadId = GetCurrentThreadId();
            dumpInformation.ExceptionPointers = exceptionPointers;
            dumpInformation.ClientPointers = 0;

            if (auto flags =
                    config->miniDumpFlags > 0 ? config->miniDumpFlags : MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithModuleHeaders;
                MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, static_cast<MINIDUMP_TYPE>(flags), &dumpInformation, nullptr, nullptr) != 0)
            {
                createdDump = true;
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
            << "\tExpAddress: 0x" << exOffset << std::endl;
        if (createdDump)
        {
            str << std::format("\nA crash dump has been generated here: {}\n", dumpPath) << std::endl;
        }
        str << "If you discover how to reproduce this issue, report this to Starport and your mod developer (if you're running one)!" << std::endl;

        MessageBoxA(nullptr, str.str().c_str(), "Fatal Crash. Unknown Error Code", MB_ICONWARNING | MB_OK);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    // clang-format off
    str << "Source: " << dllName <<
        "\nFaulting Offset: 0x" << std::hex << error->offset.value() <<
        "\nFound by: " << error->author <<
        "\nSuspected Reason: " << error->description << std::endl;
    // clang-format on

    MessageBoxA(nullptr, str.str().c_str(), "Fatal Crash. Known Error Code", MB_ICONWARNING | MB_OK);
    return EXCEPTION_EXECUTE_HANDLER;
}

using GameLoopFunc = void (*)(double time);
GameLoopFunc CallGameLoop = reinterpret_cast<GameLoopFunc>(0x5B2890);

void FlufCrashWalker::TryCatchDetour(const double time)
{
    __try
    {
        CallGameLoop(time);
    }
    __except (GlobalExceptionHandler(GetExceptionInformation()))
    {
        std::exit(1);
    }
}

FunctionDetour loadLibraryDetour(LoadLibraryA);
BOOL WINAPI DllMain(const HMODULE mod, [[maybe_unused]] const DWORD reason, [[maybe_unused]] LPVOID reserved)
{
    DisableThreadLibraryCalls(mod);

    // Add a global try/catch to the application
    MemUtils::PatchCallAddr(GetModuleHandle(nullptr), 0x1B3378, FlufCrashWalker::TryCatchDetour);
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

size_t DownloadCallback(const void* ptr, const size_t size, const size_t nmemb, void* data)
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

void FlufCrashWalker::LoadErrorPayloadFromCache(const std::string_view path)
{
    if (!std::filesystem::exists(path))
    {
        return;
    }

    const std::ifstream file(path.data(), std::ios::binary);
    if (!file.is_open())
    {
        Fluf::Log(LogLevel::Error, "Unable to open cache file for reading");
        return;
    }

    std::string buffer;
    std::copy(std::istreambuf_iterator(file.rdbuf()), std::istreambuf_iterator<char>(), std::back_inserter(buffer));

    auto result = rfl::json::read<std::vector<ErrorPayload>>(buffer);
    if (result.error().has_value())
    {
        Fluf::Log(LogLevel::Error, std::format("Error reading payload: {}", result.error().value().what()));
        return;
    }

    possibleErrors = result.value();
    Fluf::Log(LogLevel::Info, "Loaded possible errors from local crash walker cache.");
}

void FlufCrashWalker::OnGameLoad()
{
    crashCatcher = std::make_unique<CrashCatcher>();

    std::array<char, MAX_PATH> totalPath{};
    GetUserDataPath(totalPath.data());

    const std::string cachePath = std::format("{}/crash_cache.json", std::string(totalPath.data()));

    // Download latest crash payload, or load from the cache if not possible
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        Fluf::Log(LogLevel::Error, "Unable to initialise cURL");
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
        LoadErrorPayloadFromCache(cachePath);
        return;
    }

    const auto errorPayload = std::string(buffer.data, buffer.size);
    auto result = rfl::json::read<std::vector<ErrorPayload>>(errorPayload);
    free(buffer.data);
    if (result.error().has_value())
    {
        Fluf::Log(LogLevel::Error, std::format("Error reading payload: {}", result.error().value().what()));
        LoadErrorPayloadFromCache(cachePath);
        return;
    }

    possibleErrors = result.value();
    std::ofstream file(cachePath, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        Fluf::Log(LogLevel::Error, "Unable to open cache file for writing");
        LoadErrorPayloadFromCache(cachePath);
        return;
    }

    file.write(errorPayload.c_str(), errorPayload.size());
    file.close();
}

FlufCrashWalker::FlufCrashWalker()
{
    module = this;
    loadLibraryDetour.Detour(LoadLibraryDetour);
    config = std::make_unique<Config>();
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

ErrorPayload* FlufCrashWalker::FindError(const std::string_view module, const size_t offset)
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
