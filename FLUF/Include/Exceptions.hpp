#pragma once

#include <exception>
#include <string>

class ModuleLoadException : std::exception
{
        std::string reason;

    public:
        ModuleLoadException(const std::string& reason) : reason(reason) {}

        const char* what() const override { return reason.c_str(); }
};
