#include "PCH.hpp"

#include "Rml/Interfaces/FileInterface.hpp"
#include "Rml/Interfaces/RenderInterfaceDirectX9.hpp"
#include "Rml/Interfaces/RmlInterface.hpp"
#include "Rml/Interfaces/SystemInterface.hpp"
#include "RmlUi/Core/Core.h"
// #include "RmlUi/Lua.h"

RmlInterface::RmlInterface(FlufUi* fluf)
{
    // TODO: Support OpenGL as well
    // renderInterface = std::make_unique<RenderInterfaceDirectX9>();
    Rml::Initialise();
    // Rml::Lua::Initialise();
}
