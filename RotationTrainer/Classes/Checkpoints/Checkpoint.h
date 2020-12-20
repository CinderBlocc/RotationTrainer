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
	Checkpoint(ELocationType InLocationType, ELocation InLocation, float InAdditionalTime, std::string InCode, std::string InName);

    virtual void Draw(CanvasWrapper InCanvas, CameraWrapper InCamera, class RT::Frustum InFrustum, float InSeconds);
    virtual bool CheckCollision(CarWrapper InCar, CarLocations InCarLocations);

    std::string GetCode() { return Code; }
    std::string GetName() { return Name; }

protected:
    ELocationType LocationType;
	ELocation Location;
	float AdditionalTime;
	std::string Code;
	std::string Name;
};