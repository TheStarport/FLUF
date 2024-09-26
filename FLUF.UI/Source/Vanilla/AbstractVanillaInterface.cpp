#include "PCH.hpp"

#include "Vanilla/AbstractVanillaInterface.hpp"
#include <FLCore/FLCoreDefs.hpp>

void AbstractVanillaInterface::OpenDialogue(unsigned int captionIds, unsigned int bodyIds) {}

void AbstractVanillaInterface::CloseDialogue() {}

FlStaticControl* AbstractVanillaInterface::CreateDefaultStaticControl(const FlControl* parent, const char* name, const char* mesh, const Vector& pos,
                                                                      const char* hardpoint, const Vector& mouseSize) const
{
    return CreateStaticControl(parent, name, mesh, pos, hardpoint, mouseSize, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, Vector{ 0, 0, 0 });
}
