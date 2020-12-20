#include "Checkpoint.h"
#include "RenderingTools.h"

Checkpoint::Checkpoint()
    : LocationType(ELocationType::LT_DEFAULT), Location(ELocation::LOC_DEFAULT), AdditionalTime(0), Code("CheckpointCodeDefault"), Name("CheckpointNameDefault") {}

Checkpoint::Checkpoint(ELocationType InLocationType, ELocation InLocation, float InAdditionalTime, std::string InCode, std::string InName)
	: LocationType(InLocationType), Location(InLocation), AdditionalTime(InAdditionalTime), Code(InCode), Name(InName) {}

void Checkpoint::Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, class RT::Frustum InFrustum, float InSeconds)
{
    //do nothing in base class
}

bool Checkpoint::CheckCollision(CarWrapper InCar, CarLocations InCarLocations)
{
    //do nothing in base class
    return false;
}
