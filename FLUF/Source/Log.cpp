#include <PCH.hpp>

#include <Fluf.hpp>
#include <Internal/FlufConfiguration.hpp>
#include <spdlog/sinks/rotating_file_sink.h>

std::shared_ptr<spdlog::logger> logFile;

using FDump = DWORD (*)(DWORD, const char*, ...);
FDump fdumpOriginal;
DWORD FDumpDetour(DWORD unk, const char* fmt, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 4096, fmt, args);
    va_end(args);

    const std::string line(buffer);
    auto level = LogLevel::Info;

    auto contains = [](std::string_view a, std::string_view b)
    { return std::ranges::search(a, b, [](unsigned char ch1, unsigned char ch2) { return std::toupper(ch1) == std::toupper(ch2); }).begin() != a.end(); };

    if (contains(line, "WARN") || contains(line, "UNKNOWN"))
    {
        level = LogLevel::Warn;
    }
    else if (contains(line, "ERR"))
    {
        level = LogLevel::Error;
    }
    else if (contains(line, "DBG") || contains(line, "DEBUG"))
    {
        level = LogLevel::Debug;
    }

    Fluf::Log(level, line);
    std::time_t rawTime;
    char timestamp[100];
    std::time(&rawTime);
    std::tm* timeInfo = std::localtime(&rawTime);
    std::strftime(timestamp, 80, "%Y/%m/%d - %H:%M:%S", timeInfo);

    return fdumpOriginal(unk, "[%s] %s", timestamp, line.c_str());
}

std::string SetLogMetadata(void* address, const LogLevel level)
{
    if (HMODULE dll; RtlPcToFileHeader(address, reinterpret_cast<void**>(&dll)))
    {
        std::array<char, MAX_PATH> path{};
        if (GetModuleFileNameA(dll, path.data(), path.size()))
        {
            const std::string fullPath = path.data();
            std::string levelStr;
            switch (level)
            {
                case LogLevel::Trace: levelStr = "TRACE"; break;
                case LogLevel::Debug: levelStr = "DBG"; break;
                case LogLevel::Info: levelStr = "INFO"; break;
                case LogLevel::Warn: levelStr = "WARN"; break;
                case LogLevel::Error: levelStr = "ERR"; break;
            }

            return std::format("({}) {}: ", fullPath.substr(fullPath.find_last_of("\\") + 1), levelStr);
        }
    }

    return "";
}

void Fluf::Log(const LogLevel logLevel, const std::string_view message)
{
    if (logLevel < instance->config->logLevel)
    {
        return;
    }

    const std::string paddedMessage = SetLogMetadata(_ReturnAddress(), logLevel) + std::string(message);

    if (instance->config->logSinks.contains(LogSink::Console))
    {
        enum class ConsoleColour
        {
            Blue = 1,
            Green = 2,
            Red = 4,
            Bold = 8,
            Cyan = Blue | Green,
            Purple = Red | Blue,
            Yellow = Red | Green,
            White = Red | Green | Blue,
            StrongWhite = White | Bold,
            StrongCyan = Cyan | Bold,
            StrongRed = Red | Bold,
            StrongYellow = Yellow | Bold,
            StrongGreen = Green | Bold,
        };

        const auto outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        switch (logLevel)
        {
            case LogLevel::Trace: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongCyan)); break;
            case LogLevel::Debug: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongGreen)); break;
            case LogLevel::Info: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongWhite)); break;
            case LogLevel::Warn: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongYellow)); break;
            case LogLevel::Error: SetConsoleTextAttribute(outputHandle, static_cast<WORD>(ConsoleColour::StrongRed)); break;
        }

        std::cout << paddedMessage << std::endl;
    }

    if (logFile)
    {
        logFile->log(static_cast<spdlog::level::level_enum>(logLevel), paddedMessage);
    }
}

void Fluf::Trace(const std::string_view message) { return Log(LogLevel::Trace, message); }
void Fluf::Debug(const std::string_view message) { return Log(LogLevel::Debug, message); }
void Fluf::Info(const std::string_view message) { return Log(LogLevel::Info, message); }
void Fluf::Warn(const std::string_view message) { return Log(LogLevel::Warn, message); }
void Fluf::Error(const std::string_view message) { return Log(LogLevel::Error, message); }

void Fluf::SetupLogging() const
{
    // Console sink enabled, allocate console and allow us to use std::cout
    if (config->logSinks.contains(LogSink::Console))
    {
        AllocConsole();
        SetConsoleTitleA("FLUF - Freelancer Unified Framework");

        const auto console = GetConsoleWindow();
        RECT r;
        GetWindowRect(console, &r);

        MoveWindow(console, r.left, r.top, 1366, 768, TRUE);

        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        SetStdHandle(STD_OUTPUT_HANDLE, stdout);
    }

    if (config->logSinks.contains(LogSink::File))
    {
        std::array<char, MAX_PATH> path;
        GetUserDataPath(path.data());

        std::string_view pathPartial{ path.data(), strlen(path.data()) };
        logFile = spdlog::rotating_logger_st("file_logger", std::format("{}\\fluf.log", pathPartial), 1048576 * 5, 3);
        logFile->set_pattern("[%H:%M:%S %z] %v");
    }

    if (config->writeSpewToLogSinks)
    {
        auto fdump = reinterpret_cast<FDump*>(GetProcAddress(GetModuleHandleA("dacom.dll"), "FDUMP"));
        fdumpOriginal = *fdump;
        *fdump = reinterpret_cast<FDump>(FDumpDetour);
    }
}
