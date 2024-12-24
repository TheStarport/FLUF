#pragma once

#include "ImportFluf.hpp"

class FLUF_API MemoryHelper
{
        inline static auto mainMenuControl = reinterpret_cast<unsigned long*>(0x67BCC8);
        inline static auto* gamePauseIncrementor = reinterpret_cast<unsigned long*>(0x667D54);

    public:
        static bool IsInMainMenu();
        static void EnsurePauseState(bool pause);
        static void PauseGame();
        static void UnPauseGame();
};
