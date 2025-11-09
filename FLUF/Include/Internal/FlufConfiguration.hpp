#pragma once

#include "Fluf.hpp"

#include <string>
#include <unordered_set>

class Fluf;
struct FlufConfiguration
{
        static constexpr char path[] = "FLUF.yml";
        LogLevel logLevel = LogLevel::Info;
        std::unordered_set<LogSink> logSinks{ LogSink::File };
        std::unordered_set<std::string> clientModules;
        std::unordered_set<std::string> serverModules;
        bool setSaveDirectoryRelativeToExecutable = false;
        bool writeSpewToLogSinks = true;
        std::string saveDirectoryName = "SAVES";
};
