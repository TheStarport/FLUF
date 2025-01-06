#pragma once

#include "Fluf.hpp"

#include <string>
#include <unordered_set>

class Fluf;
class FlufConfiguration
{
        friend Fluf;

        bool Save();
        void Load();

    public:
        LogLevel logLevel = LogLevel::Info;
        std::unordered_set<LogSink> logSinks;
        std::unordered_set<std::string> modules;
        bool setSaveDirectoryRelativeToExecutable = false;
};
