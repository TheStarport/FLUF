#pragma once
#include <string>
#include <unordered_set>

class Fluf;
class FlufConfiguration
{
        friend Fluf;

        bool Save();
        void Load();

    public:
        std::unordered_set<std::string> modules;
};
