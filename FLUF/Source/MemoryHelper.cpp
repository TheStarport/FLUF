#include "PCH.hpp"

#include "MemoryHelper.hpp"

bool MemoryHelper::IsInMainMenu() { return *mainMenuControl != 0; }
void MemoryHelper::EnsurePauseState(const bool pause) { *gamePauseIncrementor = pause ? 0 : 1; }
void MemoryHelper::PauseGame() { *gamePauseIncrementor += 1; }
void MemoryHelper::UnPauseGame() { *gamePauseIncrementor -= 1; }
