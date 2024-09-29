#include "PCH.hpp"

#include "Rml/Interfaces/SystemInterface.hpp"

double SystemInterface::GetElapsedTime()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    return static_cast<double>(counter.QuadPart - timeStartup.QuadPart) * timeFrequency;
}

void SystemInterface::SetMouseCursor(const Rml::String& cursor_name)
{
    // TODO: Set cursor
    Rml::SystemInterface::SetMouseCursor(cursor_name);
}

bool SystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
    // TODO: Hook up to logging system
    return Rml::SystemInterface::LogMessage(type, message);
}

SystemInterface::SystemInterface()
{
    LARGE_INTEGER time_ticks_per_second;
    QueryPerformanceFrequency(&time_ticks_per_second);
    QueryPerformanceCounter(&timeStartup);

    timeFrequency = 1.0 / static_cast<double>(time_ticks_per_second.QuadPart);
}
