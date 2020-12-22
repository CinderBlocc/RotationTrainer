#include "Checkpoint.h"
#include "RenderingTools.h"

Checkpoint::Checkpoint()
    : LocationType(ELocationType::LT_DEFAULT), Location(ELocation::LOC_DEFAULT), Code("CheckpointCodeDefault"), Name("CheckpointNameDefault") {}

Checkpoint::Checkpoint(ELocationType InLocationType, ELocation InLocation, std::string InCode, std::string InName)
	: LocationType(InLocationType), Location(InLocation), Code(InCode), Name(InName) {}

void Checkpoint::Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, class RT::Frustum InFrustum, float InSeconds)
{
    //do nothing in base class
}

bool Checkpoint::CheckCollision(CarWrapper InCar, CarLocations InCarLocations)
{
    //do nothing in base class
    return false;
}
