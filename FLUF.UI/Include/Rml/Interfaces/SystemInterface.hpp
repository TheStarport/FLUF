#pragma once

#include <RmlUi/Core/SystemInterface.h>

class SystemInterface final : public Rml::SystemInterface
{
        inline static double timeFrequency;
        inline static LARGE_INTEGER timeStartup;

        double GetElapsedTime() override;

        void SetMouseCursor(const Rml::String& cursor_name) override;
        bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

    public:
        SystemInterface();
};
