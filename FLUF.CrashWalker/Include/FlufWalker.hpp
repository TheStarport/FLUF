#pragma once
#include "FLUF.CrashWalker.hpp"

#include <StackWalker.h>

class FlufWalker final : public StackWalker
{
        std::string outputBuffer;

    protected:
        void OnCallstackEntry(CallstackEntryType eType, CallstackEntry& entry) override;

    public:
        explicit FlufWalker();
};
