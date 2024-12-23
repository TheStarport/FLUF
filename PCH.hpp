#pragma once

using byte = unsigned char;
using uint = unsigned int;
using i64 = long long;
using u64 = unsigned long long;

#include <Windows.h>

#include <mmsystem.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <winuser.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <numbers>
#include <queue>
#include <ranges>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "FLCore/Common/CommonMethods.hpp"

#include <rfl.hpp>
#include <rfl/yaml.hpp>

template <typename T, const char* path>
    requires std::is_default_constructible_v<T>
struct ConfigHelper
{
        ConfigHelper() = delete;
        static std::optional<T> Load(const bool fromUserData = false, const bool saveIfNotFound = true)
        {
            std::ifstream inFile(GetSaveLocation(fromUserData).data());
            if (!inFile.is_open())
            {
                if (saveIfNotFound)
                {
                    Save(T(), fromUserData);
                    return { T() };
                }

                return std::nullopt;
            }

            auto newConfig = rfl::yaml::read<T>(inFile);
            if (newConfig.error().has_value())
            {
                const std::string err =
                    std::format("Failed to load config file '{}'.\n\n{}\n\nClick 'OK' to reset the config.", path, newConfig.error()->what());
                if (MessageBoxA(nullptr, err.c_str(), "Config Load Error", MB_OKCANCEL | MB_ICONWARNING) != IDOK)
                {
                    std::exit(1);
                }

                Save(T(), fromUserData);
                return { T() };
            }

            return newConfig.value();
        }

        static bool Save(const T& data, const bool fromUserData = false)
        {
            std::ofstream outFile(GetSaveLocation(fromUserData).data(), std::ios::trunc);
            if (!outFile.is_open())
            {
                return false;
            }

            outFile << rfl::yaml::write(static_cast<const T&>(data));
            outFile.close();

            return true;
        }

    private:
        static std::string_view GetSaveLocation(const bool fromUserData)
        {
            static std::string savePath;
            if (savePath.empty())
            {
                if (fromUserData)
                {
                    std::array<char, MAX_PATH> totalPath{};
                    GetUserDataPath(totalPath.data());
                    savePath = std::format("{}/{}", std::string(totalPath.data()), path);
                }
                else
                {
                    savePath = std::format("{}", path);
                }
            }

            return savePath;
        }
};
