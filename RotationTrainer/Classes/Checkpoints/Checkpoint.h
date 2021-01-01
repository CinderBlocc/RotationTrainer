#pragma once
#include "MacrosStructsEnums.h"
#include "bakkesmod/wrappers/wrapperstructs.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"

namespace RT
{
    class Frustum;
}

class Checkpoint
{
public:
    Checkpoint();//Need to keep default constructor for indirect derived classes
	Checkpoint(ELocationType InLocationType, ELocation InLocation, std::string InCode, std::string InName);

    virtual void ResetCheckpoint();

    virtual void Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, class RT::Frustum InFrustum, float InSeconds);
    virtual bool CheckCollision(CarWrapper InCar, CarLocations InCarLocations);

    std::string GetCode() { return Code; }
    std::string GetName() { return Name; }
    ELocationType GetLocationType() { return LocationType; }

protected:
    ELocationType LocationType;
	ELocation Location;
	std::string Code;
	std::string Name;
};
