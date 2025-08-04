#pragma once

#include <exception>
#include <string>

class ModuleLoadException : public std::exception
{
        std::string reason;

    public:
        ModuleLoadException(const std::string& reason) : reason(reason) {}

        const char* what() const override { return reason.c_str(); }
};
